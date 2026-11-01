
cmake_minimum_required(VERSION 3.20)

project(PassScienceWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT WIN32)
    message(FATAL_ERROR "PassScienceWorkbench GUI currently targets Windows.")
endif()

find_package(Threads REQUIRED)

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

# Performance and concurrency core
add_library(performance_core
    src/core/performance/Benchmark.cpp
)

target_include_directories(performance_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/performance
)

target_link_libraries(performance_core PUBLIC
    thermal_core
    Threads::Threads
)

# Manual backpropagation neural core
add_library(neural_core
    src/core/neural/Backpropagation.cpp
)

target_include_directories(neural_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/neural
)

# Embedded hardware simulation core
add_library(hardware_core
    src/core/hardware/MockDevices.cpp
)

target_include_directories(hardware_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/hardware
)

# Unified native Windows GUI application
add_executable(Launch WIN32
    src/ui/MainWindow.cpp
)

target_link_libraries(Launch PRIVATE
    thermal_core
    geometry_core
    performance_core
    hardware_core
    user32
    gdi32
    comdlg32
    Threads::Threads
)

# Separate benchmark application
add_executable(BenchmarkRunner
    src/benchmark_main.cpp
)

target_link_libraries(BenchmarkRunner PRIVATE
    performance_core
)

# Backpropagation application
add_executable(BackpropRunner
    src/neural_main.cpp
)

target_link_libraries(BackpropRunner PRIVATE
    neural_core
)

# Embedded hardware demonstration
add_executable(HardwareRunner
    src/hardware_main.cpp
)

target_link_libraries(HardwareRunner PRIVATE
    hardware_core
)

# Compiler warnings
if(MSVC)
    set(WARNING_FLAGS /W4)
else()
    set(WARNING_FLAGS -Wall -Wextra -Wpedantic)
endif()

foreach(target
    thermal_core
    geometry_core
    performance_core
    neural_core
    hardware_core
    Launch
    BenchmarkRunner
    BackpropRunner
    HardwareRunner
)
    target_compile_options(${target} PRIVATE ${WARNING_FLAGS})
endforeach()

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

add_executable(performance_tests
    tests/performance_tests.cpp
)

target_link_libraries(performance_tests PRIVATE performance_core)
add_test(NAME PerformanceTests COMMAND performance_tests)

add_executable(neural_tests
    tests/neural_tests.cpp
)

target_link_libraries(neural_tests PRIVATE neural_core)
add_test(NAME NeuralTests COMMAND neural_tests)

add_executable(hardware_tests
    tests/hardware_tests.cpp
)

target_link_libraries(hardware_tests PRIVATE hardware_core)
add_test(NAME HardwareTests COMMAND hardware_tests)
