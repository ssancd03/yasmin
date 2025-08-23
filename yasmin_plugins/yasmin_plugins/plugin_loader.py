import importlib
import xml.etree.ElementTree as ET
from yasmin import State, Blackboard, StateMachine
from typing import TYPE_CHECKING

try:
    from .pybind_bridge import CppStateFactory

    PYBIND_AVAILABLE = True
except ImportError:
    # Fallback if the pybind_bridge module is not built/installed
    PYBIND_AVAILABLE = False
    if TYPE_CHECKING:
        from .pybind_bridge import CppStateFactory


class _CppStateAdapter(State):
    def __init__(self, cpp_state):
        super().__init__(cpp_state.get_outcomes())
        self._cpp_state = cpp_state

    def execute(self, blackboard: Blackboard) -> str:
        return self._cpp_state(blackboard)


class YasminPluginLoader:

    def __init__(self):
        if PYBIND_AVAILABLE:
            self._cpp_factory = CppStateFactory()
        else:
            self._cpp_factory = None

    def load_state(self, state_elem: ET.Element) -> State:
        state_type = state_elem.attrib.get("type", "py")
        class_name = state_elem.attrib["class"]

        if state_type == "py":
            module_name = state_elem.attrib["module"]
            module = importlib.import_module(module_name)
            state_class = getattr(module, class_name)

            # Handle parameters if any
            params = state_elem.attrib.get("parameters", "")
            if params:
                param_list = [param.strip() for param in params.split(",")]
                return state_class(*param_list)
            else:
                return state_class()

        elif state_type == "cpp":
            if not PYBIND_AVAILABLE:
                raise RuntimeError(
                    "C++ states are not supported as pybind is unavailable."
                )
            return _CppStateAdapter(self._cpp_factory.create(class_name))

        else:
            raise ValueError(f"Unknown state type: {state_type}")

    def load_sm(self, xml_file: str) -> StateMachine:
        tree = ET.parse(xml_file)
        root = tree.getroot()

        if root.tag != "StateMachine":
            raise ValueError("Root element must be 'StateMachine'")

        return self.build_sm(root)

    def build_sm(self, root: ET.Element) -> StateMachine:

        sm = StateMachine(outcomes=root.attrib.get("outcomes", "").split(" "))

        for child in root:

            transitions = {}
            for cchild in child:
                if cchild.tag == "Transition":
                    transitions[cchild.attrib["from"]] = cchild.attrib["to"]

            if child.tag == "State":
                state = self.load_state(child)

            elif child.tag == "StateMachine":
                state = self.build_sm(child)

            sm.add_state(
                child.attrib["name"],
                state,
                transitions=transitions,
            )

        return sm
