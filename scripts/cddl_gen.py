#!/usr/bin/env python3
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
from datetime import datetime

my_types = {}
entry_type_names = []


# Size of "additional" field if num is encoded as int
def sizeof(num):
    if num <= 23:
        return 0
    elif num < 0x100:
        return 1
    elif num < 0x10000:
        return 2
    elif num < 0x100000000:
        return 4
    elif num < 0x10000000000000000:
        return 8
    else:
        raise ValueError("Number too large (more than 64 bits).")


# Global verbose flag
verbose_flag = False


# Print only if verbose
def verbose_print(*things):
    if verbose_flag:
        print(*things)


# Pretty print only if verbose
def verbose_pprint(*things):
    if verbose_flag:
        pprint(*things)


global_counter = 0


# Retrieve a unique id.
def counter(reset=False):
    global global_counter
    if reset:
        global_counter = 0
        return global_counter
    global_counter += 1
    return global_counter


# Replace an element in a list or tuple and return the list. For use in
# lambdas.
def list_replace_if_not_null(lst, i, r):
    if lst[i] == "NULL":
        return lst
    if isinstance(lst, tuple):
        convert = tuple
        lst = list(lst)
    else:
        assert isinstance(lst, list)
        convert = list
    lst[i] = r
    return convert(lst)


def do_flatten(to_flatten, allow_multi=False):
    to_flatten.flatten()
    if to_flatten.type in ["GROUP", "UNION"]\
            and (len(to_flatten.value) == 1)\
            and (not (to_flatten.key and to_flatten.value[0].key)):
        to_flatten.value[0].minQ *= to_flatten.minQ
        to_flatten.value[0].maxQ *= to_flatten.maxQ
        if not to_flatten.value[0].label:
            to_flatten.value[0].label = to_flatten.label
        if not to_flatten.value[0].key:
            to_flatten.value[0].key = to_flatten.key
        return to_flatten.value
    elif allow_multi and to_flatten.type in ["GROUP"] and to_flatten.minQ == 1 and to_flatten.maxQ == 1:
        return to_flatten.value
    else:
        return [to_flatten]


# Return a code snippet that assigns the value to a variable var_name and
# returns pointer to the variable, or returns NULL if the value is None.
def val_or_null(value, var_name):
    return "(%s=%d, &%s)" % (var_name, value,
                                var_name) if value is not None else "NULL"


# Assign the min_value variable.
def min_val_or_null(value):
    return val_or_null(value, "min_value")


# Assign the max_value variable.
def max_val_or_null(value):
    return val_or_null(value, "max_value")


# Return an argument list for a function call to a decoder function.
def decode_args(res, min_val, max_val):
    return "p_state, %s, %s, %s" % (
        "&(%s)" % res if res != "NULL" else res, min_val, max_val)

# Return the code that calls a decoder function with a given set of
# arguments.
def decode_statement(func, *sargs, **kwargs):
    return "(%s(%s))" % (func, decode_args(*sargs, **kwargs))


def add_semicolon(decl):
    if len(decl) != 0 and decl[-1][-1] != ";":
        decl[-1] += ";"
    return decl

