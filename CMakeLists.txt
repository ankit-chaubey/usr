cmake_minimum_required(VERSION 3.10)
project(usr C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)

# =================== Compiler Flags ===================
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Release)
endif()

# Optimized release flags
set(CMAKE_C_FLAGS_RELEASE "-O3 -march=native -DNDEBUG")
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g -DDEBUG")

# Safety and warning flags
if(CMAKE_C_COMPILER_ID MATCHES "GNU|Clang")
    add_compile_options(
        -Wall -Wextra -Wshadow -Wformat=2 -Wstrict-prototypes
        -fstack-protector-strong
    )
    # Enable extra hardening in release
    if(CMAKE_BUILD_TYPE STREQUAL "Release")
        add_compile_options(-D_FORTIFY_SOURCE=2)
    endif()
endif()

# =================== Include Paths ===================
include_directories(${PROJECT_SOURCE_DIR}/include)

# Internal include path for src/crypto/*.h files
include_directories(${PROJECT_SOURCE_DIR}/src/crypto)

# =================== Source Files ===================
set(USR_SRC
    src/bytes/bytes.c
    src/bytes/error.c
    src/string/strbuilder.c
    src/utf8/utf8.c
    src/binary/binary.c
    src/media/media.c
    src/encoding/encoding.c
    src/entities/entities.c
    src/html/html.c
    src/markdown/markdown.c
    # Crypto
    src/crypto/aes_tables.c
    src/crypto/aes_block.c
    src/crypto/aes_block_decrypt.c
    src/crypto/aes_ige.c
    src/crypto/aes_cbc.c
    src/crypto/aes_ctr.c
    src/crypto/sha256.c
    src/crypto/sha512.c
    src/crypto/hmac.c
    src/crypto/crc32.c
    src/crypto/rand.c
)

# =================== Libraries ===================
add_library(usr        STATIC ${USR_SRC})
add_library(usr_shared SHARED ${USR_SRC})

set_target_properties(usr        PROPERTIES OUTPUT_NAME "usr")
set_target_properties(usr_shared PROPERTIES OUTPUT_NAME "usr"
                                            VERSION 0.1.3
                                            SOVERSION 0)

# =================== Tests ===================
enable_testing()

add_executable(test_roundtrip   tests/test_roundtrip.c)
add_executable(test_crypto      tests/test_crypto.c)
add_executable(test_encoding    tests/test_encoding.c)
add_executable(test_utf8        tests/test_utf8.c)
add_executable(fuzz_roundtrip   tests/fuzz_roundtrip.c)

target_link_libraries(test_roundtrip   usr)
target_link_libraries(test_crypto      usr)
target_link_libraries(test_encoding    usr)
target_link_libraries(test_utf8        usr)
target_link_libraries(fuzz_roundtrip   usr)

add_test(NAME roundtrip  COMMAND test_roundtrip)
add_test(NAME crypto     COMMAND test_crypto)
add_test(NAME encoding   COMMAND test_encoding)
add_test(NAME utf8       COMMAND test_utf8)
add_test(NAME fuzz       COMMAND fuzz_roundtrip)

# =================== Example ===================
add_executable(full_demo examples/full_demo.c)
target_link_libraries(full_demo usr)

# =================== Benchmark ===================
add_executable(bench_crypto benchmarks/bench_crypto.c)
target_link_libraries(bench_crypto usr)

# =================== Install ===================
install(TARGETS usr usr_shared
        ARCHIVE DESTINATION lib
        LIBRARY DESTINATION lib
        RUNTIME DESTINATION bin)

install(DIRECTORY include/usr DESTINATION include)
