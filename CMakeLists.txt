# Part of CMake configuration for parsertl library
#
# Distributed under the Boost Software License, Version 1.0. (See accompanying
# file licence_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
#
cmake_minimum_required (VERSION 3.5)
project(parsertl VERSION 1.0.0)

# Define project
add_library(parsertl INTERFACE)
target_include_directories(parsertl INTERFACE 
	$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
	$<INSTALL_INTERFACE:include>
)

if (BUILD_EXAMPLES)
	add_subdirectory(examples)
endif()

if (BUILD_TESTING)
	enable_testing()
	add_subdirectory(tests)
endif()

# Generate install
include(GNUInstallDirs)

install(TARGETS parsertl EXPORT parsertlTargets
	ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
	RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
	PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

export(EXPORT parsertlTargets
 FILE ${CMAKE_BINARY_DIR}/cmake/parsertlTargets.cmake
 NAMESPACE parsertl::
)

install(DIRECTORY include/parsertl DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT parsertlTargets
	FILE parsertlTargets.cmake
	NAMESPACE parsertl::
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/parsertl
)
include(CMakePackageConfigHelpers)

configure_package_config_file(
  ${CMAKE_SOURCE_DIR}/cmake/parsertlConfig.cmake.in
  ${CMAKE_BINARY_DIR}/cmake/parsertlConfig.cmake
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/parsertl
)

write_basic_package_version_file(
    ${CMAKE_BINARY_DIR}/cmake/parsertlConfigVersion.cmake
	COMPATIBILITY AnyNewerVersion
)
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/cmake/parsertlConfig.cmake" "${CMAKE_CURRENT_BINARY_DIR}/cmake/parsertlConfigVersion.cmake"
	DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/parsertl
)