# Class for parsing CDDL. One instance represents one CBOR data item, with a few caveats:
#  - For repeated data, one instance represents all repetitions.
#  - For "OTHER" types, one instance points to another type definition.
#  - For "GROUP" and "UNION" types, there is no separate data item for the instance.
class CddlParser:

    # The default maximum number of repeated elements.
    DEFAULT_MAXQ = 3

    def __init__(self):
        self.id_prefix = "temp_" + str(counter())
        self.id_num = None  # Unique ID number. Only populated if needed.
        # The value of the data item. Has different meaning for different
        # types.
        self.value = None
        self.max_value = None  # Maximum value. Only used for numbers and bools.
        self.min_value = None  # Minimum value. Only used for numbers and bools.
        # The readable label associated with the element in the CDDL.
        self.label = None
        self.minQ = 1  # The minimum number of times this element is repeated.
        self.maxQ = 1  # The maximum number of times this element is repeated.
        # The size of the element. Only used for integers, byte strings, and
        # text strings.
        self.size = None
        self.min_size = None  # Minimum size.
        self.max_size = None  # Maximum size.
        # Key element. Only for children of "MAP" elements. self.key is of the
        # same class as self.
        self.key = None
        # The element specified via.cbor or.cborseq(only for byte
        # strings).self.cbor is of the same class as self.
        self.cbor = None
        # The CDDL string used to determine the minQ and maxQ. Not used after
        # minQ and maxQ are determined.
        self.quantifier = None
        # The "type" of the element. This follows the CBOR types loosely, but are more related to CDDL
        # concepts. The possible types are "INT", "UINT", "NINT", "FLOAT", "BSTR", "TSTR", "BOOL",  "NIL", "LIST",
        # "MAP","GROUP", "UNION" and "OTHER". "OTHER" represents a CDDL type defined with '='.
        self.type = None
        self.match_str = ""

    def set_id_prefix(self, id_prefix):
        self.id_prefix = id_prefix
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            for child in self.value:
                child.set_id_prefix(self.child_base_id())
        if self.cbor:
            self.cbor.set_id_prefix(self.child_base_id())
        if self.key:
            self.key.set_id_prefix(self.child_base_id())

    def id(self):
        return "%s_%s" % (self.id_prefix, self.get_id_num())

    # Id to pass to children for them to use as basis for their id/base name.
    def child_base_id(self):
        return self.id()

    def get_id_num(self):
        if self.id_num is None:
            self.id_num = counter()
        return self.id_num

    # Human readable representation.
    def mrepr(self, newline):
        reprstr = ''
        if self.quantifier:
            reprstr += self.quantifier
        if self.label:
            reprstr += self.label + ':'
        if self.key:
            reprstr += repr(self.key) + " => "
        reprstr += self.type
        if self.size:
            reprstr += '(%d)' % self.size
        if newline:
            reprstr += '\n'
        if self.value:
            reprstr += pformat(self.value, indent=4, width=1)
        if self.cbor:
            reprstr += " cbor: " + repr(self.cbor)
        return reprstr.replace('\n', '\n    ')

    def flatten(self):
        new_value = []
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            for child in self.value:
                new_value.extend(
                    do_flatten(
                        child,
                        allow_multi=self.type != "UNION"))
            self.value = new_value
        if self.key:
            self.key = do_flatten(self.key)[0]
        if self.cbor:
            self.cbor = do_flatten(self.cbor)[0]

    # Set the self.type and self.value of this element. For use during CDDL
    # parsing.
    def type_and_value(self, new_type, value_generator):
        if self.type is not None:
            raise TypeError(
                "Cannot have two values: %s, %s" %
                (self.type, new_type))
        if new_type is None:
            raise TypeError("Cannot set None as type")
        if new_type == "UNION" and self.value is not None:
            raise ValueError("Did not expect multiple parsed values for union")

        self.type = new_type

        value = value_generator()
        self.value = value

        if new_type in ["BSTR", "TSTR"]:
            if value is not None:
                self.set_size(len(value))
        if new_type in ["UINT", "NINT"]:
            if value is not None:
                self.size = sizeof(value)
                self.min_value = value
                self.max_value = value

    # Set the self.type and self.minValue and self.maxValue (or self.minSize and self.max_size depending on the type) of
    # this element. For use during CDDL parsing.
    def type_and_range(self, new_type, min_val, max_val):
        if new_type not in ["INT", "UINT", "NINT"]:
            raise TypeError(
                "Only integers (not %s) can have range" %
                (new_type,))
        if min_val > max_val:
            raise TypeError(
                "Range has larger minimum than maximum (min %d, max %d)" %
                (min_val, max_val))
        if min_val == max_val:
            return self.type_and_value(new_type, min_val)
        self.type = new_type
        self.min_value = min_val
        self.max_value = max_val
        if new_type in "UINT":
            self.set_size_range(sizeof(min_val), sizeof(max_val))
        if new_type == "NINT":
            self.set_size_range(sizeof(abs(max_val)), sizeof(abs(min_val)))
        if new_type == "INT":
            self.set_size_range(
                0, max(sizeof(abs(max_val)),
                       sizeof(abs(min_val))))

    # Set the self.value and self.size of this element. For use during CDDL
    # parsing.
    def type_value_size(self, new_type, value, size):
        self.type_and_value(new_type, value)
        self.set_size(size)

    # Set the self.label of this element. For use during CDDL parsing.
    def set_label(self, label):
        if self.type is not None:
            raise TypeError("Cannot have label after value: " + label)
        self.label = label

    # Set the self.quantifier, self.minQ, and self.maxQ of this element. For
    # use during CDDL parsing.
    def set_quantifier(self, quantifier):
        if self.type is not None:
            raise TypeError(
                "Cannot have quantifier after value: " +
                quantifier)

        quantifier_mapping = [
            (r"\?", lambda mo: (0, 1)),
            (r"\*", lambda mo: (0, None)),
            (r"\+", lambda mo: (1, None)),
            (r"(\d*)\*\*(\d*)", lambda mo: (int(mo.groups()[0] or 0), int(mo.groups()[1] or 0))),
        ]

        self.quantifier = quantifier
        for (reg, handler) in quantifier_mapping:
            match_obj = match(reg, quantifier)
            if match_obj:
                (self.minQ, self.maxQ) = handler(match_obj)
                if self.maxQ is None:
                    self.maxQ = self.DEFAULT_MAXQ
                return
        raise ValueError("invalid quantifier: %s" % quantifier)

    # Set the self.size of this element. This will also set the self.minValue and self.maxValue of UINT types.
    # For use during CDDL parsing.
    def set_size(self, size):
        if self.type is None:
            raise TypeError("Cannot have size before value: " + str(size))
        elif self.type == "UINT":
            return self.set_max_size(size)
        elif self.type not in ["INT", "UINT", "NINT", "BSTR", "TSTR"]:
            raise TypeError(".size cannot be applied to %s" % self.type)
        self.size = size

    # Set the self.minValue and self.maxValue or self.minSize and self.max_size of this element based on what values
    # can be contained within an integer of a certain size. For use during
    # CDDL parsing.
    def set_size_range(self, min_size, max_size):
        if min_size == max_size:
            return self.set_size(min_size)
        elif min_size > max_size:
            raise TypeError(
                "Invalid size range (min %d, max %d)" %
                (min_size, max_size))
        else:
            self.set_size(None)
        self.set_min_size(min_size)
        self.set_max_size(max_size)

    # Set self.minSize, and self.minValue if type is UINT.
    def set_min_size(self, minSize):
        if self.type is "UINT":
            self.minValue = 256**min(0, abs(minSize-1))
        self.minSize = minSize

    # Set self.max_size, and self.maxValue if type is UINT.
    def set_max_size(self, max_size):
        if self.type is "UINT" and max_size is not None:
            if max_size > 8:
                raise TypeError(
                    "Size too large for integer. size %d" %
                    max_size)
            self.maxValue = 256**max_size - 1
        self.max_size = max_size

    # Set the self.cbor of this element. For use during CDDL parsing.
    def set_cbor(self, cbor, cborseq):
        if self.type != "BSTR":
            raise TypeError(
                "%s must be used with bstr." %
                (".cborseq" if cborseq else ".cbor",))
        self.cbor = cbor
        if cborseq:
            self.cbor.maxQ = self.DEFAULT_MAXQ
        self.cbor.set_base_name("cbor")

    # Set the self.key of this element. For use during CDDL parsing.
    def set_key(self, key):
        if self.key is not None:
            raise TypeError("Cannot have two keys: " + key)
        if key.type == "GROUP":
            raise TypeError("A key cannot be a group")
        self.key = key
        self.key.set_base_name("key")

    # Set the self.label OR self.key of this element. In the CDDL "foo: bar", foo can be either a label or a key
    # depending on whether it is in a map. This code uses a slightly different method for choosing between label and
    # key. If the string is recognized as a type, it is treated as a key. For
    # use during CDDL parsing.
    def set_key_or_label(self, key_or_label):
        if key_or_label.type == "OTHER" and key_or_label.value not in my_types:
            self.set_label(key_or_label.value)
        else:
            if key_or_label.type == "OTHER" and self.label is None:
                self.set_label(key_or_label.value)
            self.set_key(key_or_label)

    # Append to the self.value of this element. Used with the "MAP", "LIST", "UNION", and "GROUP" types, which all have
    # a python list as self.value. The list represents the "children" of the
    # type. For use during CDDL parsing.
    def add_value(self, value):
        self.value.append(value)

    # Parse from the beginning of instr (string) until a full element has been parsed. self will become that element.
    # This function is recursive, so if a nested element ("MAP"/"LIST"/"UNION"/"GROUP") is encountered, this function
    # will create new instances and add them to self.value as a list. Likewise, if a key or cbor definition is
    # encountered, a new element will be created and assigned to self.key or self.cbor. When new elements are created,
    # getValue() is called on those elements, via parse().
    def get_value(self, instr):
        # The following regexes match different parts of the element. The order of the list is important because it
        # implements the operator precendence defined in the CDDL spec. Note that some regexes are inserted afterwards
        # because they involve a match of a concatenation of all the initial
        # regexes (with a '|' between each element).
        types = [
            (r'\A(?!\/\/).+?(?=\/\/)',
             lambda union_str:
             self.type_and_value("UNION", lambda: parse("(%s)" % union_str if ',' in union_str else union_str))),
            (r'\/\/\s*(?P<item>.+?)(?=\/\/|\Z)',
             lambda union_str: self.add_value(parse("(%s)" % union_str if ',' in union_str else union_str)[0])),
            (r'([+*?])',
             self.set_quantifier),
            (r'(\d*\*\*\d*)',
             self.set_quantifier),
            (r'uint(?!\w)',
             lambda _: self.type_and_value("UINT", lambda: None)),
            (r'nint(?!\w)',
             lambda _: self.type_and_value("NINT", lambda: None)),
            (r'int(?!\w)',
             lambda _: self.type_and_value("INT", lambda:None)),
            (r'float(?!\w)',
             lambda _: self.type_and_value("FLOAT", lambda: None)),
            (r'float16(?!\w)',
             lambda _: self.type_value_size("FLOAT", None, 2)),
            (r'float32(?!\w)',
             lambda _: self.type_value_size("FLOAT", None, 4)),
            (r'float64(?!\w)',
             lambda _: self.type_value_size("FLOAT", None, 8)),
            (r'\-?\d*\.\d+',
             lambda num: self.type_and_value("FLOAT", lambda: int(num))),
            (r'\d+\.\.\d+',
             lambda _range: self.type_and_range("UINT", *map(int, _range.split("..")))),
            (r'\-\d+\.\.\d+',
             lambda _range: self.type_and_range("INT", *map(int, _range.split("..")))),
            (r'\-\d+\.\.\-\d+',
             lambda _range: self.type_and_range("NINT", *map(int, _range.split("..")))),
            (r'\-\d+',
             lambda num: self.type_and_value("NINT", lambda: int(num))),
            (r'0[xbo]\w+',
             lambda num: self.type_and_value("UINT", lambda: int(num, 0))),
            (r'\d+',
             lambda num: self.type_and_value("UINT", lambda: int(num))),
            (r'bstr(?!\w)',
             lambda _: self.type_and_value("BSTR", lambda: None)),
            (r'tstr(?!\w)',
             lambda _: self.type_and_value("TSTR", lambda: None)),
            (r'\".*?\"(?<!\\)',
             lambda string: self.type_and_value("TSTR", lambda: string.strip('"'))),
            (r'\[(?P<item>(?>[^[\]]+|(?R))*)\]',
             lambda list_str: self.type_and_value("LIST", lambda: parse(list_str))),
            (r'\((?P<item>(?>[^\(\)]+|(?R))*)\)',
             lambda group_str: self.type_and_value("GROUP", lambda: parse(group_str))),
            (r'{(?P<item>(?>[^{}]+|(?R))*)}',
             lambda _map: self.type_and_value("MAP", lambda: parse(_map))),
            (r'bool(?!\w)',
             lambda _: self.type_and_value("BOOL", lambda: None)),
            (r'true(?!\w)',
             lambda _: self.type_and_value("BOOL", lambda: True)),
            (r'false(?!\w)',
             lambda _: self.type_and_value("BOOL", lambda: False)),
            (r'nil(?!\w)',
             lambda _: self.type_and_value("NIL", lambda: None)),
            (r'any(?!\w)',
             lambda _: self.type_and_value("ANY", lambda: None)),
            (r'(\$?\$?[\w-]+)',
             lambda other_str: self.type_and_value("OTHER", lambda: other_str.strip("$"))),
            (r'\.size \(?(?P<item>\d+\.\.\d+)\)?',
             lambda _range: self.set_size_range(*map(int, _range.split("..")))),
            (r'\.size \(?(?P<item>\d+)\)?',
             lambda size: self.set_size(int(size))),
            (r'\.cbor (?P<item>[\w-]+)',
             lambda type_str: self.set_cbor(parse(type_str)[0], False)),
            (r'\.cborseq (?P<item>[\w-]+)',
             lambda type_str: self.set_cbor(parse(type_str)[0], True))
        ]
        all_type_regex = '|'.join([regex for (regex, _) in types[3:]])
        for i in range(0, all_type_regex.count("item")):
            all_type_regex = all_type_regex.replace("item", "it%dem" % i, 1)
        types.insert(3, (r'(?P<item>'+all_type_regex+r')\s*\:',
                         lambda key_str: self.set_key_or_label(parse(key_str)[0])))
        types.insert(4, (r'(?P<item>'+all_type_regex+r')\s*\=\>',
                         lambda key_str: self.set_key(parse(key_str)[0])))
        types.insert(5, (r'(?P<item>(('+all_type_regex+r')\s*)+?)(?=\/)',
                         lambda union_str: self.type_and_value("UNION", lambda: parse(union_str))))
        types.insert(6, (r'\/\s*(?P<item>(('+all_type_regex+r')\s*)+?)(?=\/|\,|\Z)',
                         lambda union_str: self.add_value(parse(union_str)[0])))

        # Keep parsing until a comma, or to the end of the string.
        while instr != '' and instr[0] != ',':
            match_obj = None
            for (reg, handler) in types:
                match_obj = match(reg, instr)
                if match_obj:
                    try:
                        match_str = match_obj.group("item")
                    except IndexError:
                        match_str = match_obj.group(0)
                    try:
                        handler(match_str)
                    except Exception as e:
                        raise Exception("Failed while parsing this: '%s'" % match_str) from e
                    self.match_str += match_str
                    old_len = len(instr)
                    instr = sub(reg, '', instr, count=1).lstrip()
                    if old_len == len(instr):
                        raise Exception("empty match")
                    break

            if not match_obj:
                raise TypeError("Could not parse this: '%s'" % instr)

        instr = instr[1:]
        if not self.type:
            raise ValueError("No proper value while parsing: %s" % instr)

        # Return the unparsed part of the string.
        return instr

    # For checking whether this element has a key (i.e. to check that it is a valid "MAP" child).
    # This must have some recursion since CDDL allows the key to be hidden
    # behind layers of indirection.
    def elem_has_key(self):
        return self.key is not None\
            or (self.type == "OTHER" and my_types[self.value].elem_has_key())\
            or (self.type in ["GROUP", "UNION"] and all(child.elem_has_key() for child in self.value))

    # Function for performing validations that must be done after all parsing is complete. This is recursive, so
    # it will post_validate all its children + key + cbor.
    def post_validate(self):
        # Validation of this element.
        if self.type == "MAP":
            none_keys = [child for child in self.value if not child.elem_has_key()]
            if none_keys:
                raise TypeError("Map entry must have key: " + str(none_keys))
        if self.type == "OTHER":
            if self.value not in my_types.keys() or not isinstance(
                    my_types[self.value], type(self)):
                raise TypeError("%s has not been parsed." % self.value)
        if self.type == "LIST":
            for child in self.value[:-1]:
                if child.type == "ANY":
                    if child.minQ != child.maxQ:
                        raise TypeError(f"ambiguous quantity of 'any' is not supported in list, "
                                        + "except as last element:\n{str(child)}")
        if self.type == "UNION" and len(self.value) > 1:
            if any(((not child.key and child.type == "ANY") or (
                    child.key and child.key.type == "ANY")) for child in self.value):
                raise TypeError(
                    "'any' inside union is not supported since it would always be triggered.")

        # Validation of child elements.
        if self.type in ["MAP", "LIST", "UNION", "GROUP"]:
            for child in self.value:
                child.post_validate()
        if self.key:
            self.key.post_validate()
        if self.cbor:
            self.cbor.post_validate()

    def __repr__(self):
        return self.mrepr(False)


