cmake_minimum_required(VERSION 3.21..3.31)
project(crash-bandicoot-pc LANGUAGES C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_TESTING "Build original game tests" OFF)
include(CTest)

set(PSYZ_DIR ${CMAKE_CURRENT_SOURCE_DIR}/third_party/psyz)
add_subdirectory(${PSYZ_DIR}/psyz ${CMAKE_CURRENT_BINARY_DIR}/psyz)
target_compile_definitions(psyz PRIVATE __psyz)

# Original modules may contain entry points no test calls yet; keep
# function-level linking instead of splitting source files for tests.
if(APPLE)
    add_link_options(-Wl,-dead_strip)
elseif(NOT MSVC)
    add_link_options(-Wl,--gc-sections)
endif()

# Compile a group of original modules in place against PsyZ.
function(crash_original_module target)
    add_library(${target} OBJECT ${ARGN})
    target_include_directories(${target} PRIVATE include src/main)
    target_link_libraries(${target} PRIVATE psyz)
    target_compile_definitions(${target} PRIVATE __psyz)
    if(MSVC)
        target_compile_options(${target} PRIVATE /Gy /Gw /W3)
    else()
        target_compile_options(${target} PRIVATE -ffunction-sections -fdata-sections
            -Wall -Wextra -Wno-unused-parameter)
    endif()
endfunction()

# Fixed-point math, angles, distances and trigonometry. Modules join as their
# platform dependencies become portable; no duplicate game logic.
crash_original_module(crash-original-math
    src/main/engine/gte_render.c
    src/main/math/angles.c
    src/main/math/approx_distance.c
    src/main/math/contains_point.c
    src/main/math/delta.c
    src/main/math/distance.c
    src/main/math/matrix_vector.c
    src/main/math/random_state.c
    src/main/math/scaled_sine.c
    src/main/math/step_halfword.c
    src/main/math/vector_distance.c)

if(BUILD_TESTING)
    add_executable(test-original-math tests/original_math.c
        $<TARGET_OBJECTS:crash-original-math>)
    target_include_directories(test-original-math PRIVATE include)
    target_compile_definitions(test-original-math PRIVATE __psyz)
    target_link_libraries(test-original-math PRIVATE psyz)
    add_test(NAME original-game-math COMMAND test-original-math)
endif()

# Original-module tests run without creating a window or GPU device.
if(BUILD_TESTING)
    get_property(original_tests DIRECTORY PROPERTY TESTS)
    foreach(original_test IN LISTS original_tests)
        if(original_test MATCHES "^original-game-")
            set_property(TEST ${original_test} APPEND PROPERTY ENVIRONMENT
                "PSYZ_HEADLESS=1" "SDL_AUDIODRIVER=dummy")
        endif()
    endforeach()
endif()
