#
# Copyright (c) 2020 Nordic Semiconductor ASA
#
# SPDX-License-Identifier: Apache-2.0
#

from regex import match, search, sub, findall, S, M
from pprint import pformat, pprint
from os import path, linesep, makedirs
from collections import defaultdict
from argparse import ArgumentParser, RawDescriptionHelpFormatter

# Size of "additional" field if num is encoded as int
def sizeof(num):
    if (num <= 23):
        return 0
    elif (num < 0x100):
        return 1
    elif (num < 0x10000):
        return 2
    elif (num < 0x100000000):
        return 4
    elif (num < 0x10000000000000000):
        return 8
    else:
        raise ValueError("Number too large (more than 64 bits).")

# Global counter
_counter = 0

# Retrieve a unique id.
def counter(reset = False):
    global _counter
    if reset:
        _counter = 0
        return _counter
    _counter += 1
    return _counter

# Replace an element in a list or tuple and return the list. For use in lambdas.
def listReplaceIfNotNull(l, i, r):
    if l[i] is "NULL":
        return l
    if isinstance(l, tuple):
        convert = tuple
        l = list(l)
    else:
        assert isinstance(l, list)
        convert = list
    l[i] = r
    return convert(l)

# Class for parsing CDDL. One instance represents one CBOR data item, with a few caveats:
#  - For repeated data, one instance represents all repetitions.
#  - For "OTHER" types, one instance points to another type definition.
#  - For "GROUP" and "UNION" types, there is no separate data item for the instance.
class TYPE:

    # The default maximum number of repeated elements.
    DEFAULT_MAXQ = 3

    def __init__(self):
        self.id_prefix = "temp_" + str(counter())
        self.id_num = None # Unique ID number. Only populated if needed.
        self.value = None # The value of the data item. Has different meaning for different types.
        self.maxValue = None # Maximum value. Only used for numbers and bools.
        self.minValue = None # Minimum value. Only used for numbers and bools.
        self.label = None # The readable label associated with the element in the CDDL.
        self.minQ = 1 # The minimum number of times this element is repeated.
        self.maxQ = 1 # The maximum number of times this element is repeated.
        self.size = None  # The size of the element. Only used for integers, byte strings, and text strings.
        self.minSize = None # Minimum size.
        self.maxSize = None # Maximum size.
        self.cbor = None # The element specified via .cbor or .cborseq (only for byte strings). self.cbor is of the same
                         # class as self.
        self.type = None # The "type" of the element. This follows the CBOR types loosely, but are more related to CDDL
                         # concepts. The possible types are "INT", "UINT", "NINT", "FLOAT", "BSTR", "TSTR", "BOOL",
                         # "NIL", "LIST", "MAP", "GROUP", "UNION" and "OTHER". "OTHER" represents a CDDL type defined
                         # with '='.
        self.key = None # Key element. Only for children of "MAP" elements. self.key is of the same class as self.
        self.quantifier = None # The CDDL string used to determine the minQ and maxQ. Not used after minQ and maxQ are
                               # determined.
        self.matchStr = ""

    def setIdPrefix(self, id_prefix):
        self.id_prefix = id_prefix
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            for child in self.value:
                child.setIdPrefix(self.childBaseId())
        if self.cbor:
            self.cbor.setIdPrefix(self.childBaseId())
        if self.key:
            self.key.setIdPrefix(self.childBaseId())

    def id(self):
        return "%s_%s" % (self.id_prefix, self.idNum())

    # Id to pass to children for them to use as basis for their id/base name.
    def childBaseId(self):
        return self.id()

    def idNum(self):
        if self.id_num == None:
            self.id_num = counter()
        return self.id_num

    # Human readable representation.
    def mrepr(self, newline):
        reprstr = ''
        if self.quantifier:  reprstr += self.quantifier
        if self.label:  reprstr += self.label + ':'
        if self.key:    reprstr += repr(self.key) + " => "
        reprstr += self.type
        if self.size:   reprstr += '(%d)' % self.size
        if newline:     reprstr += '\n'
        if self.value:  reprstr += pformat(self.value, indent=4, width=1)
        if self.cbor:   reprstr += " cbor: " + repr(self.cbor)
        return  reprstr.replace('\n', '\n    ')

    def doFlatten(self, input, allowmulti=False):
        input.flatten()
        if input.type in ["GROUP", "UNION"] and (len(input.value) == 1) and (not (input.key and input.value[0].key)):
            input.value[0].minQ *= input.minQ
            input.value[0].maxQ *= input.maxQ
            if not input.value[0].label: input.value[0].label = input.label
            if not input.value[0].key: input.value[0].key = input.key
            return input.value
        elif allowmulti and input.type in ["GROUP"] and input.minQ == 1 and input.maxQ == 1:
            return input.value
        else:
            return [input]

    def flatten(self):
        newvalue = []
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            for child in self.value:
                newvalue.extend(self.doFlatten(child, allowmulti=self.type is not "UNION"))
            self.value = newvalue
        if self.key: self.key = self.doFlatten(self.key)[0]
        if self.cbor: self.cbor = self.doFlatten(self.cbor)[0]

    # Set the self.type and self.value of this element. For use during CDDL parsing.
    def typeAndValue(self, type, valueGenerator):
        if self.type is not None: raise TypeError("Cannot have two values: %s, %s" % (self.type, type))
        if type is None: raise TypeError("Cannot set None as type")
        if type is "UNION" and self.value is not None:
            raise ValueError("Did not expect multiple parsed values for union")

        self.type = type

        value = valueGenerator()
        self.value = value

        if type in ["BSTR", "TSTR"]:
            if value is not None:
                self.setSize(len(value))
        if type in ["UINT", "NINT"]:
            if value is not None:
                self.size = sizeof(value)
                self.minValue = value
                self.maxValue = value


    # Set the self.type and self.minValue and self.maxValue (or self.minSize and self.maxSize depending on the type) of
    # this element. For use during CDDL parsing.
    def typeAndRange(self, type, minVal, maxVal):
        if type not in ["INT", "UINT", "NINT"]:
            raise TypeError("Only integers (not %s) can have range" % (type,))
        if minVal > maxVal:
            raise TypeError("Range has larger minimum than maximum (min %d, max %d)" % (minVal, maxVal))
        if minVal == maxVal:
            return self.typeAndValue(type, minVal)
        self.type = type
        self.minValue = minVal
        self.maxValue = maxVal
        if type in "UINT":
            self.setSizeRange(sizeof(minVal), sizeof(maxVal))
        if type is "NINT":
            self.setSizeRange(sizeof(abs(maxVal)), sizeof(abs(minVal)))
        if type is "INT":
            self.setSizeRange(0, max(sizeof(abs(maxVal)), sizeof(abs(minVal))))


    # Set the self.value and self.size of this element. For use during CDDL parsing.
    def typeValueSize(self, type, value, size):
        self.typeAndValue(type, value)
        self.setSize(size)

    # Set the self.label of this element. For use during CDDL parsing.
    def setLabel(self, label):
        if self.type is not None: raise TypeError("Cannot have label after value: " + label)
        self.label = label

    # Set the self.quantifier, self.minQ, and self.maxQ of this element. For use during CDDL parsing.
    def setQuantifier(self, quantifier):
        if self.type is not None: raise TypeError("Cannot have quantifier after value: " + quantifier)

        quantifierMapping =\
        [
            ("\?", lambda matchObj: (0, 1)),
            ("\*", lambda matchObj: (0, None)),
            ("\+", lambda matchObj: (1, None)),
            ("(\d*)\*(\d*)", lambda matchObj: (int(matchObj.groups()[0]), int(matchObj.groups()[1]))),
        ]

        self.quantifier = quantifier
        for (reg, handler) in quantifierMapping:
            matchObj = match(reg, quantifier)
            if matchObj:
                (self.minQ, self.maxQ) = handler(matchObj)
                if self.maxQ is None: self.maxQ = self.DEFAULT_MAXQ
                return
        raise ValueError("invalid quantifier: %s" % quantifier)

    # Set the self.size of this element. This will also set the self.minValue and self.maxValue of UINT types.
    # For use during CDDL parsing.
    def setSize(self, size):
        if self.type is None: raise TypeError("Cannot have size before value: " + str(size))
        elif self.type is "UINT" and size not in [0, None]:
            return self.setSizeRange(0, size)
        elif self.type not in ["UINT", "BSTR", "TSTR"]:
            raise TypeError(".size cannot be applied to %s" % self.type)
        self.size = size

    # Set the self.minValue and self.maxValue or self.minSize and self.maxSize of this element based on what values
    # can be contained within an integer of a certain size. For use during CDDL parsing.
    def setSizeRange(self, minSize, maxSize):
        if minSize == maxSize:
            return self.setSize(minSize)
        elif minSize > maxSize:
            raise TypeError("Invalid size range (min %d, max %d)" % (minSize, maxSize))
        else:
            self.setSize(None)

        if self.type is "UINT":
            if maxSize > 8:
                raise TypeError("Size too large for integer. size %d" % maxSize)
        if self.type is "UINT":
            self.minValue = 256**min(0, abs(minSize-1))
            self.maxValue = 256**maxSize - 1
        self.minSize = minSize
        self.maxSize = maxSize

    # Set the self.cbor of this element. For use during CDDL parsing.
    def setCbor(self, cbor, cborseq):
        if self.type is not "BSTR": raise TypeError("%s must be used with bstr." % (".cborseq" if cborseq else ".cbor",))
        self.cbor = cbor
        if cborseq: self.cbor.maxQ = self.DEFAULT_MAXQ
        self.cbor.setBaseName("cbor")

    # Set the self.key of this element. For use during CDDL parsing.
    def setKey(self, key):
        if self.key is not None: raise TypeError("Cannot have two keys: " + key)
        if key.type is "GROUP": raise TypeError("A key cannot be a group")
        self.key = key
        self.key.setBaseName("key")

    # Set the self.label OR self.key of this element. In the CDDL "foo: bar", foo can be either a label or a key
    # depending on whether it is in a map. This code uses a slightly different method for choosing between label and
    # key. If the string is recognized as a type, it is treated as a key. For use during CDDL parsing.
    def setKeyOrLabel(self, keyorlabel):
        if keyorlabel.type is "OTHER" and keyorlabel.value not in mytypes:
            self.setLabel(keyorlabel.value)
        else:
            if keyorlabel.type is "OTHER" and self.label is None:
                self.setLabel(keyorlabel.value)
            self.setKey(keyorlabel)

    # Append to the self.value of this element. Used with the "MAP", "LIST", "UNION", and "GROUP" types, which all have
    # a python list as self.value. The list represents the "children" of the type. For use during CDDL parsing.
    def addValue(self, value):
        self.value.append(value)

    # Parse from the beginning of instr (string) until a full element has been parsed. self will become that element.
    # This function is recursive, so if a nested element ("MAP"/"LIST"/"UNION"/"GROUP") is encountered, this function
    # will create new instances and add them to self.value as a list. Likewise, if a key or cbor definition is
    # encountered, a new element will be created and assigned to self.key or self.cbor. When new elements are created,
    # getValue() is called on those elements, via parse().
    def getValue(self, instr):
        # The following regexes match different parts of the element. The order of the list is important because it
        # implements the operator precendences defined in the CDDL spec. Note that some regexes are inserted afterwards
        # because they involve a match of a concatenation of all the initial regexes (with a '|' between each element).
        types = [
            (r'\A(?!\/\/).+?(?=\/\/)',            lambda unionstr: self.typeAndValue("UNION", lambda: parse("(%s)" % unionstr if ',' in unionstr else unionstr))),
            (r'\/\/\s*(?P<item>.+?)(?=\/\/|\Z)',  lambda unionstr: self.addValue(parse("(%s)" % unionstr if ',' in unionstr else unionstr)[0])),
            (r'([+*?])',                          lambda quantifier: self.setQuantifier(quantifier)),
            (r'(\d*\*\d*)',                       lambda quantifier: self.setQuantifier(quantifier)),
            (r'uint',                             lambda _: self.typeAndValue("UINT", lambda: None)),
            (r'nint',                             lambda _: self.typeAndValue("NINT", lambda: None)),
            (r'int',                              lambda _: self.typeAndValue("INT",  lambda:None)),
            (r'float',                            lambda _: self.typeAndValue("FLOAT", lambda: None)),
            (r'float16',                          lambda _: self.typeValueSize("FLOAT", None, 2)),
            (r'float32',                          lambda _: self.typeValueSize("FLOAT", None, 4)),
            (r'float64',                          lambda _: self.typeValueSize("FLOAT", None, 8)),
            (r'\-?\d*\.\d+',                      lambda num: self.typeAndValue("FLOAT", lambda: int(num))),
            (r'\-\d+',                            lambda num: self.typeAndValue("NINT", lambda: int(num))),
            (r'\d+\.\.\d+',                       lambda range: self.typeAndRange("UINT", *map(int, range.split("..")))),
            (r'\-\d+\.\.\d+',                     lambda range: self.typeAndRange("INT", *map(int, range.split("..")))),
            (r'\-\d+\.\.\-\d+',                   lambda range: self.typeAndRange("NINT", *map(int, range.split("..")))),
            (r'\d+',                              lambda num: self.typeAndValue("UINT", lambda: int(num))),
            (r'0[xbo]\w+',                        lambda num: self.typeAndValue("UINT", lambda: int(num, 0))),
            (r'bstr',                             lambda _: self.typeAndValue("BSTR", lambda: None)),
            (r'tstr',                             lambda _: self.typeAndValue("TSTR", lambda: None)),
            (r'\".*?\"(?<!\\)',                   lambda string: self.typeAndValue("TSTR", lambda: string.strip('"'))),
            (r'\[(?P<item>(?>[^[\]]+|(?R))*)\]',  lambda liststr: self.typeAndValue("LIST", lambda: parse(liststr))),
            (r'\((?P<item>(?>[^\(\)]+|(?R))*)\)', lambda groupstr: self.typeAndValue("GROUP", lambda: parse(groupstr))),
            (r'{(?P<item>(?>[^{}]+|(?R))*)}',     lambda map: self.typeAndValue("MAP", lambda: parse(map))),
            (r'bool',                             lambda _: self.typeAndValue("BOOL", lambda: None)),
            (r'true',                             lambda _: self.typeAndValue("BOOL", lambda: True)),
            (r'false',                            lambda _: self.typeAndValue("BOOL", lambda: False)),
            (r'nil',                              lambda _: self.typeAndValue("NIL", lambda: None)),
            (r'any',                              lambda _: self.typeAndValue("ANY", lambda: None)),
            (r'(\$?\$?[\w-]+)',                   lambda otherstr: self.typeAndValue("OTHER", lambda: otherstr.strip("$"))),
            (r'\.size \(?(?P<item>\d+)\)?',       lambda size: self.setSize(int(size))),
            (r'\.size \(?(?P<item>\d+\.\.\d+)\)?',lambda range: self.setSizeRange(*map(int, range.split("..")))),
            (r'\.cbor (?P<item>[\w-]+)',          lambda typestr: self.setCbor(parse(typestr)[0], False)),
            (r'\.cborseq (?P<item>[\w-]+)',       lambda typestr: self.setCbor(parse(typestr)[0], True))
        ]
        all_type_regex = '|'.join([regex for (regex, _) in types[3:]])
        for i in range(0, all_type_regex.count("item")):
            all_type_regex = all_type_regex.replace("item", "it%dem" % i, 1)
        types.insert(3, (r'(?P<item>'+all_type_regex+r')\s*\:', lambda keystr: self.setKeyOrLabel(parse(keystr, self.id_prefix)[0])))
        types.insert(4, (r'(?P<item>'+all_type_regex+r')\s*\=\>', lambda keystr: self.setKey(parse(keystr, self.id_prefix)[0])))
        types.insert(5, (r'(?P<item>(('+all_type_regex+r')\s*)+?)(?=\/)', lambda unionstr: self.typeAndValue("UNION", lambda: parse(unionstr))))
        types.insert(6, (r'\/\s*(?P<item>(('+all_type_regex+r')\s*)+?)(?=\/|\,|\Z)', lambda unionstr: self.addValue(parse(unionstr)[0])))

        # Keep parsing until a comma, or to the end of the string.
        while instr is not '' and instr[0] is not ',':
            matchObj = None
            for (reg, handler) in types:
                # matchObj = match(reg, instr, S)
                matchObj = match(reg, instr)
                if matchObj:
                    break

            if not matchObj: raise TypeError("Could not parse this: '%s'" % instr)
            try:
                matchstr = matchObj.group("item")
            except IndexError as e:
                matchstr = matchObj.group(0)
            handler(matchstr)
            self.matchStr += matchstr
            oldLen = len(instr)
            instr = sub(reg, '', instr, count=1).lstrip()
            if oldLen == len(instr):
                raise(Exception("empty match"))
        instr = instr[1:]
        if not self.type: raise ValueError("No proper value while parsing: %s" % instr)

        # Return the unparsed part of the string.
        return instr

    # For checking whether this element has a key (i.e. to check that it is a valid "MAP" child).
    # This must have some recursion since CDDL allows the key to be hidden behind layers of indirection.
    def hasKey(self):
        return self.key is not None\
            or (self.type is "OTHER" and mytypes[self.value].hasKey())\
            or (self.type in ["GROUP", "UNION"] and all(child.hasKey() for child in self.value))

    # Function for performing validations that must be done after all parsing is complete. This is recursive, so
    # it will postValidate all its children + key + cbor.
    def postValidate(self):
        # Validation of this element.
        if self.type is "MAP":
            none_keys = [child for child in self.value if not child.hasKey()]
            if (none_keys):
                raise TypeError("Map entry must have key: " + str(none_keys))
        if self.type is "OTHER":
            if self.value not in mytypes.keys() or type(mytypes[self.value]) is not type(self):
                raise TypeError("%s has not been parsed." % self.value)
        if type is "ANY":
            if (self.minQ != self.maxQ):
                raise TypeError("ambiguous uses of 'any' are not supported.")
        if self.type is "UNION" and len(self.value) > 1:
            if any(((not child.key and child.type == "ANY") or (child.key and child.key.type == "ANY")) for child in self.value):
                raise TypeError("'any' inside union is not supported since it would always be triggered.")

        # Validation of child elements.
        if self.type in ["MAP", "LIST", "UNION", "GROUP"]:
            for child in self.value:
                child.postValidate()
        if self.key:
            self.key.postValidate()
        if self.cborVarCondition():
            self.cbor.postValidate()

    def __repr__(self):
        return self.mrepr(False)