# Class for generating C code that decodes CBOR and validates it according
# to the CDDL.
class CodeGenerator(CddlParser):

    indentation = "\t"
    newl_ind = "\n" + indentation

    def __init__(self, base_name=None):
        super(CodeGenerator, self).__init__()
        # The prefix used for C code accessing this element, i.e. the struct
        # hierarchy leading
        self.accessPrefix = None
        # up to this element. This can change multiple times during generation to suit
        # different situations.
        # The delimiter used between elements in the accessPrefix.
        self.access_delimiter = "."
        # Used as a guard against endless recursion in self.dependsOn()
        self.dependsOnCall = False
        self.base_name = base_name  # Used as default for self.get_base_name()

    # Base name used for functions, variables, and typedefs.
    def get_base_name(self):
        return ((self.base_name or self.label
                 or (self.key.value if self.key and self.key.type in ["TSTR", "OTHER"] else None)
                 or (f"{self.value}_{self.type.lower()}" if self.type == "TSTR" and self.value is not None else None)
                 or (f"{self.type.lower()}{self.value}" if self.type in ["INT", "UINT"] and self.value is not None else None)
                 or (next((key for key, value in my_types.items() if value == self), None))
                 or ("_"+self.value if self.type == "OTHER" else None)
                 or ("_"+self.value[0].get_base_name()
                     if self.type in ["LIST", "GROUP"] and self.value is not None else None)
                 or (self.cbor.value if self.cbor and self.cbor.type in ["TSTR", "OTHER"] else None)
                 or self.type.lower()).replace("-", "_"))

    # Set an explicit base name for this element.
    def set_base_name(self, base_name):
        self.base_name = base_name

    # Add uniqueness to the base name.
    def id(self):
        return "%s%s" % ((self.id_prefix + "_")
                         if self.id_prefix != "" else "", self.get_base_name())

    # Base name if this element needs to declare a type.
    def raw_type_name(self):
        return "%s_t" % self.id()

    # Name of the type of this element's actual value variable.
    def val_type_name(self):
        if self.multi_val_condition():
            return self.raw_type_name()

        # Will fail runtime if we don't use lambda for type_name()
        # pylint: disable=unnecessary-lambda
        name = {
            "INT": lambda: "int32_t",
            "UINT": lambda: "uint32_t",
            "NINT": lambda: "int32_t",
            "FLOAT": lambda: "float_t",
            "BSTR": lambda: "cbor_string_type_t",
            "TSTR": lambda: "cbor_string_type_t",
            "BOOL": lambda: "bool",
            "NIL": lambda: None,
            "ANY": lambda: None,
            "LIST": lambda: self.value[0].type_name() if len(self.value) >= 1 else None,
            "MAP": lambda: self.value[0].type_name() if len(self.value) >= 1 else None,
            "GROUP": lambda: self.value[0].type_name() if len(self.value) >= 1 else None,
            "UNION": lambda: self.union_type(),
            "OTHER": lambda: my_types[self.value].type_name(),
        }[self.type]()

        return name

    # Name of the type of for the repeated part of this element.
    def repeated_type_name(self):
        if self.self_repeated_multi_var_condition():
            name = self.raw_type_name()
            if self.val_type_name() == name:
                name = "_" + name
        else:
            name = self.val_type_name()
        return name

    # Name of the type for this element.
    def type_name(self):
        if self.multi_var_condition():
            name = self.raw_type_name()
            if self.val_type_name() == name:
                name = "_" + name
            if self.repeated_type_name() == name:
                name = "_" + name
        else:
            name = self.repeated_type_name()
        return name

    # Name of variables and enum members for this element.
    def var_name(self):
        name = ("_%s" % self.id())
        return name

    # Create an access prefix based on an existing prefix, delimiter and a
    # suffix.
    def access_append_delimiter(self, prefix, delimiter, *suffix):
        assert prefix is not None, "No access prefix for %s" % self.var_name()
        return delimiter.join((prefix,) + suffix)

    # Create an access prefix from this element's prefix, delimiter and a
    # provided suffix.
    def access_append(self, *suffix):
        suffix = list(suffix)
        return self.access_append_delimiter(
            self.accessPrefix, self.access_delimiter, *suffix)

    # "Path" to this element's variable.
    # If full is false, the path to the repeated part is returned.
    def var_access(self):
        return self.access_append()

    # "Path" to access this element's actual value variable.
    def val_access(self):
        return self.access_append(self.var_name())

    # Whether to include a "present" variable for this element.
    def present_var_condition(self):
        return self.minQ == 0 and self.maxQ <= 1

    # Name of the "present" variable for this element.
    def present_var_name(self):
        return "%s_present" % (self.var_name())

    # Declaration of the "present" variable for this element.
    def present_var(self):
        return ["size_t %s;" % self.present_var_name()]

    # Full "path" of the "present" variable for this element.
    def present_var_access(self):
        return self.access_append(self.present_var_name())

    # Whether to include a "count" variable for this element.
    def count_var_condition(self):
        return self.maxQ > 1

    # Name of the "count" variable for this element.
    def count_var_name(self):
        return "%s_count" % (self.var_name())

    # Declaration of the "count" variable for this element.
    def count_var(self):
        return ["size_t %s;" % self.count_var_name()]

    # Full "path" of the "count" variable for this element.
    def count_var_access(self):
        return self.access_append(self.count_var_name())

    # Whether to include a "cbor" variable for this element.
    def is_cbor(self):
        return (self.type_name() is not None) and ((self.type != "OTHER") or (
                (self.value not in entry_type_names) and my_types[self.value].is_cbor()))

    # Whether to include a "cbor" variable for this element.
    def cbor_var_condition(self):
        return (self.cbor is not None) and self.cbor.is_cbor()

    # Whether to include a "choice" variable for this element.
    def choice_var_condition(self):
        return self.type == "UNION"

    # Name of the "choice" variable for this element.
    def choice_var_name(self):
        return self.var_name() + "_choice"

    # Declaration of the "choice" variable for this element.
    def choice_var(self):
        var = self.enclose("enum", [val.var_name() + "," for val in self.value])
        var[-1] += f" {self.choice_var_name()};"
        return var

    # Full "path" of the "choice" variable for this element.
    def choice_var_access(self):
        return self.access_append(self.choice_var_name())

    # Whether to include a "key" variable for this element.
    def key_var_condition(self):
        return self.key is not None

    # Whether this value adds any repeated elements by itself. I.e. excluding
    # multiple elements from children.
    def self_repeated_multi_var_condition(self):
        return (self.key_var_condition()
                or self.cbor_var_condition()
                or self.choice_var_condition())

    # Whether this element's actual value has multiple members.
    def multi_val_condition(self):
        return (self.type in ["LIST", "MAP", "GROUP", "UNION"]
                and (len(self.value) > 1
                     or (len(self.value) == 1
                        and self.value[0].multi_member())))

    # Whether any extra variables are to be included for this element for each
    # repetition.
    def repeated_multi_var_condition(self):
        return self.self_repeated_multi_var_condition() or self.multi_val_condition()

    # Whether any extra variables are to be included for this element outside
    # of repetitions.
    def multi_var_condition(self):
        return self.present_var_condition() or self.count_var_condition()

    # Whether this element must involve a call to multi_decode(), i.e. unless
    # it's repeated exactly once.
    def multi_decode_condition(self):
        return self.minQ != 1 or self.maxQ != 1

    # Name of the decoder function for this element.
    def decode_func_name(self):
        return "decode%s" % self.var_name()

    # Name of the decoder function for the repeated part of this element.
    def repeated_decode_func_name(self):
        return "decode_repeated%s" % self.var_name()

    # Declaration of the variables of all children.
    def child_declarations(self):
        decl = [line for child in self.value for line in child.full_declaration()]
        return decl

    # Declaration of the variables of all children.
    def child_single_declarations(self):
        decl = [
            line for child in self.value for line in child.add_var_name(child.single_var_type(), anonymous=True)]
        return decl

    # Enclose a list of declarations in a block (struct, union or enum).
    def enclose(self, ingress, declaration):
        return [f"{ingress} {{"] + [self.indentation +
                                    line for line in declaration] + ["}"]

    # Type declaration for unions.
    def union_type(self):
        declaration = self.enclose("union", self.child_single_declarations())
        return declaration

    # Recursively set the access prefix for this element and all its children.
    def set_access_prefix(self, prefix):
        self.accessPrefix = prefix
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            list(map(lambda child: child.set_access_prefix(
                self.var_access()), self.value))
        if self.key is not None:
            self.key.set_access_prefix(self.var_access())
        if self.cbor_var_condition():
            self.cbor.set_access_prefix(self.var_access())

    # Whether this type has multiple member variables.
    def multi_member(self):
        return self.multi_var_condition() or self.repeated_multi_var_condition()

    # Take a multi member type name and create a variable declaration. Make it an array if the element is repeated.
    def add_var_name(self, var_type, full=False, anonymous=False):
        if var_type:
            assert(var_type[-1][-1] == "}" or len(var_type) == 1), f"Expected single var: {var_type!r}"
            if not anonymous or var_type[-1][-1] != "}":
                var_type[-1] += " %s%s" % (self.var_name(), "[%d]" % self.maxQ if full and self.maxQ != 1 else "")
            var_type = add_semicolon(var_type)
        return var_type

    # The type for this element as a member variable.
    def var_type(self):
        if not self.multi_val_condition() and self.val_type_name() is not None:
            return [self.val_type_name()]
        elif self.type == "UNION":
            return self.union_type()
        return []

    # Declaration of the repeated part of this element.
    def repeated_declaration(self):
        var_type = self.var_type()
        multi_var = False

        decl = self.add_var_name(var_type, anonymous = self.type == "UNION")

        if self.type in ["LIST", "MAP", "GROUP"]:
            decl += self.child_declarations()
            multi_var = len(decl) > 1

        if self.key_var_condition():
            key_var = self.key.full_declaration()
            decl = key_var + decl
            multi_var = key_var != []

        if self.choice_var_condition():
            choice_var = self.choice_var()
            decl += choice_var
            multi_var = choice_var != []

        if self.cbor_var_condition():
            cbor_var = self.cbor.full_declaration()
            decl += cbor_var
            multi_var = cbor_var != []

        # if self.type not in ["LIST", "MAP", "GROUP"] or len(self.value) <= 1:
            # This assert isn't accurate for value lists with NIL or ANY
            # members.
            # assert multi_var == self.repeated_multi_var_condition(
            # ), f"""assert {multi_var} == {self.repeated_multi_var_condition()}
            # type: {self.type}
            # decl {decl}
            # self.key_var_condition() is {self.key_var_condition()}
            # self.key is {self.key}
            # self.key.full_declaration() is {self.key.full_declaration()}
            # self.cbor_var_condition() is {self.cbor_var_condition()}
            # self.choice_var_condition() is {self.choice_var_condition()}
            # self.value is {self.value}"""
        return decl

    # Declaration of the full type for this element.
    def full_declaration(self):
        multi_var = False

        if self.multi_var_condition():
            decl = self.add_var_name(
                [self.repeated_type_name()] if self.repeated_type_name() is not None else [], full=True)
        else:
            decl = self.repeated_declaration()

        if self.count_var_condition():
            count_var = self.count_var()
            decl += count_var
            multi_var = count_var != []

        if self.present_var_condition():
            present_var = self.present_var()
            decl += present_var
            multi_var = present_var != []

        assert multi_var == self.multi_var_condition()
        return decl

    # Return the type definition of this element. If there are multiple variables, wrap them in a struct so the function
    # always returns a single type with no name.
    # If full is False, only repeated part is used.
    def single_var_type(self, full=True):
        if full and self.multi_member():
            return self.enclose(
                "struct", self.full_declaration())
        elif not full and self.repeated_multi_var_condition():
            return self.enclose(
                "struct", self.repeated_declaration())
        else:
            return self.var_type()

    # Whether this element needs a check (memcmp) for a string value.
    def expected_string_condition(self):
        return self.type in ["BSTR", "TSTR"] and self.value is not None

    # Whether this element should have a typedef in the code.
    def type_def_condition(self):
        if self in my_types.values() and self.multi_member():
            return True
        return False

    # Whether this type needs a typedef for its repeated part.
    def repeated_type_def_condition(self):
        if self.repeated_multi_var_condition() and self.multi_var_condition():
            return True
        return False

    # Return the type definition of this element, and all its children + key +
    # cbor.
    def type_def(self):
        ret_val = []
        if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
            ret_val.extend(
                [elem for typedef in [child.type_def() for child in self.value] for elem in typedef])
        if self.cbor_var_condition():
            ret_val.extend(self.cbor.type_def())
        if self.key_var_condition():
            ret_val.extend(self.key.type_def())
        if self.type == "OTHER":
            ret_val.extend(my_types[self.value].type_def())
        if self.repeated_type_def_condition():
            ret_val.extend(
                [(self.single_var_type(full=False), self.repeated_type_name())])
        if self.type_def_condition():
            ret_val.extend([(self.single_var_type(), self.type_name())])
        return ret_val

    # Return the function name and arguments to call to decode this element. Only used when this element DOESN'T define
    # its own decoder function (when it's a primitive type, for which functions already exist, or when the function is
    # defined elsewhere ("OTHER"))
    def single_func_prim(self):
        vals = [self.val_access(), min_val_or_null(self.min_value), max_val_or_null(self.max_value)]
        sizes = [self.val_access(), min_val_or_null(self.min_size), max_val_or_null(self.max_size)]

        if self.type in ["LIST", "MAP"]:
            assert len(self.value) <= 1, f"List must be empty or have a single element, has {len(self.value)} children."

        # Will fail runtime if we don't use lambda for single_func()
        # pylint: disable=unnecessary-lambda
        retval = {
            "INT": lambda: ["intx32_decode", *vals],
            "UINT": lambda: ["uintx32_decode", *vals],
            "NINT": lambda: ["intx32_decode", *vals],
            "FLOAT": lambda: ["float_decode", *vals],
            "BSTR": lambda: ["strx_decode" if not self.cbor_var_condition() else "strx_start_decode", *sizes],
            "TSTR": lambda: ["strx_decode", *sizes],
            "BOOL": lambda: ["boolx_decode", self.val_access(), min_val_or_null(1 if self.value else 0),
                             max_val_or_null(0 if not self.value else 1)],
            "NIL": lambda: ["primx_decode", "NULL", min_val_or_null(22), max_val_or_null(22)],
            "MAP":  lambda: self.value[0].single_func() if len(self.value) >= 1 else ["list_start_decode", self.count_var_access(), "0", "0"],
            "LIST": lambda: self.value[0].single_func() if len(self.value) >= 1 else ["list_start_decode", self.count_var_access(), "0", "0"],
            "ANY": lambda: ["any_decode", "NULL", "NULL", "NULL"],
            "OTHER": lambda: list_replace_if_not_null(my_types[self.value].single_func(), 1, self.val_access()),
        }[self.type]()
        return retval

    # Return the function name and arguments to call to decode this element. Only used when this element has its own
    # decode function
    def single_func_impl(self, full=True):
        return (self.decode_func_name() if full else self.repeated_decode_func_name(), self.var_access()
                if full else self.val_access(), "NULL", "NULL")

    # Whether this element needs its own decoder function.
    def single_func_impl_condition(self):
        return (False or self.key or self.cbor_var_condition() or self.expected_string_condition() or
                self.type_def_condition())

    # Whether this element needs its own decoder function.
    def repeated_single_func_impl_condition(self):
        return self.repeated_type_def_condition()

    # Return the function name and arguments to call to decode this element.
    def single_func(self):
        if self.single_func_impl_condition():
            return self.single_func_impl()
        else:
            return self.single_func_prim()

    # Return the function name and arguments to call to decode the repeated
    # part of this element.
    def repeated_single_func(self):
        if self.repeated_single_func_impl_condition():
            return self.single_func_impl(full=False)
        else:
            return self.single_func_prim()

    # Return a number indicating how many other elements this element depends on. Used putting functions and typedefs
    # in the right order.
    def depends_on(self):
        ret_vals = [1]

        if not self.dependsOnCall:
            self.dependsOnCall = True
            if self.cbor_var_condition():
                ret_vals.append(self.cbor.depends_on())
            if self.key:
                ret_vals.append(self.key.depends_on())
            if self.type == "OTHER":
                ret_vals.append(1 + my_types[self.value].depends_on())
            if self.type in ["LIST", "MAP", "GROUP", "UNION"]:
                ret_vals.extend(child.depends_on() for child in self.value)
            self.dependsOnCall = False

        return max(ret_vals)

    # Make a string from the list returned by single_func_prim()
    def decode_single_func_prim(self):
        return decode_statement(*self.single_func_prim())

    # Return the full code needed to decode a "BSTR" or "TSTR" element.
    def decode_str(self):
        assert self.type in ["BSTR", "TSTR"], "Expected string type."
        return self.decode_single_func_prim() + (
            "&& !memcmp(\"{0}\", {1}.value, {1}.len)".format(
                self.value, self.val_access()) if self.expected_string_condition() else "")

    # Recursively sum the total minimum and maximum element count for this
    # element.

    def list_counts(self):
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
            # Lists are their own element
            "LIST": lambda: (self.minQ, self.maxQ),
            # Maps are their own element
            "MAP": lambda: (self.minQ, self.maxQ),
            "GROUP": lambda: (self.minQ * sum((child.minQ for child in self.value)),
                              self.maxQ * sum((child.maxQ for child in self.value))),
            "UNION": lambda: (self.minQ * min((child.minQ for child in self.value)),
                              self.maxQ * max((child.maxQ for child in self.value))),
            "OTHER": lambda: (q1 * q2 for q1, q2 in zip((self.minQ, self.maxQ), my_types[self.value].list_counts())),
        }[self.type]()

    # Return the full code needed to decode a "LIST" or "MAP" element with children.
    def decode_list(self):
        assert self.type in ["LIST", "MAP"], "Expected LIST or MAP type, was %s." % self.type
        min_counts, max_counts = zip(*(child.list_counts() for child in self.value)) if self.value else ((0,), (0,))
        return "(%s)" % (self.newl_ind + "&& ").join((decode_statement("list_start_decode",
                                "*(p_temp_elem_count++)",
                                str(sum(min_counts)),
                                str(sum(max_counts))),)
         + tuple(child.full_decode()
                 for child in self.value)
         + ("((p_state->elem_count = *(--p_temp_elem_count)) || 1)",))

    # Return the full code needed to decode a "GROUP" element's children.
    def decode_group(self):
        assert self.type in ["GROUP"], "Expected GROUP type."
        return "(%s)" % (self.newl_ind + "&& ").join(
            (child.full_decode() for child in self.value))

    # Return the full code needed to decode a "UNION" element's children.
    def decode_union(self):
        assert self.type in ["UNION"], "Expected UNION type."
        child_values = ["(%s && ((%s = %s) || 1))" %
                        (child.full_decode(),
                         self.choice_var_access(), child.var_name()) for child in self.value]

        # Reset state for all but the first child.
        for i in range(1, len(child_values)):
            child_values[i] = f"((p_state->p_payload = p_payload_bak) && ((p_state->elem_count = elem_count_bak) || 1) && {child_values[i]})"

        return "((p_payload_bak = p_state->p_payload) && ((elem_count_bak = p_state->elem_count) || 1) && (%s))" % \
               (self.newl_ind + "|| ").join(child_values)

    # Return the full code needed to decode this element, including children,
    # key and cbor, excluding repetitions.
    def repeated_decode(self):
        decoder = {
            "INT": self.decode_single_func_prim,
            "UINT": self.decode_single_func_prim,
            "NINT": self.decode_single_func_prim,
            "FLOAT": self.decode_single_func_prim,
            "BSTR": self.decode_str,
            "TSTR": self.decode_str,
            "BOOL": self.decode_single_func_prim,
            "NIL": self.decode_single_func_prim,
            "ANY": self.decode_single_func_prim,
            "LIST": self.decode_list,
            "MAP": self.decode_list,
            "GROUP": self.decode_group,
            "UNION": self.decode_union,
            "OTHER": self.decode_single_func_prim,
        }[self.type]()
        if self.key or self.cbor:
            arguments = ([self.key.full_decode()] if self.key is not None else [])\
                + ([decoder])\
                + ([f"(p_state->elem_count += {self.cbor.maxQ})", self.cbor.full_decode()]
                   if self.cbor_var_condition() else [])
            decoder = "(%s)" % ((self.newl_ind + "&& ").join(arguments),)
        return decoder

    # Code for the size of the repeated part of this element.
    def result_len(self):
        if self.repeated_type_name() is None:
            return "0"
        else:
            return "sizeof(%s)" % self.repeated_type_name()

    # Return the full code needed to decode this element, including children,
    # key, cbor, and repetitions.
    def full_decode(self):
        if self.multi_decode_condition():
            func, *arguments = self.repeated_single_func()
            return (
                "multi_decode(%s, %s, &%s, (void*)%s, %s, %s)" %
                (self.minQ,
                 self.maxQ,
                 self.count_var_access() if self.count_var_condition() else self.present_var_access(),
                 func,
                 decode_args(*arguments),
                 self.result_len()))
        else:
            return self.repeated_decode()

    # Return the body of the decoder function for this element.
    def decode(self):
        return self.repeated_decode()

    # Recursively return a list of the bodies of the decoder functions for
    # this element and its children + key + cbor.

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
        if self.type == "OTHER" and self.value not in entry_type_names:
            for decoder in my_types[self.value].decoders():
                yield decoder
        if self.repeated_single_func_impl_condition():
            yield (self.decode(), self.repeated_decode_func_name(), self.repeated_type_name())
        if ((self.type != "OTHER") or self.repeated_multi_var_condition()) and (
                self.single_func_impl_condition()):
            decode_body = self.decode()
            yield (decode_body, self.decode_func_name(), self.type_name())

