cmake_minimum_required(VERSION 3.10)

set(MOONNET_VERSION "1.0.3")
project(moonnet VERSION ${MOONNET_VERSION} LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_BUILD_TYPE "Release")
if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    set(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O2 -Wall -g -ggdb")
    message(STATUS "CMAKE_BUILD_TYPE = Debug")
else()
    set(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O2 -Wall")
    message(STATUS "CMAKE_BUILD_TYPE = Release")
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/bin)

include(GNUInstallDirs)

include_directories(include)

add_subdirectory(src)
add_subdirectory(example)

install(DIRECTORY include/moonnet/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/moonnet)

include(CMakePackageConfigHelpers)

write_basic_package_version_file(
  "${CMAKE_CURRENT_BINARY_DIR}/moonnetConfigVersion.cmake"
  VERSION ${MOONNET_VERSION}
  COMPATIBILITY AnyNewerVersion
)

configure_package_config_file(
  "${CMAKE_CURRENT_SOURCE_DIR}/cmake/moonnetConfig.cmake.in"
  "${CMAKE_CURRENT_BINARY_DIR}/moonnetConfig.cmake"
  INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonnet
)

install(FILES
  "${CMAKE_CURRENT_BINARY_DIR}/moonnetConfig.cmake"
  "${CMAKE_CURRENT_BINARY_DIR}/moonnetConfigVersion.cmake"
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonnet
)

export(EXPORT moonnetTargets
  FILE "${CMAKE_CURRENT_BINARY_DIR}/moonnetTargets.cmake"
  NAMESPACE moonnet::
)

install(EXPORT moonnetTargets
  FILE moonnetTargets.cmake
  NAMESPACE moonnet::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/moonnet
)

add_custom_target(clean-all
    COMMAND ${CMAKE_COMMAND} -E echo "Checking for build directory..."
    COMMAND ${CMAKE_COMMAND} -E echo "Removing files in bin and lib directories..."
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/bin/*
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${PROJECT_SOURCE_DIR}/lib/*
    COMMAND ${CMAKE_COMMAND} -E echo "Removing build directory..."
    COMMAND ${CMAKE_COMMAND} -E remove_directory ${PROJECT_SOURCE_DIR}/build
    COMMENT "Clean all build, bin, and lib files."
)
