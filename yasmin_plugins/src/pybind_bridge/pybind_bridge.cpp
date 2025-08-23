#include "yasmin/blackboard/blackboard_wrapper.hpp"
#include <pluginlib/class_loader.hpp>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <yasmin/blackboard/blackboard.hpp>
#include <yasmin/state.hpp>

namespace py = pybind11;

class CppStateWrapper {
public:
  explicit CppStateWrapper(std::shared_ptr<yasmin::State> impl) : impl_(impl) {}
  std::set<std::string> get_outcomes() const { return impl_->get_outcomes(); }
  std::string operator()(std::shared_ptr<yasmin::blackboard::Blackboard> bb) {
    return (*impl_)(bb);
  }
  std::string operator()(BlackboardWrapper &bb_wrapper) {
    return (*impl_)(bb_wrapper.native());
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
      .def("__call__",
           py::overload_cast<BlackboardWrapper &>(&CppStateWrapper::operator()))
      .def("__call__",
           py::overload_cast<std::shared_ptr<yasmin::blackboard::Blackboard>>(
               &CppStateWrapper::operator()))
      .def("get_outcomes", &CppStateWrapper::get_outcomes)
      .def("__str__", &CppStateWrapper::to_string);

  py::class_<CppStateFactory>(m, "CppStateFactory")
      .def(py::init<>())
      .def("available_classes", &CppStateFactory::available_classes)
      .def("create", &CppStateFactory::create);
}
