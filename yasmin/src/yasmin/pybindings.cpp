// Copyright (C) 2025 Miguel Ángel González Santamarta
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <https://www.gnu.org/licenses/>.

#include "yasmin/blackboard/blackboard_wrapper.hpp"
#include "yasmin/state_wrapper.hpp"
#include <pybind11/pybind11.h>

namespace py = pybind11;
using namespace yasmin;
using namespace yasmin::blackboard;

PYBIND11_MODULE(yasmin_py, m) {
  py::class_<BlackboardWrapper>(m, "Blackboard")
      // __init__ with optional initial data
      .def(py::init<py::dict>(), py::arg("initial_data") = py::dict())

      // __setitem__ for bb["key"] = value
      .def("__setitem__", &BlackboardWrapper::set_item)

      // __getitem__ for value = bb["key"]
      .def("__getitem__", &BlackboardWrapper::get_item)

      // __delitem__ for del bb["key"]
      .def("__delitem__", &BlackboardWrapper::del_item)

      // __contains__ for "key" in bb
      .def("__contains__", &BlackboardWrapper::contains)

      // __len__ for len(bb)
      .def("__len__", &BlackboardWrapper::size)

      // __str__ for str(bb)
      .def("__str__", &BlackboardWrapper::to_string)

      // __repr__ for repr(bb)
      .def("__repr__", &BlackboardWrapper::to_string)

      // Additional methods for completeness
      .def("get", &BlackboardWrapper::get_item, py::arg("key"),
           "Get a value from the blackboard")

      .def("set", &BlackboardWrapper::set_item, py::arg("key"),
           py::arg("value"), "Set a value in the blackboard")

      .def("remove", &BlackboardWrapper::del_item, py::arg("key"),
           "Remove a value from the blackboard")

      .def("contains", &BlackboardWrapper::contains, py::arg("key"),
           "Check if a key exists in the blackboard")

      .def("size", &BlackboardWrapper::size,
           "Get the number of key-value pairs")

      .def("to_string", &BlackboardWrapper::to_string,
           "Get string representation of the blackboard")

      .def_property("remappings", &BlackboardWrapper::get_remapping,
                    &BlackboardWrapper::set_remapping)

      .def_property_readonly(
          "native", &BlackboardWrapper::native,
          "Get the native C++ blackboard object (for advanced use)");

  py::class_<StateWrapper>(m, "State")
      // __init__ with optional initial data
      .def(py::init<py::set>(), py::arg("outcomes") = py::set())
      .def(py::init<py::list>(), py::arg("outcomes") = py::list())

      // Call operators
      .def("__call__", py::overload_cast<>(&StateWrapper::operator()))
      .def("__call__",
           py::overload_cast<yasmin::blackboard::BlackboardWrapper &>(
               &StateWrapper::operator()))
      .def("__call__",
           py::overload_cast<std::shared_ptr<yasmin::blackboard::Blackboard>>(
               &StateWrapper::operator()))

      // get outcomes
      .def("get_outcomes", &StateWrapper::get_outcomes)

      // string representation
      .def("__str__", &StateWrapper::to_string)

      .def_property_readonly(
          "native", &StateWrapper::native,
          "Get the native C++ state object (for advanced use)");
}