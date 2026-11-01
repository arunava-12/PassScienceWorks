
cmake_minimum_required(VERSION 3.20)

project(PassScienceWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Thermal FEM core
add_library(thermal_core
    src/core/thermal/ThermalSolver.cpp
)

target_include_directories(thermal_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/thermal
)

# Geometry and temperature data core
add_library(geometry_core
    src/core/geometry/STLParser.cpp
    src/core/geometry/TemperatureCSV.cpp
)

target_include_directories(geometry_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/geometry
)

# Main application
add_executable(Launch
    src/main.cpp
)

target_link_libraries(Launch PRIVATE
    thermal_core
    geometry_core
)

# Compiler warnings
if(MSVC)
    target_compile_options(thermal_core PRIVATE /W4)
    target_compile_options(geometry_core PRIVATE /W4)
    target_compile_options(Launch PRIVATE /W4)
else()
    target_compile_options(thermal_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(geometry_core PRIVATE -Wall -Wextra -Wpedantic)
    target_compile_options(Launch PRIVATE -Wall -Wextra -Wpedantic)
endif()

# Automated tests
enable_testing()

add_executable(thermal_tests
    tests/thermal_tests.cpp
)

target_link_libraries(thermal_tests PRIVATE thermal_core)

add_test(NAME ThermalTests COMMAND thermal_tests)

add_executable(geometry_tests
    tests/geometry_tests.cpp
)

target_link_libraries(geometry_tests PRIVATE geometry_core)

add_test(NAME GeometryTests COMMAND geometry_tests)
