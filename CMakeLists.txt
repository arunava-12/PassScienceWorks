
cmake_minimum_required(VERSION 3.20)

project(PassScienceWorkbench VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(thermal_core
    src/core/thermal/ThermalSolver.cpp
)

target_include_directories(thermal_core PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/src/core/thermal
)

add_executable(Launch
    src/main.cpp
)

target_link_libraries(Launch PRIVATE thermal_core)

if(MSVC)
    target_compile_options(thermal_core PRIVATE /W4)
    target_compile_options(Launch PRIVATE /W4)
else()
    target_compile_options(thermal_core PRIVATE
        -Wall -Wextra -Wpedantic
    )
    target_compile_options(Launch PRIVATE
        -Wall -Wextra -Wpedantic
    )
endif()

enable_testing()

add_executable(thermal_tests
    tests/thermal_tests.cpp
)

target_link_libraries(thermal_tests PRIVATE thermal_core)

add_test(NAME ThermalTests COMMAND thermal_tests)
