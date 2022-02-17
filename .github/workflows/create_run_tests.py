
from yaml import load, dump, Dumper, BaseLoader

class NoAliasDumper(Dumper):
	def ignore_aliases(self, data):
		return True

if __name__ == "__main__":
	with open("run-tests.yaml.in", 'r') as f:
		parsed = load(f.read(), Loader=BaseLoader)
	del parsed["anchors"]
	with open("run-tests.yaml", 'w') as f:
		f.write(dump(parsed, Dumper=NoAliasDumper))
