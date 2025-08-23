cmake_minimum_required(VERSION 3.16)
project(yasmin_plugins)

find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(rclcpp REQUIRED)
find_package(pluginlib REQUIRED)
find_package(class_loader REQUIRED)
find_package(yasmin REQUIRED)
find_package(pybind11 REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)

set(CMAKE_CXX_STANDARD 17)

# Build pybind11 bridge
pybind11_add_module(pybind_bridge src/pybind_bridge/pybind_bridge.cpp)
target_link_libraries(pybind_bridge PUBLIC
  pluginlib::pluginlib
  class_loader::class_loader
  yasmin::yasmin
)

# Set the output name and install location for the pybind11 module
set_target_properties(pybind_bridge PROPERTIES
  OUTPUT_NAME "pybind_bridge"
  PREFIX ""
)

# Install the pybind11 module to the same directory as the Python package
install(TARGETS pybind_bridge
  DESTINATION lib/python3/dist-packages/${PROJECT_NAME}
)

# Install Python loader package
ament_python_install_package(${PROJECT_NAME})

ament_package()