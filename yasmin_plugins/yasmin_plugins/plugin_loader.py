import importlib
from yasmin import State, blackboard
from typing import Iterable, List, Dict, TYPE_CHECKING

try:
    from .pybind_bridge import CppStateFactory, CppState

    PYBIND_AVAILABLE = True
except ImportError:
    # Fallback if the pybind_bridge module is not built/installed
    PYBIND_AVAILABLE = False
    if TYPE_CHECKING:
        from .pybind_bridge import CppStateFactory, CppState


class _CppStateAdapter(State):
    def __init__(self, cpp_state):
        super().__init__(cpp_state.get_outcomes())
        self._cpp_state = cpp_state

    def execute(self, blackboard: blackboard.Blackboard) -> str:
        # Convert Python blackboard to dict and use the new C++ method
        blackboard_dict = blackboard._data if hasattr(blackboard, "_data") else {}
        return self._cpp_state.call_with_dict(blackboard_dict)


class YasminPluginLoader:
    """
    Plugin spec format:
      - C++: {"type": "cpp", "class": "example_cpp_state_class_name"}
      - Py : {"type": "py",  "module": "example_py_module", "class": "ExamplePyState"}
    """

    def __init__(self):
        if PYBIND_AVAILABLE:
            self._cpp_factory = CppStateFactory()
        else:
            self._cpp_factory = None

    def load_plugins(self, specs: Iterable[Dict]) -> List[State]:
        states = []
        for spec in specs:
            if spec["type"] == "cpp":
                if not PYBIND_AVAILABLE:
                    raise RuntimeError(
                        "C++ plugins are not available. The pybind_bridge module is not installed."
                    )
                cpp_state = self._cpp_factory.create(spec["class"])
                states.append(_CppStateAdapter(cpp_state))
            elif spec["type"] == "py":
                mod = importlib.import_module(spec["module"])
                cls = getattr(mod, spec["class"])
                states.append(cls())
            else:
                raise ValueError(f"Unknown plugin type: {spec['type']}")
        return states
