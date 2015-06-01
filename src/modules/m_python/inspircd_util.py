import sys

def unload(mod):
	deps = [module for module in sys.modules if module.startswith(mod + '.')]
	for dep in deps:
		del sys.modules[dep]
	if mod in sys.modules:
		del sys.modules[mod]