# Class for generating C code that decodes CBOR and validates it according to the CDDL.
class TYPE_decoder_generator_CBOR(TYPE):

    indentation = "\t"
    newl_ind = "\n" + indentation

    def __init__(self, base_name=None):
        super(TYPE_decoder_generator_CBOR, self).__init__()
        self.accessPrefix = None # The prefix used for C code accessing this element, i.e. the struct hierarchy leading
                                 # up to this element. This can change multiple times during generation to suit
                                 # different situations.
        self.accessDelimiter = "." # The delimiter used between elements in the accessPrefix.
        self.countIndirection = 0 # The depth of indirection of the elem_count within the current C function. This is
                                  # incremented e.g. when decoding elements in a list, and decremented when the list has
                                  # been decoded.
        self.dependsOnCall = False # Used as a guard against endless recursion in self.dependsOn()
        self.base_name = base_name # Used as default for self.baseName()

    # Base name used for functions, variables, and typedefs.
    def baseName(self):
        return ((self.base_name or self.label
                 or (self.key.value if self.key and self.key.type in ["TSTR", "OTHER"] else None)
                 or (f"{self.value}_{self.type.lower()}" if self.type is "TSTR" and self.value is not None else None)
                 or (next((key for key, value in mytypes.items() if value == self), None))
                 or ("_"+self.value if self.type is "OTHER" else None)
                 or ("_"+self.value[0].baseName() if self.type in ["LIST", "GROUP"] and self.value is not None else None)
                 or (self.cbor.value if self.cbor and self.cbor.type in ["TSTR", "OTHER"] else None)
                 or self.type.lower()).replace("-", "_"))

    # Set an explicit base name for this element.
    def setBaseName(self, base_name):
        self.base_name = base_name

    # Add uniqueness to the base name.
    def id(self):
        return "%s%s" % ((self.id_prefix+"_") if self.id_prefix is not "" else "", self.baseName())

    # Base name if this element needs to declare a type.
    def rawTypeName(self):
        return "%s_t" % self.id()

    # Name of the type of this element's actual value variable.
    def valTypeName(self):
        if self.multiValCondition():
            return self.rawTypeName()

        name = {
            "INT":   lambda : "int32_t",
            "UINT":  lambda : "uint32_t",
            "NINT":  lambda : "int32_t",
            "FLOAT": lambda : "float_t",
            "BSTR":  lambda : "cbor_string_type_t",
            "TSTR":  lambda : "cbor_string_type_t",
            "BOOL":  lambda : "bool",
            "NIL":   lambda : None,
            "ANY":   lambda : None,
            "LIST":  lambda : self.value[0].typeName(),
            "MAP":   lambda : self.value[0].typeName(),
            "GROUP": lambda : self.value[0].typeName(),
            "UNION": lambda : self.unionType(),
            "OTHER": lambda : mytypes[self.value].typeName(),
        }[self.type]()

        return name

    # Name of the type of for the repeated part of this element.
    def repeatedTypeName(self):
        if self.selfRepeatedMultiVarCondition():
            name = self.rawTypeName()
            if self.valTypeName() == name:
                name = "_" + name
        else:
            name = self.valTypeName()
        return name

    # Name of the type for this element.
    def typeName(self):
        if self.multiVarCondition():
            name = self.rawTypeName()
            if self.valTypeName() == name:
                name = "_" + name
            if self.repeatedTypeName() == name:
                name = "_" + name
        else:
            name = self.repeatedTypeName()
        return name

    # Name of variables and enum members for this element.
    def varName(self):
        name = ("_%s" % self.id())
        return name

    # Create an access prefix based on an existing prefix, delimiter and a suffix.
    def accessAppendDelim(self, prefix, delimiter, *suffix):
        assert prefix is not None, "No access prefix for %s" % self.varName()
        return delimiter.join((prefix,) + suffix)

    # Create an access prefix from this element's prefix, delimiter and a provided suffix.
    def accessAppend(self, *suffix, full=True):
        suffix = list(suffix)
        return self.accessAppendDelim(self.accessPrefix, self.accessDelimiter, *suffix)

    # "Path" to this element's variable.
    # If full is false, the path to the repeated part is returned.
    def varAccess(self, full=False):
        return self.accessAppend(full=full)

    # "Path" to access this element's actual value variable.
    def valAccess(self):
        return self.accessAppend(self.varName(), full=False)

    # Whether to include a "present" variable for this element.
    def presentVarCondition(self):
        return self.minQ is 0 and self.maxQ <= 1

    # Name of the "present" variable for this element.
    def presentVarName(self):
        return "%s_present" % (self.varName())

    # Declaration of the "present" variable for this element.
    def presentVar(self):
        return ["size_t %s;" % self.presentVarName()]

    # Full "path" of the "present" variable for this element.
    def presentVarAccess(self):
        return self.accessAppend(self.presentVarName())

    # Whether to include a "count" variable for this element.
    def countVarCondition(self):
        return self.maxQ > 1

    # Name of the "count" variable for this element.
    def countVarName(self):
        return "%s_count" % (self.varName())

    # Declaration of the "count" variable for this element.
    def countVar(self):
        return ["size_t %s;" % self.countVarName()]

    # Full "path" of the "count" variable for this element.
    def countVarAccess(self):
        return self.accessAppend(self.countVarName())

    # Whether to include a "cbor" variable for this element.
    def isCbor(self):
        return (self.typeName() is not None) and ((self.type != "OTHER") or ((self.value not in entryTypeNames) and mytypes[self.value].isCbor()))

    # Whether to include a "cbor" variable for this element.
    def cborVarCondition(self):
        return ((self.cbor is not None) and self.cbor.isCbor())

    # Whether to include a "choice" variable for this element.
    def choiceVarCondition(self):
        return self.type is "UNION"

    # Name of the "choce" variable for this element.
    def choiceVarName(self):
        return self.varName() + "_choice"

    # Declaration of the "choice" variable for this element.
    def choiceVar(self):
        var = self.enclose("enum", [val.varName() + "," for val in self.value])
        var[-1] += f" {self.choiceVarName()};"
        return var

    # Full "path" of the "choice" variable for this element.
    def choiceVarAccess(self):
        return self.accessAppend(self.choiceVarName(), full=False)

    # Whether to include a "key" variable for this element.
    def keyVarCondition(self):
        return self.key is not None

    # Whether this value adds any repeated elements by itself. I.e. excluding multiple elements from children.
    def selfRepeatedMultiVarCondition(self):
        return (self.keyVarCondition()
                or self.cborVarCondition()
                or self.choiceVarCondition())

    # Whether this element's actual value has multiple members.
    def multiValCondition(self):
        return (self.type in ["LIST", "MAP", "GROUP", "UNION"]
                and (len(self.value) > 1
                     or self.value[0].multiMember()))

    # Whether any extra variables are to be included for this element for each repetition.
    def repeatedMultiVarCondition(self):
        return self.selfRepeatedMultiVarCondition() or self.multiValCondition()

    # Whether any extra variables are to be included for this element outside of repetitions.
    def multiVarCondition(self):
        return self.presentVarCondition() or self.countVarCondition()

    # Whether this element must involve a call to multi_decode(), i.e. unless it's repeated exactly once.
    def multiDecodeCondition(self):
        return self.minQ != 1 or self.maxQ != 1

    # Name of the decoder function for this element.
    def decodeFuncName(self):
        return "decode%s" % self.varName()

    # Name of the decoder function for the repeated part of this element.
    def repeatedDecodeFuncName(self):
        return "decode_repeated%s" % self.varName()

    # Declaration of the variables of all children.
    def childDeclarations(self):
        decl = [line for child in self.value for line in child.fullDeclaration()]
        return decl

    # Declaration of the variables of all children.
    def childSingleDeclarations(self):
        decl = [line for child in self.value for line in child.addVarName(child.singleVarType())]
        return decl

    # Enclose a list of declarations in a block (struct, union or enum).
    def enclose(self, ingress, declaration):
        return [f"{ingress} {{"] + [self.indentation + line for line in declaration] + ["}"]

    # Type declaration for unions.
    def unionType(self):
        declaration = self.enclose("union", self.childSingleDeclarations())
        return declaration

    # Recursively set the access prefix for this element and all its children.
    def setAccessPrefix(self, prefix):
        self.accessPrefix = prefix
        if self.type in ["LIST", "MAP", "GROUP"]:
            list(map(lambda child: child.setAccessPrefix(self.varAccess()), self.value))
        if self.type is "UNION":
            list(map(lambda child: child.setAccessPrefix(self.accessAppendDelim(self.valAccess(), self.accessDelimiter, child.varName()) if child.multiMember() else self.valAccess()), self.value))
        if self.key is not None:
            self.key.setAccessPrefix(self.varAccess())
        if self.cborVarCondition():
            self.cbor.setAccessPrefix(self.varAccess())

    # Whether this type has multiple member variables.
    def multiMember(self):
        return self.multiVarCondition() or self.repeatedMultiVarCondition()

    # Take a multi member type name and create a variable declaration. Make it an array if the element is repeated.
    def addVarName(self, varType):
        if (varType != []):
            assert(varType[-1][-1] == "}" or len(varType) == 1), f"Expected single var: {varType!r}"
            varType[-1] += " %s%s;" % (self.varName(), "[%d]" % self.maxQ if self.maxQ != 1 else "")
        return varType

    # The type for this element as a member variable.
    def varType(self):
        if not self.multiValCondition() and self.valTypeName() is not None:
            return [self.valTypeName()]
        elif self.type is "UNION":
            return self.unionType()
        return []

    # Declaration of the repeated part of this element.
    def repeatedDeclaration(self):
        varType = self.varType()
        multiVar = False

        decl = self.addVarName(varType)

        if self.type in ["LIST", "MAP", "GROUP"]:
            decl += self.childDeclarations()
            multiVar = len(decl) > 1

        if self.keyVarCondition():
            keyVar = self.key.fullDeclaration()
            decl = keyVar + decl
            multiVar = keyVar is not []

        if self.choiceVarCondition():
            choiceVar = self.choiceVar()
            decl += choiceVar
            multiVar = choiceVar is not []

        if self.cborVarCondition():
            cborVar = self.cbor.fullDeclaration()
            decl += cborVar
            multiVar = cborVar is not []

        if self.type not in ["LIST", "MAP", "GROUP"] or len(self.value) <= 1:
            # This assert isn't accurate for value lists with NIL or ANY members.
            assert multiVar == self.repeatedMultiVarCondition(), f"""type: {self.type}
            multivar is {multiVar} while
            self.repeatedMultiVarCondition() is {self.repeatedMultiVarCondition()} for
            decl {decl}
            self.keyVarCondition() is {self.keyVarCondition()}
            self.key is {self.key}
            self.cborVarCondition() is {self.cborVarCondition()}
            self.choiceVarCondition() is {self.choiceVarCondition()}
            self.value is {self.value}"""
        return decl

    # Declaration of the full type for this element.
    def fullDeclaration(self):
        multiVar = False

        if self.multiVarCondition():
            decl = self.addVarName([self.repeatedTypeName()] if self.repeatedTypeName() is not None else [])
        else:
            decl = self.repeatedDeclaration()

        if self.countVarCondition():
            countVar = self.countVar()
            decl += countVar
            multiVar = countVar is not []

        if self.presentVarCondition():
            presentVar = self.presentVar()
            decl += presentVar
            multiVar = presentVar is not []

        assert multiVar == self.multiVarCondition()
        return decl

    # Return the type definition of this element. If there are multiple variables, wrap them in a struct so the function
    # always returns a single type with no name.
    # If full is False, only repeated part is used.
    def singleVarType(self, full=True):
        if self.multiMember():
            return self.enclose("struct", self.fullDeclaration() if full else self.repeatedDeclaration())
        else:
            return self.varType()

    # Whether this element needs a check (memcmp) for a string value.
    def expectedStringCondition(self):
        return self.type in ["BSTR", "TSTR"] and not not self.value

    # Whether this element should have a typedef in the code.
    def typeDefCondition(self):
        if (self in mytypes.values() and self.multiMember()):
            return True
        return False

    # Whether this type needs a typedef for its repeated part.
    def repeatedTypeDefCondition(self):
        if self.repeatedMultiVarCondition() and self.multiVarCondition():
            return True
        return False

    # Return the type definition of this element, and all its children + key + cbor.
    def typeDef(self):
        retval = []
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            retval.extend([elem for typedef in [child.typeDef() for child in self.value] for elem in typedef])
        if self.cborVarCondition():
            retval.extend(self.cbor.typeDef())
        if self.keyVarCondition():
            retval.extend(self.key.typeDef())
        if self.type is "OTHER":
            retval.extend(mytypes[self.value].typeDef())
        if self.repeatedTypeDefCondition():
            retval.extend([(self.singleVarType(full=False), self.repeatedTypeName())])
        if self.typeDefCondition():
            retval.extend([(self.singleVarType(), self.typeName())])
        return retval

    # The variable to use as the running element count for this variable. Since an instance of this variable must be
    # kept for each level of the data structure, sometimes a function must keep an array of such counts.
    def elemCountVar(self, indirection = None):
        if indirection is None:
            indirection = self.countIndirection
        if indirection is 0:
            return "p_elem_count"
        else:
            return "(&elem_count[%d])" % (indirection - 1)

    # Return a code snippet that sets the relevant element count and max count variables. This is used when entering
    # a new level of the data structure.
    def setElemCounts(self, newMax):
        return ("(%s)" % " || ".join((
            "(*%s = %s)" % (self.elemCountVar(self.countIndirection + 1), newMax),
            "1"
        )))

    # Return an argument list for a function call to a decoder function.
    def decodeArgs(self, res, min_val, max_val, countIndirection=None):
        elemCountVar = self.elemCountVar(countIndirection)
        return "pp_payload, p_payload_end, %s, %s, %s, %s" % (elemCountVar,
                                                              "&(%s)" % res if res is not "NULL" else res,
                                                              min_val,
                                                              max_val)

    # Return the code that calls a decoder function with a given set of arguments.
    def decodeStatement(self, func, *args, **kwargs):
        return "(%s(%s))" % (func, self.decodeArgs(*args, **kwargs))

    # Return a code snippet that assigns a variable the address of another variable, or to NULL.
    def valOrNull(self, value, varname):
        return "(%s=%d) ? &%s : &%s" % (varname, value, varname, varname) if value is not None else "NULL"

    # Assign the min_value variable.
    def minValOrNull(self, value):
        return self.valOrNull(value, "min_value")

    # Assign the max_value variable.
    def maxValOrNull(self, value):
        return self.valOrNull(value, "max_value")

    # Return the function name and arguments to call to decode this element. Only used when this element DOESN'T define
    # its own decoder function (when it's a primitive type, for which functions already exist, or when the function is
    # defined elsewhere ("OTHER"))
    def singleFuncPrim(self):
        retval = {
            "INT":   lambda: ["intx32_decode", self.valAccess(), self.minValOrNull(self.minValue), self.maxValOrNull(self.maxValue)],
            "UINT":  lambda: ["uintx32_decode", self.valAccess(), self.minValOrNull(self.minValue), self.maxValOrNull(self.maxValue)],
            "NINT":  lambda: ["intx32_decode", self.valAccess(), self.minValOrNull(self.minValue), self.maxValOrNull(self.maxValue)],
            "FLOAT": lambda: ["float_decode", self.valAccess(), self.minValOrNull(self.minValue), self.maxValOrNull(self.maxValue)],
            "BSTR":  lambda: ["strx_decode" if not self.cborVarCondition() else "strx_start_decode", self.valAccess(), self.minValOrNull(self.minSize), self.maxValOrNull(self.maxSize)],
            "TSTR":  lambda: ["strx_decode", self.valAccess(), self.minValOrNull(self.minSize), self.maxValOrNull(self.maxSize)],
            "BOOL":  lambda: ["boolx_decode", self.valAccess(), self.minValOrNull(1 if self.value else 0), self.maxValOrNull(0 if self.value == False else 1)],
            "NIL":   lambda: ["primx_decode", "NULL", self.minValOrNull(22), self.maxValOrNull(22)],
            "ANY":   lambda: ["any_decode", "NULL", "NULL", "NULL"],
            "LIST":  lambda: self.value[0].singleFunc(),
            "OTHER": lambda: listReplaceIfNotNull(mytypes[self.value].singleFunc(), 1, self.valAccess()),
        }[self.type]()
        return retval

    # Return the function name and arguments to call to decode this element. Only used when this element has its own
    # decode function
    def singleFuncImpl(self, full=True):
        retval = (self.decodeFuncName() if full else self.repeatedDecodeFuncName(), self.varAccess() if full else self.valAccess(), "NULL", "NULL")
        return retval

    # Whether this element needs its own decoder function.
    def singleFuncImplCondition(self):
        retval = (False or self.key or self.cborVarCondition() or self.expectedStringCondition() or self.typeDefCondition())
        return retval

    # Whether this element needs its own decoder function.
    def repeatedSingleFuncImplCondition(self):
        return self.repeatedTypeDefCondition()

    # Return the function name and arguments to call to decode this element.
    def singleFunc(self):
        if self.singleFuncImplCondition():
            return self.singleFuncImpl()
        else:
            return self.singleFuncPrim()

    # Return the function name and arguments to call to decode the repeated part of this element.
    def repeatedSingleFunc(self):
        if self.repeatedSingleFuncImplCondition():
            return self.singleFuncImpl(full=False)
        else:
            return self.singleFuncPrim()

    # Return a number indicating how many other elements this element depends on. Used putting functions and typedefs
    # in the right order.
    def dependsOn(self):
        retvals = [1]

        if not self.dependsOnCall:
            self.dependsOnCall = True
            if self.cborVarCondition():
                retvals.append(self.cbor.dependsOn())
            if self.key:
                retvals.append(self.key.dependsOn())
            if self.type is "OTHER":
                retvals.append(1 + mytypes[self.value].dependsOn())
            if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
                retvals.extend(child.dependsOn() for child in self.value)
            self.dependsOnCall = False

        return max(retvals)

    # Make a string from the list returned by singleFuncPrim()
    def decodeSingleFuncPrim(self):
        return self.decodeStatement(*self.singleFuncPrim())

    # Return the full code needed to decode a "BSTR" or "TSTR" element.
    def decodeStr(self):
        assert self.type in ["BSTR", "TSTR"], "Expected string type."
        return self.decodeSingleFuncPrim() +\
            ("&& !memcmp(\"{0}\", {1}.value, {1}.len)".format(self.value, self.valAccess()) if self.expectedStringCondition() else "")


    # Recursively sum the total minimum and maximum element count for this element.
    def listCounts(self):
        return {
            "INT": lambda: (self.minQ, self.maxQ),
            "UINT": lambda: (self.minQ, self.maxQ),
            "NINT": lambda: (self.minQ, self.maxQ),
            "FLOAT": lambda: (self.minQ, self.maxQ),
            "BSTR": lambda: (self.minQ, self.maxQ),
            "TSTR": lambda: (self.minQ, self.maxQ),
            "BOOL": lambda: (self.minQ, self.maxQ),
            "NIL": lambda: (self.minQ, self.maxQ),
            "ANY": lambda: (self.minQ, self.maxQ),
            "LIST": lambda: (self.minQ, self.maxQ), # Lists are their own element
            "MAP": lambda: (self.minQ, self.maxQ), # Maps are their own element
            "GROUP": lambda: (self.minQ*sum((child.minQ for child in self.value)), self.maxQ*sum((child.maxQ for child in self.value))),
            "UNION": lambda: (self.minQ*min((child.minQ for child in self.value)), self.maxQ*max((child.maxQ for child in self.value))),
            "OTHER": lambda: (q1*q2 for q1, q2 in zip((self.minQ, self.maxQ), mytypes[self.value].listCounts())),
        }[self.type]()

    # Return the full code needed to decode a "LIST" or "MAP" element with children.
    def decodeList(self):
        assert self.type in ["LIST", "MAP"], "Expected LIST or MAP type, was %s." % self.type
        minCounts, maxCounts = zip(*(child.listCounts() for child in self.value)) if self.value else ((0,), (0,))
        return "(%s)" % (self.newl_ind + "&& ").join((self.decodeStatement("list_start_decode",
                                                         "temp_elem_count",
                                                         str(sum(minCounts)),
                                                         str(sum(maxCounts))),)
                                     + (self.setElemCounts("(%s * %d)" % ("temp_elem_count", 2 if self.type is "MAP" else 1)),)
                                     + tuple(child.fullDecode(self.countIndirection + 1) for child in self.value))

    # Return the full code needed to decode a "GROUP" element's children.
    def decodeGroup(self):
        assert self.type in ["GROUP"], "Expected GROUP type."
        return "(%s)" % (self.newl_ind + "&& ").join((child.fullDecode(self.countIndirection) for child in self.value))

    # Return the full code needed to decode a "UNION" element's children.
    def decodeUnion(self):
        assert self.type in ["UNION"], "Expected UNION type."
        return "((p_payload_bak = *pp_payload) && ((elem_count_bak = *%s) || 1) && (%s))" % (self.elemCountVar(), (self.newl_ind + "|| ").join(("(%s && %s && ((%s = %s) || 1))" % ("(*pp_payload = p_payload_bak) && ((*%s = elem_count_bak) || 1)" % self.elemCountVar(), child.fullDecode(self.countIndirection), self.choiceVarAccess(), child.varName()) for child in self.value)))

    # Return the full code needed to decode an "OTHER" element. This fetches the code of the type referenced by self.value.
    def decodeOther(self):
        assert self.type in ["OTHER"], "Expected OTHER type."
        return mytypes[self.value].decodeStatement(*self.singleFuncPrim(), countIndirection=self.countIndirection)

    # Return the full code needed to decode this element, including children, key and cbor, excluding repetitions.
    def repeatedDecode(self):
        decoder = {
            "INT": self.decodeSingleFuncPrim,
            "UINT": self.decodeSingleFuncPrim,
            "NINT": self.decodeSingleFuncPrim,
            "FLOAT": self.decodeSingleFuncPrim,
            "BSTR": self.decodeStr,
            "TSTR": self.decodeStr,
            "BOOL": self.decodeSingleFuncPrim,
            "NIL": self.decodeSingleFuncPrim,
            "ANY": self.decodeSingleFuncPrim,
            "LIST": self.decodeList,
            "MAP": self.decodeList,
            "GROUP": self.decodeGroup,
            "UNION": self.decodeUnion,
            "OTHER": self.decodeOther,
        }[self.type]()
        if self.key or self.cbor:
            args = ([self.key.fullDecode(self.countIndirection)] if self.key is not None else [])\
                 + ([decoder])\
                 + ([self.setElemCounts(str(self.maxQ)), self.cbor.fullDecode(self.countIndirection + 1)] if self.cborVarCondition() else [])
            decoder = "(%s)" % ((self.newl_ind + "&& ").join(args),)
        return decoder


    # Code for the size of the repeated part of this element.
    def resultLen(self):
        if self.type is "ANY":
            return "0"
        else:
            return "sizeof(%s)" % self.repeatedTypeName()

    # Return the full code needed to decode this element, including children, key, cbor, and repetitions.
    def fullDecode(self, countIndirection):
        if self.multiDecodeCondition():
            func, *args = self.repeatedSingleFunc()
            return ("multi_decode(%s, %s, &%s, (void*)%s, %s, %s)" % (self.minQ,
                                                          self.maxQ,
                                                          self.countVarAccess() if self.countVarCondition() else self.presentVarAccess(),
                                                          func,
                                                          self.decodeArgs(*args,
                                                                         countIndirection),
                                                          self.resultLen()))
        else:
            self.countIndirection = countIndirection
            return self.repeatedDecode()

    # Return the body of the decoder function for this element.
    def decode(self):
        return self.repeatedDecode()


    # Recursively return a list of the bodies of the decoder functions for this element and its children + key + cbor.
    def decoders(self):
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            for child in self.value:
                for decoder in child.decoders():
                    yield decoder
        if self.cbor:
            for decoder in self.cbor.decoders():
                yield decoder
        if self.key:
            for decoder in self.key.decoders():
                yield decoder
        if self.type is "OTHER" and self.value not in entryTypeNames:
            for decoder in mytypes[self.value].decoders():
                yield decoder
        if self.repeatedSingleFuncImplCondition() and self.countIndirection is 0:
            yield (self.decode(), self.repeatedDecodeFuncName(), self.repeatedTypeName(), self.maxCountIndirection(0))
        if ((not self.type is "OTHER") or self.repeatedMultiVarCondition()) and (self.singleFuncImplCondition() and self.countIndirection is 0):
            decodeBody = self.decode()
            yield (decodeBody, self.decodeFuncName(), self.typeName(), self.maxCountIndirection(0))

    # Find the maximum indirection for the element count variable for this element.
    def maxCountIndirection(self, current):
        if self.countIndirection < current:
            return current

        candidates = [self.countIndirection]

        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            candidates += [child.maxCountIndirection(self.countIndirection) for child in self.value]
            if any([child.minQ != 1 or child.maxQ != 1 for child in self.value]):
                candidates.append(self.countIndirection + 1)
        if self.cborVarCondition():
            candidates.append(self.cbor.maxCountIndirection(self.countIndirection))
        if self.key:
            candidates.append(self.key.maxCountIndirection(self.countIndirection))
        if self.multiDecodeCondition():
            candidates.append(current + 1)
        return max(candidates)