# Consumes and parses a single CDDL type, returning a
# CodeGenerator instance.
def parse_single(instr, base_name=None):
    value = CodeGenerator(base_name=base_name)
    instr = value.get_value(instr).strip()
    return value, instr


# Parses entire instr and returns a list of CodeGenerator
# instances.
def parse(instr):
    instr = instr.strip()
    values = []
    while instr != '':
        (value, instr) = parse_single(instr)
        values.append(value)
    return values


# Returns a dict containing multiple typename=>string
def get_types(instr):
    type_regex = r"(\s*?\$?\$?([\w-]+)\s*(\/{0,2})=\s*(.*?)(?=(\Z|\s*\$?\$?[\w-]+\s*\/{0,2}=(?!\>))))"
    result = defaultdict(lambda: "")
    types = [(key, value, slashes) for (_1, key, slashes, value, _2) in findall(type_regex, instr, S | M)]
    for key, value, slashes in types:
        if slashes:
            result[key] += slashes
            result[key] += value
            result[key] = result[key].lstrip(slashes)  # strip from front
        else:
            result[key] = value
    return dict(result)


# Strip CDDL comments (';') from the string.
def strip_comments(instr):
    comment_regex = r"\;.*?\n"
    return sub(comment_regex, '', instr)


# Return a list of typedefs for all defined types, with duplicate typedefs
# removed.
def unique_types(types):
    type_names = {}
    out_types = []
    for mtype in types:
        for type_def in mtype.type_def():
            type_name = type_def[1]
            if type_name not in type_names.keys():
                type_names[type_name] = type_def[0]
                out_types.append(type_def)
            else:
                assert (''.join(type_names[type_name]) == ''.join(type_def[0])),\
                       ("Two elements share the type name %s, but their implementations are not identical. "
                        + "Please change one or both names. One of them is %s") % (type_name, pformat(mtype.type_def()))
    return out_types


