#
# heifimageplugin listfile
#

cmake_minimum_required(VERSION 3.10)

project(heifimageplugin)

set(CMAKE_CXX_STANDARD 17)
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
find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)
add_definitions(-DQT_NO_KEYWORDS)
set(CMAKE_AUTOMOC ON)

# libheif
find_package(PkgConfig)
pkg_check_modules(libheif REQUIRED IMPORTED_TARGET libheif>=1.2)

#
# logging
#

set(
  HEIFIMAGEPLUGIN_ENABLE_LOGGING OFF
  CACHE BOOL "Enable logging"
)
set(
  HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG OFF
  CACHE BOOL "Enable logging: debug level"
)
set(
  HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE OFF
  CACHE BOOL "Enable logging: trace level"
)

set(enable_log ${HEIFIMAGEPLUGIN_ENABLE_LOGGING})
set(enable_log_debug ${HEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG})
set(enable_log_trace ${HEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE})

if (${enable_log_trace})
  set(enable_log_debug ON)
endif ()

if (${enable_log_debug})
  set(enable_log ON)
endif ()

if (enable_log)
  include_directories("${PROJECT_SOURCE_DIR}/thirdparty/spdlog/include")
  add_definitions(-DHEIFIMAGEPLUGIN_ENABLE_LOGGING)

  if (${enable_log_trace})
    add_definitions(-DHEIFIMAGEPLUGIN_ENABLE_LOGGING_TRACE)
  elseif (${enable_log_debug})
    add_definitions(-DHEIFIMAGEPLUGIN_ENABLE_LOGGING_DEBUG)
  endif ()
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
  Qt5::Widgets
  PkgConfig::libheif
)