mytypes = {}
entryTypeNames = []

# Consumes and parses a single CDDL type, returning a TYPE_decoder_generator_CBOR instance.
def parseSingle(instr, single = False, base_name = None):
    value = TYPE_decoder_generator_CBOR(base_name=base_name)
    instr = value.getValue(instr).strip()
    return (value, instr)

# Parses entire instr and returns a list of TYPE_decoder_generator_CBOR instances.
def parse(instr, single = False):
    instr = instr.strip()
    values = []
    while instr != '':
        (value, instr) = parseSingle(instr)
        values.append(value)
    return values

# Returns a dict containing multiple typename=>string
def getTypes(instr):
    typeRegex = "(\s*?\$?\$?([\w-]+)\s*(\/{0,2})=\s*(.*?)(?=(\Z|\s*\$?\$?[\w-]+\s*\/{0,2}=(?!\>))))"
    result = defaultdict(lambda: "")
    types = [(key, value, slashes) for (_1, key, slashes, value, _2) in findall(typeRegex, instr, S | M)]
    for key, value, slashes in types:
        if slashes:
            result[key] += slashes
            result[key] += value
            result[key] = result[key].lstrip(slashes) # strip from front
        else:
            result[key] = value
    return dict(result)

# Strip CDDL comments (';') from the string.
def stripComments(instr):
    commentRegex = r"\;.*?\n"
    return sub(commentRegex, '', instr)