# Return a list of decoder functions for all defined types, with duplicate
# functions removed.
def unique_funcs(types):
    func_names = {}
    out_types = []
    for mtype in types:
        decoders = list(mtype.decoders())
        for funcType in decoders:
            func_decode = funcType[0]
            func_name = funcType[1]
            if func_name not in func_names.keys():
                func_names[func_name] = funcType
                out_types.append(funcType)
            elif func_name in func_names.keys():
                assert func_names[func_name][0] == func_decode,\
                    ("Two elements share the function name %s, but their implementations are not identical. "
                        + "Please change one or both names.\n\n%s\n\n%s") % \
                    (func_name, func_names[func_name][0], func_decode)

    return out_types


# Return a list of decoder functions for all defined types, with unused
# functions removed.
def used_funcs(types, entry_types):
    entry_types = [
        (func_type.decode(),
         func_type.decode_func_name(),
         func_type.type_name()) for func_type in entry_types]
    out_types = [func_type for func_type in entry_types]
    full_code = "".join([func_type[0] for func_type in entry_types])
    for func_type in reversed(types):
        func_name = func_type[1]
        if func_type not in entry_types and search(r"%s\W" % func_name, full_code):
            full_code += func_type[0]
            out_types.append(func_type)
    return list(reversed(out_types))


