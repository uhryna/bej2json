cmake_minimum_required(VERSION 3.14)
project(bej2json C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
add_compile_options(-Wall -Wextra)

# Заголовки
include_directories(${CMAKE_SOURCE_DIR}/include)

# Бібліотека з модулів
add_library(bej STATIC
  source/io.c
  source/writer.c
  source/sequence.c
  source/dict.c
  source/decoder.c
)

include(CTest)     # створює опцію BUILD_TESTING і підготовлює CTest
enable_testing()   # вмикає генерацію CTest-файлів на верхньому рівні
add_subdirectory(tests)
# CLI
add_executable(bej2json source/main.c)
target_link_libraries(bej2json PRIVATE bej)
