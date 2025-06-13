import zcbor
import cbor2
import cProfile, pstats


try:
    import zcbor
except ImportError:
    print(
        """
The zcbor package must be installed to run these tests.
During development, install with `pip3 install -e .` to install in a way
that picks up changes in the files without having to reinstall.
"""
    )
    exit(1)

cddl_contents = """
Foo = int/bool
Bar = [0*1000(Foo)]
"""
message = list(range(500)) + list(bool(i % 2) for i in range(500))
raw_message = cbor2.dumps(message)
cmd_spec = zcbor.DataTranslator.from_cddl(cddl_contents, 3).my_types["Bar"]
# cmd_spec = zcbor.DataDecoder.from_cddl(cddl_contents, 3).my_types["Bar"]

profiler = cProfile.Profile()
profiler.enable()
json_obj = cmd_spec.str_to_json(raw_message)
profiler.disable()

profiler.print_stats()
