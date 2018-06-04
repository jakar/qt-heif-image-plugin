#
# heif_image_plugin listfile
#

cmake_minimum_required(VERSION 3.2)

project(heifimageplugin)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/imageformats")

# compiler flags
# TODO: separate GCC and Clang warnings; add more
set(
  CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
  -Wall \
  -Wextra \
  -Wshadow \
  -Wformat-nonliteral \
  -Wformat-security \
  -Wnon-virtual-dtor \
  "
)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -Og")

#
# third-party libs
#

# qt
find_package(Qt5 COMPONENTS Core Gui REQUIRED)
add_definitions(-DQT_NO_KEYWORDS)
set(CMAKE_AUTOMOC ON)

# libheif
find_package(PkgConfig)
pkg_check_modules(libheif REQUIRED IMPORTED_TARGET libheif>=1.2)

#
# project source
#

include_directories(
  BEFORE
  "${CMAKE_CURRENT_SOURCE_DIR}/src"
  "${CMAKE_CURRENT_BINARY_DIR}/src"
)

file(GLOB sources src/*.cpp)

add_library(heifimageplugin MODULE ${sources})

target_link_libraries(
  heifimageplugin
  PRIVATE
  Qt5::Gui
  PkgConfig::libheif
)

message("cmake library path: ${CMAKE_LIBRARY_PATH}")

install(TARGETS heifimageplugin
        DESTINATION ${CMAKE_LIBRARY_PATH}/qt5/plugins/imageformats)