# Return a list of typedefs for all defined types, with duplicate typedefs removed.
def uniqueTypes(types):
    typeNames={}
    outTypes = []
    for mtype in types:
        for typeDef in mtype.typeDef():
            typeName = typeDef[1]
            if typeName not in typeNames.keys():
                typeNames[typeName] = typeDef[0]
                outTypes.append(typeDef)
            else:
                assert (''.join(typeNames[typeName]) == ''.join(typeDef[0])),\
                       ("Two elements share the type name %s, but their implementations are not identical. "
                      + "Please change one or both names. One of them is %s") % (typeName, pprint(mtype.typeDef()))
    return outTypes


# Return a list of decoder functions for all defined types, with duplicate functions removed.
def uniqueFuncs(types):
    funcNames={}
    outTypes = []
    for mtype in types:
        decoders = list(mtype.decoders())
        for funcType in decoders:
            funcdecode = funcType[0]
            funcName = funcType[1]
            if funcName not in funcNames.keys():
                funcNames[funcName] = funcType
                outTypes.append(funcType)
            elif funcName in funcNames.keys():
                assert funcNames[funcName][0] == funcdecode,\
                       ("Two elements share the function name %s, but their implementations are not identical. "
                      + "Please change one or both names.\n\n%s\n\n%s") % (funcName, funcNames[funcName][0], funcdecode)

    return outTypes

