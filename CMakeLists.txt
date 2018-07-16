cmake_minimum_required(VERSION 3.5)  # lowest version tried

project(qtheifimageplugin)

set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin/imageformats")

add_subdirectory(src)

# vim:sw=2
