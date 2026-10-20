cmake_minimum_required(VERSION 3.21..3.31)
project(crash-bandicoot-pc LANGUAGES C ASM)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

option(BUILD_TESTING "Build original game tests" OFF)
option(CRASH_GTE_ORACLE "Check the C ports of the hand-written MIPS routines against their translation" OFF)
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
    if(CRASH_GTE_ORACLE)
        target_compile_definitions(${target} PRIVATE CRASH_GTE_ORACLE)
    endif()
    if(MSVC)
        target_compile_options(${target} PRIVATE /Gy /Gw /W3)
    else()
        # Pointer/integer conversions are errors: game memory holds PS1
        # addresses (docs/MEMORY_MODEL.md), never truncated host pointers.
        target_compile_options(${target} PRIVATE -ffunction-sections -fdata-sections
            -Wall -Wextra -Wno-unused-parameter -Wno-unused-label
            -Werror=int-conversion -Werror=pointer-integer-compare
            -Werror=pointer-to-int-cast -Werror=int-to-pointer-cast
            -Werror=incompatible-pointer-types -Werror=implicit-function-declaration
            -Werror=incompatible-function-pointer-types -Werror=return-type
            -Werror=deprecated-non-prototype)
    endif()
endfunction()

# Game memory (a PS1 RAM image with every global at its original address)
# and the host side of the PS1-format interfaces.
add_library(crash-platform OBJECT
    src/main/ram_layout.S
    src/platform/ram.c
    src/platform/gpu.c)
target_include_directories(crash-platform PRIVATE include)
target_link_libraries(crash-platform PRIVATE psyz)
target_compile_definitions(crash-platform PRIVATE __psyz)

# C ports of the hand-written MIPS routines (src/main/engine/gte_native.h).
# With CRASH_GTE_ORACLE they are compiled as Native_<name> and checked
# against the translation on every call (src/main/engine/gte_oracle.c).
crash_original_module(crash-original-gte-native
    src/main/engine/r_util.c
    src/main/render/slst_decode.c
    src/main/render/world_transform_fx.c
    src/main/render/model_transform.c)
if(CRASH_GTE_ORACLE)
    target_compile_definitions(crash-original-gte-native PRIVATE GTE_NATIVE_IMPL)
    target_compile_options(crash-original-gte-native PRIVATE
        -include ${CMAKE_CURRENT_SOURCE_DIR}/src/main/engine/gte_native.h)
endif()

# Fixed-point math, angles, distances and trigonometry. Modules join as their
# platform dependencies become portable; no duplicate game logic.
crash_original_module(crash-original-math
    src/main/engine/gte_oracle.c
    src/main/engine/gte_render.c
    src/main/math/angles.c
    src/main/math/approx_distance.c
    src/main/math/contains_point.c
    src/main/math/delta.c
    src/main/math/distance.c
    src/main/math/distance_limit.c
    src/main/math/matrix_vector.c
    src/main/math/random_state.c
    src/main/math/scaled_sine.c
    src/main/math/step_halfword.c
    src/main/math/vector_distance.c)

crash_original_module(crash-original-camera
    src/main/cam/camera.c)

crash_original_module(crash-original-core
    src/main/core/main.c)

crash_original_module(crash-original-engine
    src/main/engine/allocate_buffers.c
    src/main/engine/audio_channel_initialize.c
    src/main/engine/audio_initialize.c
    src/main/engine/audio_mode_update.c
    src/main/engine/audio_reset.c
    src/main/engine/audio_sequence_gain.c
    src/main/engine/audio_stop.c
    src/main/engine/audio_stop_current.c
    src/main/engine/audio_transition_update.c
    src/main/engine/audio_voice.c
    src/main/engine/bios_ctrl.c
    src/main/engine/card.c
    src/main/engine/cd_dir.c
    src/main/engine/cd_read.c
    src/main/engine/display_clear.c
    src/main/engine/field138.c
    src/main/engine/fixed_result.c
    src/main/engine/format.c
    src/main/engine/format_copy.c
    src/main/engine/grid_cell.c
    src/main/engine/grid_rectangle.c
    src/main/engine/level_effects.c
    src/main/engine/level_resource_choose.c
    src/main/engine/level_resource_initialize.c
    src/main/engine/level_start.c
    src/main/engine/message_callback.c
    src/main/engine/midi_load.c
    src/main/engine/noop_175C8.c
    src/main/engine/noop_26138.c
    src/main/engine/noop_2F6A8.c
    src/main/engine/noop_entries.c
    src/main/engine/object_defaults.c
    src/main/engine/object_state_filter.c
    src/main/engine/release_buffers.c
    src/main/engine/reset.c
    src/main/engine/resource_adjust.c
    src/main/engine/reverb_disable.c
    src/main/engine/reverb_set.c
    src/main/engine/settings_reset.c
    src/main/engine/shutdown.c
    src/main/engine/state.c
    src/main/engine/state_word_copy.c
    src/main/engine/stream_dispatch.c
    src/main/engine/tag_list.c
    src/main/engine/tree_message.c
    src/main/engine/tree_recursive.c
    src/main/engine/tree_roots.c
    src/main/engine/tree_visit_first.c
    src/main/engine/tree_visit_recursive.c
    src/main/engine/tree_walk.c
    src/main/engine/update_request.c
    src/main/engine/value_56644.c
    src/main/engine/value_56664.c)

