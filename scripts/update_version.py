from sys import argv
from pathlib import Path
from re import sub, match, S
from datetime import datetime

p_root = Path(__file__).absolute().parents[1]
# p_README = Path(p_root, 'README.md')
p_VERSION = Path(p_root, 'zcbor', 'VERSION')
p_RELEASE_NOTES = Path(p_root, 'RELEASE_NOTES.md')
p_common_h = Path(p_root, 'include', 'zcbor_common.h')

if __name__ == "__main__":
    version = argv[1]
    (major, minor, bugfix) = version.split('.')

    p_VERSION.write_text(version, encoding="utf-8")
    relnotes_contents = p_RELEASE_NOTES.read_text(encoding="utf-8")
    relnotes_lines = relnotes_contents.splitlines()
    if version not in relnotes_lines[0]:
        relnotes_new_header = f"# zcbor v. {version} ({datetime.today().strftime('%Y-%m-%d')})\n"
        if ".99" not in relnotes_lines[0]:
            prev_entry = match(r"\A# zcbor.*?(?=# zcbor v.)", relnotes_contents, S).group(0)
            relnotes_contents = prev_entry + relnotes_contents
        relnotes_contents = sub(r".*?\n", relnotes_new_header, relnotes_contents, count=1)
        p_RELEASE_NOTES.write_text(relnotes_contents, encoding="utf-8")
    p_common_h_contents = p_common_h.read_text(encoding="utf-8")
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_MAJOR )\d+", f"\\g<1>{major}", p_common_h_contents)
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_MINOR )\d+", f"\\g<1>{minor}", common_h_new_contents)
    common_h_new_contents = sub(r"(#define ZCBOR_VERSION_BUGFIX )\d+", f"\\g<1>{bugfix}", common_h_new_contents)
    p_common_h.write_text(common_h_new_contents, encoding="utf-8")
