cmake_minimum_required(VERSION 3.16)
project(bej_tests C CXX)

set(CMAKE_C_STANDARD 11)
set(CMAKE_CXX_STANDARD 17)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_executable(bej_tests
  test_nnint.cc
  test_io.cc
  test_dict_synth.cc
  test_decoder_synth.cc
  test_writer.cc
  test_decoder_memory.cc
)

target_include_directories(bej_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/include
)

target_sources(bej_tests PRIVATE
  ${CMAKE_SOURCE_DIR}/source/io.c
  ${CMAKE_SOURCE_DIR}/source/sequence.c
  ${CMAKE_SOURCE_DIR}/source/dict.c
  ${CMAKE_SOURCE_DIR}/source/decoder.c
  ${CMAKE_SOURCE_DIR}/source/writer.c
)
target_link_libraries(bej_tests PRIVATE gtest gtest_main)
enable_testing()
add_test(NAME bej_tests COMMAND bej_tests)