crash_original_module(crash-original-gool
    src/main/gool/alloc.c
    src/main/gool/bitfield.c
    src/main/gool/broadcast.c
    src/main/gool/change_state.c
    src/main/gool/create_object.c
    src/main/gool/entries_9D08.c
    src/main/gool/entries_A2F4.c
    src/main/gool/entries_A3CC.c
    src/main/gool/entries_A5E0.c
    src/main/gool/entries_A8CC.c
    src/main/gool/entries_AAE0.c
    src/main/gool/frame_restore.c
    src/main/gool/groups.c
    src/main/gool/interpret.c
    src/main/gool/object.c
    src/main/gool/object_release_callback.c
    src/main/gool/path_follow.c
    src/main/gool/physics.c
    src/main/gool/project.c
    src/main/gool/query_nearest.c
    src/main/gool/resource.c
    src/main/gool/send_event.c
    src/main/gool/test_state_flags.c
    src/main/gool/text_draw.c
    src/main/gool/translate_gop.c
    src/main/gool/update_object.c
    src/main/gool/vm_operands.c)

crash_original_module(crash-original-level
    src/main/level/checkpoint.c
    src/main/level/collision.c
    src/main/level/grid_search.c
    src/main/level/grid_workspace.c
    src/main/level/light.c
    src/main/level/octree.c
    src/main/level/path_sample.c
    src/main/level/update.c
    src/main/level/zone_query.c)

crash_original_module(crash-original-libc
    src/main/libc/malloc.c)

crash_original_module(crash-original-ns
    src/main/ns/alloc.c
    src/main/ns/cd.c
    src/main/ns/cd_seek.c
    src/main/ns/encoding.c
    src/main/ns/entry_lookup.c
    src/main/ns/hash_lookup.c
    src/main/ns/id_string.c
    src/main/ns/init.c
    src/main/ns/load_526C.c
    src/main/ns/load_5408.c
    src/main/ns/load_5500.c
    src/main/ns/load_5634.c
    src/main/ns/load_5918.c
    src/main/ns/load_5C58.c
    src/main/ns/load_list.c
    src/main/ns/name.c
    src/main/ns/page.c
    src/main/ns/page_4170.c
    src/main/ns/page_free.c
    src/main/ns/parse_value.c
    src/main/ns/release.c
    src/main/ns/resource_handle_resolve.c
    src/main/ns/resource_process.c
    src/main/ns/resource_resolve_check.c
    src/main/ns/resource_type.c
    src/main/ns/slot_4E44.c
    src/main/ns/slot_release.c
    src/main/ns/slot_unref.c
    src/main/ns/slots.c
    src/main/ns/texture_allocate.c
    src/main/ns/wait.c)

crash_original_module(crash-original-render
    src/main/render/buffer.c
    src/main/render/buffer_swap.c
    src/main/render/camera_matrix.c
    src/main/render/display.c
    src/main/render/frame_update.c
    src/main/render/gte.c
    src/main/render/mode_initialize.c
    src/main/render/object_transform.c
    src/main/render/quad_draw.c
    src/main/render/tile_fade.c
    src/main/render/transform.c
    src/main/render/transform_88CC.c
    src/main/render/transform_8964.c
    src/main/render/tri_draw.c
    src/main/render/value_range.c
    src/main/render/word_transform.c
    src/main/render/world_setup.c)

# The executable: the platform layer (disc, executable loading, crt0, BIOS)
# starts the original CORE_Main.
add_executable(crash-bandicoot-pc
    src/platform/main.c
    src/platform/bios.c
    src/platform/ram_code.c
    src/platform/interp.c
    $<TARGET_OBJECTS:crash-platform>
    $<TARGET_OBJECTS:crash-original-math>
    $<TARGET_OBJECTS:crash-original-gte-native>
    $<TARGET_OBJECTS:crash-original-camera>
    $<TARGET_OBJECTS:crash-original-core>
    $<TARGET_OBJECTS:crash-original-engine>
    $<TARGET_OBJECTS:crash-original-gool>
    $<TARGET_OBJECTS:crash-original-level>
    $<TARGET_OBJECTS:crash-original-libc>
    $<TARGET_OBJECTS:crash-original-ns>
    $<TARGET_OBJECTS:crash-original-render>)
target_include_directories(crash-bandicoot-pc PRIVATE include)
target_link_libraries(crash-bandicoot-pc PRIVATE psyz)
target_compile_definitions(crash-bandicoot-pc PRIVATE __psyz)
psyz_title(crash-bandicoot-pc "Crash Bandicoot")

if(BUILD_TESTING)
    add_executable(test-original-math tests/original_math.c
        $<TARGET_OBJECTS:crash-original-math> $<TARGET_OBJECTS:crash-original-gte-native>
        $<TARGET_OBJECTS:crash-platform>)
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
