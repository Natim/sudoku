cmake_minimum_required(VERSION 3.16)
project(sudoku LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(sudoku
  src/main.cpp
  src/bitmap.cpp
  src/dialog_box.cpp
  src/encoding.cpp
  src/sudoku_grid.cpp
  src/window.cpp
)

target_include_directories(sudoku PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/include
)

target_compile_definitions(sudoku PRIVATE
  SUDOKU_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_compile_options(sudoku PRIVATE -Wall -Wextra)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(GRAPHLIB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/lib/libgraphlib_64.a")
else()
  set(GRAPHLIB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/lib/libgraphlib_32.a")
endif()

add_library(graphlib STATIC IMPORTED GLOBAL)
set_target_properties(graphlib PROPERTIES IMPORTED_LOCATION "${GRAPHLIB_LIB}")

find_package(X11 REQUIRED)

target_link_libraries(sudoku PRIVATE graphlib X11::X11)