# Return a list of decoder functions for all defined types, with unused functions removed.
def usedFuncs(types, entryTypes):
    entryTypes = [(funcType.decode(), funcType.decodeFuncName(), funcType.typeName(), funcType.maxCountIndirection(0)) for funcType in entryTypes]
    outTypes = [funcType for funcType in entryTypes]
    fullCode = "".join([funcType[0] for funcType in entryTypes])
    for funcType in reversed(types):
        funcName = funcType[1]
        if funcType not in entryTypes and search(r"%s\W" % funcName, fullCode):
            fullCode += funcType[0]
            outTypes.append(funcType)
    return list(reversed(outTypes))

# Return a list of typedefs for all defined types, with unused types removed.
def usedTypes(typeDefs, entryTypes):
    outTypes = [typeDef for typeDef in entryTypes]
    fullCode = "".join(["".join(typeDef[0]) for typeDef in entryTypes])
    for typeDef in reversed(typeDefs):
        typeName = typeDef[1]
        if typeDef not in entryTypes and search(r"%s\W" % typeName, fullCode):
            fullCode += "".join(typeDef[0])
            outTypes.append(typeDef)
    return list(reversed(outTypes))



def parse_args():
    parser = ArgumentParser(
        description=
'''Parse a CDDL file and produce C code that validates and decodes CBOR.
The output from this script is a C file and a header file. The header file
contains typedefs for all the types specified in the cddl input file, as well
as declarations to decode functions for the types designated as entry types when
running the script. The c file contains all the code for decoding and validating
the types in the CDDL input file. All types are validated as they are decoded.

Where a `bstr .cbor <Type>` is specified in the CDDL, AND the Type is an entry
type, the decoder will not decode the string, only provide a pointer into the
payload buffer. This is useful to reduce the size of typedefs, or to break up
decoding. Using this mechanism is necessary when the CDDL contains self-
referencing types, since the C type cannot be self referencing.

The verbose flag turns on printing, both in the CBOR primitive decoding, and in
the generated code. This is meant for debugging when decoding fails since the
point at which decoding failed is not returned to the caller in normal operation.

This script requires 'regex' for lookaround functionality not present in 're'.''',
        formatter_class=RawDescriptionHelpFormatter)

    parser.add_argument("-i", "--input", required=True, type=str,
                        help="Path to input CDDL file.")
    parser.add_argument("--output-c", "--oc", required=True, type=str,
                        help="Path to output C file.")
    parser.add_argument("--output-h", "--oh", required=True, type=str,
                        help="Path to output header file.")
    parser.add_argument("-t","--entry-types", required=True, type=str, nargs="+",
                        help="Names of the types which should have their decode functions exposed.")
    parser.add_argument("-v","--verbose", required=False, action="store_true", default=False,
                        help="Whether the C code should include printing.")

    return parser.parse_args()


