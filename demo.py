# test_plugins.py
from yasmin_plugins.plugin_loader import YasminPluginLoader
from yasmin.blackboard import Blackboard

loader = YasminPluginLoader()

plugins = [
    {"type": "cpp", "class": "yasmin_demos/FooState"},
    {
        "type": "py",
        "module": "yasmin_demos.bar_state",
        "class": "BarState",
    },
]

states = loader.load_plugins(plugins)
bb = Blackboard()

for s in states:
    print(f"Executing {s} with outcomes {s.get_outcomes()}")
    outcome = s(bb)
    print(f"Outcome: {outcome}")
