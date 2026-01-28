cmake_minimum_required(VERSION 3.10)
project(usr C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =========================
# Include paths
# =========================
include_directories(${PROJECT_SOURCE_DIR}/include)

# =========================
# Core library sources
# =========================
file(GLOB_RECURSE USR_SRC
    src/core/*.c
    src/bytes/*.c
    src/utf8/*.c
    src/binary/*.c
    src/media/*.c
    src/crypto/*.c
    src/entities/*.c
    src/markdown/*.c
    src/html/*.c
)

# =========================
# Static library
# =========================
add_library(usr STATIC ${USR_SRC})
set_target_properties(usr PROPERTIES OUTPUT_NAME "usr")

# =========================
# Shared library (IMPORTANT)
# =========================
add_library(usr_shared SHARED ${USR_SRC})
set_target_properties(usr_shared PROPERTIES OUTPUT_NAME "usr")

# =========================
# Tests
# =========================
enable_testing()

add_executable(test_roundtrip tests/test_roundtrip.c)
target_link_libraries(test_roundtrip usr)

add_executable(fuzz_roundtrip tests/fuzz_roundtrip.c)
target_link_libraries(fuzz_roundtrip usr)

add_test(NAME roundtrip COMMAND test_roundtrip)
add_test(NAME fuzz COMMAND fuzz_roundtrip)