# Return a list of typedefs for all defined types, with unused types removed.
def used_types(type_defs, entry_types):
    out_types = [typeDef for typeDef in entry_types]
    full_code = "".join(["".join(typeDef[0]) for typeDef in entry_types])
    for typeDef in reversed(type_defs):
        type_name = typeDef[1]
        if typeDef not in entry_types and search(r"%s\W" % type_name, full_code):
            full_code += "".join(typeDef[0])
            out_types.append(typeDef)
    return list(reversed(out_types))


def parse_args():
    parser = ArgumentParser(
        description='''Parse a CDDL file and produce C code that validates and decodes CBOR.
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

This script requires 'regex' for lookaround functionality not present in 're'.''',
        formatter_class=RawDescriptionHelpFormatter)

    parser.add_argument("-i", "--input", required=True, type=str,
                        help="Path to input CDDL file.")
    parser.add_argument("--output-c", "--oc", required=True, type=str,
                        help="Path to output C file.")
    parser.add_argument("--output-h", "--oh", required=True, type=str,
                        help="Path to output header file.")
    parser.add_argument(
        "-t",
        "--entry-types",
        required=True,
        type=str,
        nargs="+",
        help="Names of the types which should have their decode functions exposed.")
    parser.add_argument(
        "-v",
        "--verbose",
        required=False,
        action="store_true",
        default=False,
        help="Print more information while parsing CDDL and generating code.")
    parser.add_argument("--time-header", required=False, action="store_true", default=False,
                        help="Put the current time in a comment in the generated files.")

    return parser.parse_args()


