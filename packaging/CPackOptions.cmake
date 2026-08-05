# Read once per generator by cpack, after the settings of CMakeLists.txt.

# The package installs under /usr, but the archive is unpacked anywhere: it
# holds bin/ and share/ next to each other, and the program finds its images
# from its own location.
if(CPACK_GENERATOR MATCHES "TGZ|TXZ|TBZ2|ZIP")
  set(CPACK_PACKAGING_INSTALL_PREFIX "")
  set(CPACK_PACKAGE_FILE_NAME
    "sudoku-${CPACK_PACKAGE_VERSION}-linux-${CPACK_SYSTEM_PROCESSOR}")
endif()
