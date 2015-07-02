########################################################################
# Project setup
########################################################################
cmake_minimum_required(VERSION 2.6)
project(telecan CXX C)
set(TELECAN_MAJOR_VERSION 0)
set(TELECAN_MINOR_VERSION 1)
set(TELECAN_PATCH_VERSION 07f1e34)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0 -ggdb")

option(test "Build all tests." OFF)
########################################################################
# Compiler specific setup
########################################################################
if(CMAKE_COMPILER_IS_GNUCXX AND NOT WIN32)
  #http://gcc.gnu.org/wiki/Visibility
  add_definitions(-fvisibility=hidden)
endif()

list(APPEND CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake/Modules)

########################################################################
# Find boost
########################################################################
if(UNIX AND EXISTS "/usr/lib64")
    list(APPEND BOOST_LIBRARYDIR "/usr/lib64") #fedora 64-bit fix
endif(UNIX AND EXISTS "/usr/lib64")
set(Boost_ADDITIONAL_VERSIONS
    "1.35.0" "1.35" "1.36.0" "1.36" "1.37.0" "1.37" "1.38.0" "1.38" "1.39.0" "1.39"
    "1.40.0" "1.40" "1.41.0" "1.41" "1.42.0" "1.42" "1.43.0" "1.43" "1.44.0" "1.44"
    "1.45.0" "1.45" "1.46.0" "1.46" "1.47.0" "1.47" "1.48.0" "1.48" "1.49.0" "1.49"
    "1.50.0" "1.50" "1.51.0" "1.51" "1.52.0" "1.52" "1.53.0" "1.53" "1.54.0" "1.54"
    "1.55.0" "1.55" "1.56.0" "1.56" "1.57.0" "1.57" "1.58.0" "1.58" "1.59.0" "1.59"
    "1.60.0" "1.60" "1.61.0" "1.61" "1.62.0" "1.62" "1.63.0" "1.63" "1.64.0" "1.64"
    "1.65.0" "1.65" "1.66.0" "1.66" "1.67.0" "1.67" "1.68.0" "1.68" "1.69.0" "1.69"
)
find_package(Boost "1.35" COMPONENTS filesystem system)

if(NOT Boost_FOUND)
    message(FATAL_ERROR "Boost required to compile gr-gsm")
endif()

########################################################################
# Find gnuradio build dependencies
########################################################################
find_package(Gnuradio)
find_package(LibHackRF)

if(NOT GNURADIO_RUNTIME_FOUND)
    message(FATAL_ERROR "GnuRadio Runtime required to compile gr-gsm")
endif()

########################################################################
# Setup the include and linker paths
########################################################################
include_directories(
    src
    ${Boost_INCLUDE_DIRS}
    ${CPPUNIT_INCLUDE_DIRS}
    ${GNURADIO_RUNTIME_INCLUDE_DIRS}
)

link_directories(
    ${Boost_LIBRARY_DIRS}
    ${CPPUNIT_LIBRARY_DIRS}
    ${GNURADIO_ALL_LIBRARY_DIRS}
)

file(GLOB_RECURSE SRC_FILES ${PROJECT_SOURCE_DIR}/src/*.cc)
list(REMOVE_ITEM SRC_FILES
  ${CMAKE_CURRENT_SOURCE_DIR}/src/telecom_analyzer.cc)
add_library(telecanlib ${SRC_FILES})
add_executable(telecan ${CMAKE_CURRENT_SOURCE_DIR}/src/telecom_analyzer.cc)
target_link_libraries(telecanlib ${Boost_LIBRARIES}
  ${GNURADIO_ALL_LIBRARIES} ${LIBHACKRF_LIBRARIES})
target_link_libraries(telecan telecanlib)

if (test)
  # This adds another subdirectory, which has 'project(gtest)'.
  add_subdirectory(lib/gmock-1.7.0)

  enable_testing()

  # Include the gtest library. gtest_SOURCE_DIR is available due to
  # 'project(gtest)' above.
  include_directories(
    ${gtest_SOURCE_DIR}/include
    ${gmock_SOURCE_DIR}/include
    ${gtest_SOURCE_DIR}
    ${gmock_SOURCE_DIR})

  add_custom_target(check COMMAND ${CMAKE_CTEST_COMMAND} --verbose)

  ##############
  # Unit Tests
  ##############
  file(GLOB_RECURSE TEST_FILES ${PROJECT_SOURCE_DIR}/test/*.cc)
  add_executable(runUnitTests
    ${TEST_FILES})

  # Extra linking for the project.
  target_link_libraries(runUnitTests telecanlib)

  # Standard linking to gtest stuff.
  target_link_libraries(runUnitTests gtest gmock gtest_main)

  add_test(GTest runUnitTests)
endif()
