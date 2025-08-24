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

#ifndef YASMIN__BLACKBOARD__BLACKBOARD_WRAPPER_HPP
#define YASMIN__BLACKBOARD__BLACKBOARD_WRAPPER_HPP

#include "yasmin/blackboard/blackboard.hpp"
#include <nlohmann/json.hpp>
#include <pybind11/pybind11.h>
#include <type_traits>

namespace py = pybind11;
using json = nlohmann::json;

namespace yasmin {
namespace blackboard {

/**
 * @class BlackboardWrapper
 * @brief A Python wrapper for the C++ Blackboard class, providing a
 * dictionary-like interface.
 */
class BlackboardWrapper {
public:
  /**
   * @brief Constructor for BlackboardWrapper.
   * @param initial_data Optional initial data to populate the blackboard.
   */
  BlackboardWrapper(py::dict initial_data = py::dict())
      : bb(std::make_shared<Blackboard>()) {
    for (auto item : initial_data) {
      std::string key = py::str(item.first).cast<std::string>();
      py::object value = item.second.cast<py::object>();
      this->set_item(key, value);
    }
  }

  /**
   * @brief Set a value in the blackboard.
   * @param key The key to associate with the value.
   * @param value The value to store.
   */
  void set_item(const std::string &key, py::object value) {
    // Check for primitive types first to avoid JSON serialization
    if (py::isinstance<py::int_>(value)) {
      this->bb->set<int64_t>(key, value.cast<int64_t>());
      this->type_registry[key] = "int";
    } else if (py::isinstance<py::float_>(value)) {
      bb->set<double>(key, value.cast<double>());
      this->type_registry[key] = "float";
    } else if (py::isinstance<py::bool_>(value)) {
      this->bb->set<bool>(key, value.cast<bool>());
      this->type_registry[key] = "bool";
    } else if (py::isinstance<py::str>(value)) {
      this->bb->set<std::string>(key, value.cast<std::string>());
      this->type_registry[key] = "string";
    } else if (py::isinstance<py::list>(value)) {
      // Handle lists directly by serializing to JSON
      json j = this->convert_py_to_json(value);
      this->bb->set<std::string>(key, j.dump());
      this->type_registry[key] = "list";
    } else if (py::isinstance<py::tuple>(value)) {
      // Handle tuples directly by serializing to JSON
      json j = this->convert_py_to_json(value);
      this->bb->set<std::string>(key, j.dump());
      this->type_registry[key] = "tuple";
    } else if (py::isinstance<py::dict>(value)) {
      // Handle dicts directly by serializing to JSON
      json j = this->convert_py_to_json(value);
      this->bb->set<std::string>(key, j.dump());
      this->type_registry[key] = "dict";
    } else if (value.is_none()) {
      // Store None as empty string or special marker
      this->bb->set<std::string>(key, "__NONE__");
      this->type_registry[key] = "None";
    } else {
      // For other complex types, use JSON serialization
      json j = this->convert_py_to_json(value);
      this->bb->set<std::string>(key, j.dump());
      this->type_registry[key] = "json";
    }
  }