# Render a single decoding function with signature and body.
def render_function(decoder):
    body = decoder[0]
    return f"""
static bool {decoder[1]}(
		uint8_t const ** pp_payload, uint8_t const * const p_payload_end,
		size_t * const p_elem_count, void * p_result, void * p_min_value,
		void * p_max_value)
{{
	cbor_decode_print("{ decoder[1] }\\n");
	{"size_t temp_elem_count;" if "temp_elem_count" in body else ""}
	{f"size_t elem_count[{ decoder[3] }];" if "elem_count[" in body else ""}
	{"uint32_t current_list_num;" if "current_list_num" in body else ""}
	{"uint8_t const * p_payload_bak;" if "p_payload_bak" in body else ""}
	{"size_t elem_count_bak;" if "elem_count_bak" in body else ""}
	{"uint32_t min_value;" if "min_value" in body else ""}
	{"uint32_t max_value;" if "max_value" in body else ""}
	{decoder[2]}* p_type_result = ({decoder[2]}*)p_result;

	if (!{ body })
	{{
		cbor_decode_trace();
		return false;
	}}
	return true;
}}""".replace("	\n", "") # call replace() to remove empty lines.


# Render a single entry function (API function) with signature and body.
def render_entry_function(decoder):
    return f"""
bool cbor_{decoder.decodeFuncName()}(const uint8_t * p_payload, size_t payload_len, {decoder.typeName()} * p_result)
{{
	size_t elem_count = 1;
	return {decoder.decodeFuncName()}(&p_payload, p_payload + payload_len, &elem_count, p_result, NULL, NULL);
}}"""


