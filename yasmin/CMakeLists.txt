cmake_minimum_required(VERSION 3.5)
project(yasmin)

# Default to C++17
if(NOT CMAKE_CXX_STANDARD)
  set(CMAKE_CXX_STANDARD 17)
endif()

# Find dependencies
find_package(ament_cmake REQUIRED)
find_package(ament_cmake_python REQUIRED)
find_package(Python3 REQUIRED COMPONENTS Development Interpreter)
find_package(pybind11 REQUIRED)
find_package(nlohmann_json REQUIRED)

# C++
include_directories(src)
include_directories(include)

set(SOURCES
  src/yasmin/blackboard/blackboard.cpp
  src/yasmin/logs.cpp
  src/yasmin/state.cpp
  src/yasmin/cb_state.cpp
  src/yasmin/state_machine.cpp
  src/yasmin/concurrence.cpp
)

add_library(${PROJECT_NAME} SHARED ${SOURCES})
target_include_directories(${PROJECT_NAME} PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_BINARY_DIR}/include>"
  "$<INSTALL_INTERFACE:include/${PROJECT_NAME}>")

install(TARGETS ${PROJECT_NAME}
  EXPORT ${PROJECT_NAME}
  ARCHIVE DESTINATION lib
  LIBRARY DESTINATION lib
  RUNTIME DESTINATION bin
)

install(DIRECTORY include/
  DESTINATION include/${PROJECT_NAME}
)

# Python bindings
pybind11_add_module(yasmin_blackboard
    src/yasmin/blackboard/blackboard_bindings.cpp
)
target_link_libraries(yasmin_blackboard PRIVATE
    pybind11::module
    ${PROJECT_NAME}
    nlohmann_json::nlohmann_json
)
install(TARGETS yasmin_blackboard
    LIBRARY DESTINATION lib/${PROJECT_NAME}
)
ament_environment_hooks(PYTHONPATH.sh.in)

ament_export_include_directories("include/${PROJECT_NAME}")
ament_export_libraries(${PROJECT_NAME})
ament_export_targets(${PROJECT_NAME})
ament_export_dependencies(nlohmann_json)

# Python package
ament_python_install_package(${PROJECT_NAME})

# Tests
if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  ament_lint_auto_find_test_dependencies()
  
  install(
    DIRECTORY tests/python/
    DESTINATION share/${PROJECT_NAME}/test
  )

  install(
    PROGRAMS tests/run_pytests.sh
    DESTINATION lib/${PROJECT_NAME}
  )
endif()

ament_package()