# Render a single decoding function with signature and body.
def render_function(decoder):
    body = decoder[0]
    return f"""
static bool {decoder[1]}(
		cbor_decode_state_t *p_state, void * p_result, void * p_min_value,
		void * p_max_value)
{{
	cbor_decode_print("{ decoder[1] }\\n");
	{f"size_t temp_elem_counts[{body.count('p_temp_elem_count')}];" if "p_temp_elem_count" in body else ""}
	{"size_t *p_temp_elem_count = temp_elem_counts;" if "p_temp_elem_count" in body else ""}
	{"uint32_t current_list_num;" if "current_list_num" in body else ""}
	{"uint8_t const * p_payload_bak;" if "p_payload_bak" in body else ""}
	{"size_t elem_count_bak;" if "elem_count_bak" in body else ""}
	{"uint32_t min_value;" if "min_value" in body else ""}
	{"uint32_t max_value;" if "max_value" in body else ""}
	{decoder[2]}* p_type_result = ({decoder[2]}*)p_result;

	bool result = ({ body });

	if (!result)
	{{
		cbor_decode_trace();
	}}

	{"p_state->elem_count = temp_elem_counts[0];" if "p_temp_elem_count" in body else ""}
	return result;
}}""".replace("	\n", "")  # call replace() to remove empty lines.