  /**
   * @brief Get a value from the blackboard.
   * @param key The key associated with the value.
   * @return The value associated with the specified key.
   * @throws py::key_error if the key does not exist.
   * @throws py::value_error if the value cannot be converted to a Python type.
   */
  py::object get_item(const std::string &key) {
    if (!bb->contains(key)) {
      throw py::key_error("Key '" + key + "' not found in blackboard");
    }

    // Use the type registry for efficient and accurate retrieval
    auto it = this->type_registry.find(key);
    if (it != this->type_registry.end()) {
      const std::string &type = it->second;

      if (type == "None") {
        return py::none();
      } else if (type == "bool") {
        return py::cast(this->bb->get<bool>(key));
      } else if (type == "int") {
        return py::cast(this->bb->get<int64_t>(key));
      } else if (type == "float") {
        return py::cast(this->bb->get<double>(key));
      } else if (type == "list") {
        std::string str_val = this->bb->get<std::string>(key);
        json j = json::parse(str_val);
        return this->convert_json_to_py(j);
      } else if (type == "tuple") {
        std::string str_val = this->bb->get<std::string>(key);
        json j = json::parse(str_val);
        py::object py_list = this->convert_json_to_py(j);
        return py::tuple(py_list); // Convert list back to tuple
      } else if (type == "dict") {
        std::string str_val = this->bb->get<std::string>(key);
        json j = json::parse(str_val);
        return this->convert_json_to_py(j);
      } else if (type == "json") {
        std::string str_val = this->bb->get<std::string>(key);
        json j = json::parse(str_val);
        return this->convert_json_to_py(j);
      } else if (type == "string") {
        return py::cast(this->bb->get<std::string>(key));
      }
    }

    // Fallback to dynamic type detection (less efficient but handles edge
    // cases)
    std::string type = this->get_type(key);

    if (type == "None") {
      return py::none();
    } else if (type == "bool") {
      return py::cast(this->bb->get<bool>(key));
    } else if (type == "int") {
      return py::cast(this->bb->get<int64_t>(key));
    } else if (type == "float") {
      return py::cast(this->bb->get<double>(key));
    } else if (type == "list") {
      std::string str_val = this->bb->get<std::string>(key);
      json j = json::parse(str_val);
      return this->convert_json_to_py(j);
    } else if (type == "tuple") {
      std::string str_val = this->bb->get<std::string>(key);
      json j = json::parse(str_val);
      py::object py_list = this->convert_json_to_py(j);
      return py::tuple(py_list); // Convert list back to tuple
    } else if (type == "dict") {
      std::string str_val = this->bb->get<std::string>(key);
      json j = json::parse(str_val);
      return this->convert_json_to_py(j);
    } else if (type == "json") {
      std::string str_val = this->bb->get<std::string>(key);
      json j = json::parse(str_val);
      return this->convert_json_to_py(j);
    } else if (type == "string") {
      return py::cast(this->bb->get<std::string>(key));
    } else {
      // Unknown type, try string as fallback
      try {
        return py::cast(this->bb->get<std::string>(key));
      } catch (...) {
        throw py::value_error("Failed to retrieve value for key '" + key +
                              "' with type '" + type + "'");
      }
    }
  }

  /**
   * @brief Delete a value from the blackboard.
   * @param key The key associated with the value to delete.
   * @throws py::key_error if the key does not exist.
   */
  void del_item(const std::string &key) {
    if (!this->bb->contains(key)) {
      throw py::key_error("Key '" + key + "' not found in blackboard");
    }
    this->bb->remove(key);
    this->type_registry.erase(key);
  }

  /**
   * @brief Check if a key exists in the blackboard.
   * @param key The key to check.
   * @return True if the key exists, false otherwise.
   */
  bool contains(const std::string &key) { return this->bb->contains(key); }

  /**
   * @brief Get the number of key-value pairs in the blackboard.
   * @return The size of the blackboard.
   */
  int size() { return this->bb->size(); }

  /**
   * @brief Get the remapping of the blackboard.
   * @return The remapping as a Python dictionary.
   */
  py::dict get_remapping() {
    py::dict result;
    auto remapping = this->bb->get_remapping();
    for (const auto &pair : remapping) {
      result[pair.first.c_str()] = pair.second;
    }
    return result;
  }

  /**
   * @brief Set the remapping of the blackboard.
   * @param remapping The remapping to set as a Python dictionary.
   */
  void set_remapping(const py::dict &remapping) {
    std::map<std::string, std::string> cpp_map;
    for (auto item : remapping) {
      std::string key = py::str(item.first).cast<std::string>();
      std::string value = py::str(item.second).cast<std::string>();
      cpp_map[key] = value;
    }
    this->bb->set_remapping(cpp_map);
  }

  /**
   * @brief Convert the blackboard to a string representation.
   * @return The string representation of the blackboard.
   */
  std::string to_string() { return this->bb->to_string(); }

  /**
   * @brief Get the underlying native Blackboard instance.
   * @return A shared pointer to the native Blackboard instance.
   */
  std::shared_ptr<Blackboard> native() { return this->bb; }

