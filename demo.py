# test_plugins.py
from yasmin_plugins.plugin_loader import YasminPluginLoader

loader = YasminPluginLoader()

sm = loader.load_sm("example.xml")
print(sm())