# Render a single entry function (API function) with signature and body.
def render_entry_function(decoder):
    return f"""
bool cbor_{decoder.decode_func_name()}(const uint8_t * p_payload, size_t payload_len, {decoder.type_name()} * p_result)
{{
	cbor_decode_state_t state = {{
		.p_payload = p_payload,
		.p_payload_end = p_payload + payload_len,
		.elem_count = 1
	}};

	return {decoder.decode_func_name()}(&state, p_result, NULL, NULL);
}}"""


# Render the entire generated C file contents.
def render_c_file(functions, header_file_name, entry_types, print_time):
    return f"""/* Generated with cddl_gen.py (https://github.com/oyvindronningstad/cddl_gen){'''
 * at: ''' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') if print_time else ''}
 */

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"
#include "{header_file_name}"

{linesep.join([render_function(decoder) for decoder in functions])}

{linesep.join([render_entry_function(decoder) for decoder in entry_types])}
"""


# Render the entire generated header file contents.
def render_h_file(type_defs, header_guard, entry_types, print_time):
    return \
        f"""/* Generated with cddl_gen.py (https://github.com/oyvindronningstad/cddl_gen){'''
 * at: ''' + datetime.now().strftime('%Y-%m-%d %H:%M:%S') if print_time else ''}
 */

#ifndef {header_guard}
#define {header_guard}

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include <string.h>
#include "cbor_decode.h"


{(linesep+linesep).join([f"typedef {linesep.join(typedef[0])} {typedef[1]};" for typedef in type_defs])}

{(linesep+linesep).join([f"bool cbor_{decoder.decode_func_name()}(const uint8_t * p_payload, size_t payload_len, {decoder.type_name()} * p_result);" for decoder in entry_types])}

#endif // {header_guard}
"""


def main():
    global my_types, entry_type_names, verbose_flag

    # Parse command line arguments.
    args = parse_args()
    entry_type_names = args.entry_types
    verbose_flag = args.verbose

    # Read CDDL file. my_types will become a dict {name:str => definition:str}
    # for all types in the CDDL file.
    print ("Parsing " + args.input)
    with open(args.input, 'r') as f:
        my_types = get_types(strip_comments(f.read()))

    # Parse the definitions, replacing the each string with a
    # CodeGenerator instance.
    for my_type in my_types:
        (parsed, _) = parse_single(my_types[my_type].replace("\n", " "), base_name=my_type)
        my_types[my_type] = do_flatten(parsed)[0]
        my_types[my_type].set_id_prefix("")

        counter(True)

    # set access prefix (struct access paths) for all the definitions.
    for my_type in my_types:
        my_types[my_type].set_access_prefix("(*p_type_result)")

    # post_validate all the definitions.
    for my_type in my_types:
        my_types[my_type].post_validate()

    # Parsing is done, pretty print the result.
    verbose_print("Parsed CDDL types:")
    verbose_pprint(my_types)

    # Prepare the list of types that will have exposed decoder functions.
    entry_types = [my_types[entry] for entry in entry_type_names]

    # Sort type definitions so the typedefs will come in the correct order in the header file and the function in the
    # correct order in the c file.
    sorted_types = list(sorted(entry_types, key=lambda _type: _type.depends_on(), reverse=False))

    u_funcs = unique_funcs(sorted_types)
    u_funcs = used_funcs(u_funcs, entry_types)
    u_types = unique_types(sorted_types)

    # Create and populate the generated c and h file.
    makedirs("./" + path.dirname(args.output_c), exist_ok=True)
    with open(args.output_c, 'w') as f:
        print ("Writing to " + args.output_c)
        f.write(render_c_file(functions=u_funcs, header_file_name=path.basename(args.output_h),
                              entry_types=entry_types, print_time=args.time_header))
    makedirs("./" + path.dirname(args.output_h), exist_ok=True)
    with open(args.output_h, 'w') as f:
        print("Writing to " + args.output_h)
        f.write(render_h_file(type_defs=u_types,
                              header_guard=path.basename(args.output_h).replace(".", "_").replace("-", "_").upper()
                                           + "__",
                              entry_types=entry_types, print_time=args.time_header))


if __name__ == "__main__":
    main()
