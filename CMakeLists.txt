#
# qt-heif-image-plugin listfile
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

set(sanitizer_flags "-fsanitize=address -fsanitize=undefined")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} ${sanitizer_flags}")
set(CMAKE_LINKER_FLAGS_DEBUG
    "${CMAKE_MODULE_LINKER_FLAGS_DEBUG} ${sanitizer_flags}")

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
# logging
#

set(HEIF_IMAGE_PLUGIN_ENABLE_TRACE OFF CACHE BOOL "Enable trace logging")

if (${HEIF_IMAGE_PLUGIN_ENABLE_TRACE})
  add_definitions(-DHEIF_IMAGE_PLUGIN_ENABLE_TRACE)
endif ()

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

#
# installation (adapted from lxqt-qtplugin)
#

get_target_property(
  QT_QMAKE_EXECUTABLE ${Qt5Core_QMAKE_EXECUTABLE} IMPORTED_LOCATION)

if (NOT QT_QMAKE_EXECUTABLE)
  message(FATAL_ERROR "qmake is not found.")
endif ()

execute_process(
  COMMAND ${QT_QMAKE_EXECUTABLE} -query QT_INSTALL_PLUGINS
  OUTPUT_VARIABLE QT_PLUGINS_DIR
  OUTPUT_STRIP_TRAILING_WHITESPACE
)

if (QT_PLUGINS_DIR)
  message(STATUS "Qt5 plugin directory:" "${QT_PLUGINS_DIR}")
else ()
  message(FATAL_ERROR "Qt5 plugin directory cannot be detected.")
endif ()

install(TARGETS heifimageplugin
        LIBRARY DESTINATION "${QT_PLUGINS_DIR}/imageformats")
