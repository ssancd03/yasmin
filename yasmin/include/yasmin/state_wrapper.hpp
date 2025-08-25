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

#ifndef YASMIN__STATE_WRAPPER_HPP
#define YASMIN__STATE_WRAPPER_HPP

#include "yasmin/blackboard/blackboard_wrapper.hpp"
#include <pybind11/pybind11.h>
#include <yasmin/blackboard/blackboard.hpp>
#include <yasmin/state.hpp>

namespace py = pybind11;

namespace yasmin {

std::set<std::string> py_set_to_std_set(const py::set &pySet) {
  std::set<std::string> stdSet;
  for (const auto &item : pySet) {
    stdSet.insert(item.cast<std::string>());
  }
  return stdSet;
}

std::set<std::string> py_list_to_std_set(const py::list &pyList) {
  std::set<std::string> stdSet;
  for (const auto &item : pyList) {
    stdSet.insert(item.cast<std::string>());
  }
  return stdSet;
}

py::set std_set_to_py_set(const std::set<std::string> &stdSet) {
  py::set pySet;
  for (const auto &item : stdSet) {
    pySet.add(item);
  }
  return pySet;
}

/**
 * @class StateWrapper
 * @brief A wrapper class to hold a C++ State instance and expose it to Python.
 *
 * This class manages a shared pointer to a yasmin::State instance and provides
 * methods to interact with it from Python.
 */
class StateWrapper {
public:
  /**
   * @brief Constructor for StateWrapper.
   * @param outcomes Optional outcomes.
   */
  StateWrapper(py::set outcomes = py::set())
      : impl_(std::make_shared<State>(py_set_to_std_set(outcomes))) {}
  StateWrapper(py::list outcomes = py::list())
      : impl_(std::make_shared<State>(py_list_to_std_set(outcomes))) {}

  /**
   * @brief Constructs a StateWrapper with a shared pointer to a
   * yasmin::State.
   * @param impl A shared pointer to a yasmin::State instance.
   */
  explicit StateWrapper(std::shared_ptr<yasmin::State> impl) : impl_(impl) {}

  /**
   * @brief Retrieves the possible outcomes of the state.
   * @return A set of strings representing the possible outcomes.
   */
  py::set get_outcomes() const {
    return std_set_to_py_set(impl_->get_outcomes());
  }

  /**
   * @brief Calls the underlying C++ State instance with a Blackboard.
   * @return The outcome of the state execution as a string.
   */
  std::string operator()() {
    std::shared_ptr<blackboard::Blackboard> blackboard =
        std::make_shared<blackboard::Blackboard>();
    return (*impl_)(blackboard);
  }

  /**
   * @brief Calls the underlying C++ State instance with a Blackboard.
   * @param bb A shared pointer to a yasmin::Blackboard instance.
   * @return The outcome of the state execution as a string.
   */
  std::string operator()(std::shared_ptr<yasmin::blackboard::Blackboard> bb) {
    return (*impl_)(bb);
  }

  /**
   * @brief Calls the underlying C++ State instance with a BlackboardWrapper.
   * @param bb_wrapper A reference to a BlackboardWrapper instance.
   * @return The outcome of the state execution as a string.
   */
  std::string operator()(yasmin::blackboard::BlackboardWrapper &bb_wrapper) {
    return (*impl_)(bb_wrapper.native());
  }

  /**
   * @brief Converts the state to a string representation.
   * @return A string representation of the state.
   */
  std::string to_string() const { return impl_->to_string(); }

  /**
   * @brief Get the underlying native State instance.
   * @return A shared pointer to the native State instance.
   */
  std::shared_ptr<State> native() { return this->impl_; }

private:
  /// The underlying C++ State instance.
  std::shared_ptr<yasmin::State> impl_;
};

} // namespace yasmin

#endif // YASMIN__STATE_WRAPPER_HPP
