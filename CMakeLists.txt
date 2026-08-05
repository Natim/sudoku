cmake_minimum_required(VERSION 3.16)

# Stamped on the packages, and passed the tag being released by the workflow.
set(SUDOKU_VERSION "0.1.0" CACHE STRING "Version of the packages")

project(sudoku VERSION ${SUDOKU_VERSION} LANGUAGES CXX)

include(GNUInstallDirs)

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
  src/paths.cpp
  src/random.cpp
  src/score.cpp
  src/sudoku_grid.cpp
  src/window.cpp
)

target_include_directories(sudoku_core PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_SOURCE_DIR}/third_party/graphlib/include
)

# Last resort of assetsDir(), which otherwise looks beside the executable and so
# follows the build tree, the package and the archive alike.
target_compile_definitions(sudoku_core PUBLIC
  SUDOKU_ASSETS_DIR="${CMAKE_INSTALL_FULL_DATADIR}/sudoku/assets"
)

target_compile_options(sudoku_core PUBLIC -Wall -Wextra)

target_link_libraries(sudoku_core PUBLIC graphlib X11::X11)

add_executable(sudoku src/main.cpp)
target_link_libraries(sudoku PRIVATE sudoku_core)

# The desktop entry of the source tree points at the executable and icon by
# absolute path: the game runs from the build directory without an install step.
set(SUDOKU_EXEC "${CMAKE_CURRENT_BINARY_DIR}/sudoku")
set(SUDOKU_ICON "${CMAKE_CURRENT_SOURCE_DIR}/assets/sudoku.svg")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/sudoku.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/sudoku.desktop"
  @ONLY
)

# The packaged one goes through the PATH and the icon theme instead, the prefix
# being the business of whoever installs it.
set(SUDOKU_EXEC "sudoku")
set(SUDOKU_ICON "sudoku")
configure_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/packaging/sudoku.desktop.in"
  "${CMAKE_CURRENT_BINARY_DIR}/packaging/sudoku.desktop"
  @ONLY
)

# Checks and helpers, left out of the default build: the Makefile builds and
# runs one at a time, as make verify-celebration.
foreach(check verify_bitmap verify_celebration verify_generator write_sample_grid)
  add_executable(${check} EXCLUDE_FROM_ALL scripts/${check}.cpp)
  target_link_libraries(${check} PRIVATE sudoku_core)
endforeach()

install(TARGETS sudoku RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})

install(DIRECTORY assets/images
  DESTINATION ${CMAKE_INSTALL_DATADIR}/sudoku/assets
  FILES_MATCHING PATTERN "*.bmp"
)

install(FILES assets/sudoku.svg
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/icons/hicolor/scalable/apps
)

install(FILES "${CMAKE_CURRENT_BINARY_DIR}/packaging/sudoku.desktop"
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# The MIT licence asks for its notice to travel with the binaries, and Debian
# expects it under that name.
install(FILES LICENSE
  DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/doc/sudoku
  RENAME copyright
)

set(CPACK_PACKAGE_NAME "sudoku")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Rémy Hubscher <remy@yampa.ai>")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Sudoku à jouer à la souris, sur X11")
set(CPACK_PACKAGE_HOMEPAGE_URL "https://github.com/Natim/sudoku")
set(CPACK_STRIP_FILES ON)
set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")
set(CPACK_SYSTEM_PROCESSOR "${CMAKE_SYSTEM_PROCESSOR}")
# The archive is unpacked wherever its user pleases, so it drops the prefix.
set(CPACK_PROJECT_CONFIG_FILE "${CMAKE_CURRENT_SOURCE_DIR}/packaging/CPackOptions.cmake")
set(CPACK_DEBIAN_PACKAGE_SECTION "games")
set(CPACK_DEBIAN_FILE_NAME DEB-DEFAULT)
# Read the libraries the executable ended up needing rather than list them by
# hand: the X11 and C++ runtime packages differ between distributions.
set(CPACK_DEBIAN_PACKAGE_SHLIBDEPS ON)

include(CPack)