  /**
   * @brief Get the type of the value associated with a key.
   * @param key The key to check.
   * @return The type as a string (e.g., "int", "float", "string", "list",
   * "dict", "None", etc.).
   * @throws py::key_error if the key does not exist.
   */
  std::string get_type(const std::string &key) {
    if (!this->bb->contains(key)) {
      throw py::key_error("Key '" + key + "' not found");
    }

    // Use the type registry for accurate type information
    auto it = this->type_registry.find(key);
    if (it != this->type_registry.end()) {
      return it->second;
    }

    // Fallback for values that might have been set through other means
    // (e.g., direct C++ access to the underlying blackboard)
    // This fallback is more robust and deterministic
    try {
      // First check for None marker
      std::string str_val = this->bb->get<std::string>(key);
      if (str_val == "__NONE__") {
        this->type_registry[key] = "None";
        return "None";
      }

      // Check if it's valid JSON and determine the specific type
      try {
        json j = json::parse(str_val);
        if (j.is_array()) {
          // We can't distinguish between list and tuple from JSON alone,
          // so we default to "json" for fallback cases
          this->type_registry[key] = "json";
          return "json";
        } else if (j.is_object()) {
          this->type_registry[key] = "json";
          return "json";
        } else {
          // Other JSON types (shouldn't happen for our use case)
          this->type_registry[key] = "json";
          return "json";
        }
      } catch (...) {
        // It's a regular string
        this->type_registry[key] = "string";
        return "string";
      }
    } catch (...) {
      // Not a string type, check other types
    }

    // Check for boolean (must be before int due to implicit conversion)
    try {
      this->bb->get<bool>(key);
      // Verify it's not actually an int by checking if int casting also works
      try {
        int64_t int_val = this->bb->get<int64_t>(key);
        // If both bool and int work, determine which was originally stored
        // This is a limitation of the try-catch approach, but we can make
        // educated guesses
        bool bool_val = this->bb->get<bool>(key);
        if ((int_val == 0 && !bool_val) || (int_val == 1 && bool_val)) {
          // Could be either, but if value is 0 or 1, lean towards bool if small
          // values
          if (int_val >= 0 && int_val <= 1) {
            this->type_registry[key] = "bool";
            return "bool";
          } else {
            this->type_registry[key] = "int";
            return "int";
          }
        } else {
          this->type_registry[key] = "int";
          return "int";
        }
      } catch (...) {
        this->type_registry[key] = "bool";
        return "bool";
      }
    } catch (...) {
      // Not a bool
    }

    // Check for integer
    try {
      this->bb->get<int64_t>(key);
      this->type_registry[key] = "int";
      return "int";
    } catch (...) {
      // Not an int
    }

    // Check for float
    try {
      this->bb->get<double>(key);
      this->type_registry[key] = "float";
      return "float";
    } catch (...) {
      // Not a float
    }

    // If we reach here, type is unknown
    this->type_registry[key] = "unknown";
    return "unknown";
  }

private:
  /// The underlying native Blackboard instance
  std::shared_ptr<Blackboard> bb;
  /// Track the actual types of stored values to avoid ambiguity
  std::map<std::string, std::string> type_registry;

  /**
   * @brief Convert a Python object to JSON.
   * @param obj The Python object to convert.
   * @return The corresponding JSON representation.
   */
  json convert_py_to_json(py::object obj) {
    if (py::isinstance<py::list>(obj) || py::isinstance<py::tuple>(obj)) {
      json j_array = json::array();
      for (auto item : obj) {
        j_array.push_back(convert_py_to_json(item.cast<py::object>()));
      }
      return j_array;
    } else if (py::isinstance<py::dict>(obj)) {
      json j_object = json::object();
      py::dict py_dict = obj.cast<py::dict>();

      for (auto item : py_dict) {
        std::string key = py::str(item.first).cast<std::string>();
        py::object value = item.second.cast<py::object>();
        j_object[key] = convert_py_to_json(value);
      }
      return j_object;
    } else if (py::isinstance<py::int_>(obj)) {
      return obj.cast<int64_t>();
    } else if (py::isinstance<py::float_>(obj)) {
      return obj.cast<double>();
    } else if (py::isinstance<py::bool_>(obj)) {
      return obj.cast<bool>();
    } else if (py::isinstance<py::str>(obj)) {
      return obj.cast<std::string>();
    } else if (obj.is_none()) {
      return nullptr;
    } else {
      // For other types, convert to string as fallback
      return py::str(obj).cast<std::string>();
    }
  }

  /**
   * @brief Convert JSON to a Python object.
   * @param j The JSON object to convert.
   * @return The corresponding Python object.
   */
  py::object convert_json_to_py(const json &j) {
    switch (j.type()) {
    case json::value_t::number_integer:
    case json::value_t::number_unsigned:
      return py::cast(j.get<int64_t>());
    case json::value_t::number_float:
      return py::cast(j.get<double>());
    case json::value_t::string:
      return py::cast(j.get<std::string>());
    case json::value_t::boolean:
      return py::cast(j.get<bool>());
    case json::value_t::array: {
      py::list py_list;
      for (const auto &item : j) {
        py_list.append(convert_json_to_py(item));
      }
      return py_list;
    }
    case json::value_t::object: {
      py::dict py_dict;
      for (auto it = j.begin(); it != j.end(); ++it) {
        py_dict[it.key().c_str()] = convert_json_to_py(it.value());
      }
      return py_dict;
    }
    case json::value_t::null:
      return py::none();
    default:
      return py::none();
    }
  }
};

} // namespace blackboard
} // namespace yasmin

#endif // YASMIN__BLACKBOARD__BLACKBOARD_WRAPPER_HPP
