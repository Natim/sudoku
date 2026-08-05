cmake_minimum_required(VERSION 3.16)
project(sudoku LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(CMAKE_SIZEOF_VOID_P EQUAL 8)
  set(GRAPHLIB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/lib/libgraphlib_64.a")
else()
  set(GRAPHLIB_LIB "${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/lib/libgraphlib_32.a")
endif()

add_library(graphlib STATIC IMPORTED GLOBAL)
set_target_properties(graphlib PROPERTIES IMPORTED_LOCATION "${GRAPHLIB_LIB}")

find_package(X11 REQUIRED)

# Everything but main(), so the checks in scripts/ link the very code they check
# instead of a list of sources of their own.
add_library(sudoku_core STATIC
  src/bitmap.cpp
  src/celebration.cpp
  src/dialog_box.cpp
  src/encoding.cpp
  src/generator.cpp
  src/random.cpp
  src/score.cpp
  src/sudoku_grid.cpp
  src/window.cpp
)

target_include_directories(sudoku_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/include
)

target_compile_definitions(sudoku_core PUBLIC
  SUDOKU_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets"
)

target_compile_options(sudoku_core PUBLIC -Wall -Wextra)

target_link_libraries(sudoku_core PUBLIC graphlib X11::X11)

add_executable(sudoku src/main.cpp)
target_link_libraries(sudoku PRIVATE sudoku_core)

# The desktop entry points at the executable and icon by absolute path: the
# game runs from the source tree without an install step.
set(SUDOKU_EXEC "${CMAKE_CURRENT_BINARY_DIR}/sudoku")
set(SUDOKU_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/sudoku.svg")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/sudoku.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/sudoku.desktop"
  @ONLY
)

# Checks and helpers, left out of the default build: the Makefile builds and
# runs one at a time, as make verify-celebration.
foreach(check verify_bitmap verify_celebration verify_generator write_sample_grid)
  add_executable(${check} EXCLUDE_FROM_ALL scripts/${check}.cpp)
  target_link_libraries(${check} PRIVATE sudoku_core)
endforeach()
