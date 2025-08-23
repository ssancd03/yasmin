#include <pluginlib/class_loader.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <yasmin/blackboard/blackboard.hpp>
#include <yasmin/state.hpp>

namespace py = pybind11;

// Helper function to convert Python dict to C++ Blackboard
std::shared_ptr<yasmin::blackboard::Blackboard>
dict_to_blackboard(const py::dict &py_dict) {
  auto bb = std::make_shared<yasmin::blackboard::Blackboard>();

  for (auto item : py_dict) {
    std::string key = py::str(item.first);
    py::handle value = item.second;

    // For now, we'll support basic types. This can be extended as needed.
    if (py::isinstance<py::str>(value)) {
      bb->set<std::string>(key, py::cast<std::string>(value));
    } else if (py::isinstance<py::int_>(value)) {
      bb->set<int>(key, py::cast<int>(value));
    } else if (py::isinstance<py::float_>(value)) {
      bb->set<double>(key, py::cast<double>(value));
    } else if (py::isinstance<py::bool_>(value)) {
      bb->set<bool>(key, py::cast<bool>(value));
    }
    // Add more type conversions as needed
  }

  return bb;
}

class CppStateWrapper {
public:
  explicit CppStateWrapper(std::shared_ptr<yasmin::State> impl) : impl_(impl) {}
  std::set<std::string> get_outcomes() const { return impl_->get_outcomes(); }
  std::string operator()(std::shared_ptr<yasmin::blackboard::Blackboard> bb) {
    return (*impl_)(bb);
  }
  std::string call_with_dict(const py::dict &py_dict) {
    auto bb = dict_to_blackboard(py_dict);
    return (*impl_)(bb);
  }
  std::string to_string() const { return impl_->to_string(); }

private:
  std::shared_ptr<yasmin::State> impl_;
};

class CppStateFactory {
public:
  CppStateFactory() : loader_("yasmin", "yasmin::State") {}
  std::vector<std::string> available_classes() {
    return loader_.getDeclaredClasses();
  }
  CppStateWrapper create(const std::string &class_name) {
    auto instance = loader_.createSharedInstance(class_name);
    return CppStateWrapper(instance);
  }

private:
  pluginlib::ClassLoader<yasmin::State> loader_;
};

PYBIND11_MODULE(pybind_bridge, m) {
  py::class_<CppStateWrapper>(m, "CppState")
      .def("__call__", &CppStateWrapper::operator())
      .def("call_with_dict", &CppStateWrapper::call_with_dict)
      .def("get_outcomes", &CppStateWrapper::get_outcomes)
      .def("__str__", &CppStateWrapper::to_string);

  py::class_<CppStateFactory>(m, "CppStateFactory")
      .def(py::init<>())
      .def("available_classes", &CppStateFactory::available_classes)
      .def("create", &CppStateFactory::create);
}