# Render the entire generated C file contents.
def render_c_file(functions, header_file_name, entryTypes, verbose):
    return \
f"""#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>

{"#define CDDL_CBOR_VERBOSE" if verbose else ""}
#include "cbor_decode.h"
#include "{header_file_name}"

{linesep.join([render_function(decoder) for decoder in functions])}

{linesep.join([render_entry_function(decoder) for decoder in entryTypes])}
"""


# Render the entire generated header file contents.
def render_h_file(typeDefs, header_guard, entryTypes):
    return \
f"""#ifndef {header_guard}
#define {header_guard}

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"


{(linesep+linesep).join([f"typedef {linesep.join(typedef[0])} {typedef[1]};" for typedef in typeDefs])}

{(linesep+linesep).join([f"bool cbor_{decoder.decodeFuncName()}(const uint8_t * p_payload, size_t payload_len, {decoder.typeName()} * p_result);" for decoder in entryTypes])}

#endif // {header_guard}
"""


if __name__ == "__main__":
    # Parse command line arguments.
    args = parse_args()
    entryTypeNames = args.entry_types

    # Read CDDL file. mytypes will become a dict {name:str => definition:str} for all types in the CDDL file.
    with open(args.input, 'r') as f:
        mytypes = getTypes(stripComments(f.read()))

    # Parse the definitions, replacing the each string with a TYPE_decoder_generator_CBOR instance.
    for mytype in mytypes:
        (parsed, _) = parseSingle(mytypes[mytype].replace("\n", " "), "", base_name = mytype)
        mytypes[mytype] = parsed.doFlatten(parsed)[0]
        mytypes[mytype].setIdPrefix("")

        counter(True)

    # set access prefix (struct access paths) for all the definitions.
    for mytype in mytypes:
        mytypes[mytype].setAccessPrefix("(*p_type_result)")

    # Postvalidate all the definitions.
    for mytype in mytypes:
        mytypes[mytype].postValidate()

    # Parsing is done, pretty print the result.
    # pprint(mytypes)

    # Prepare the list of types that will have exposed decoder functions.
    entryTypes = [mytypes[entry] for entry in entryTypeNames]

    # Sort type definitions so the typedefs will come in the correct order in the header file and the function in the
    # correct order in the c file.
    sortedTypes = list(sorted(entryTypes, key=lambda type: type.dependsOn(), reverse=False))

    uFuncs = uniqueFuncs(sortedTypes)
    uFuncs = usedFuncs(uFuncs, entryTypes)
    uTypes = uniqueTypes(sortedTypes)

    # Create and populate the generated c and h file.
    makedirs("./" + path.dirname(args.output_c), exist_ok=True)
    with open(args.output_c, 'w') as f:
        print ("Writing to " + args.output_c)
        f.write(render_c_file(functions=uFuncs, header_file_name=path.basename(args.output_h), entryTypes=entryTypes, verbose=args.verbose))
    makedirs("./" + path.dirname(args.output_h), exist_ok=True)
    with open(args.output_h, 'w') as f:
        print ("Writing to " + args.output_h)
        f.write(render_h_file(typeDefs=uTypes, header_guard=path.basename(args.output_h).replace(".","_").upper() + "__", entryTypes=entryTypes))
