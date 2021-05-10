for f in crashes/*; do
	cat $f |  ../build-afl/fuzz_manifest12
done