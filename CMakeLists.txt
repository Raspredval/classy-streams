cmake_minimum_required(VERSION 3.16)

project(classy-streams LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(LIBRARY_HEADERS
    "include/NetworkStreams.hpp"
    "include/ConsoleStreams.hpp"
    "include/BufferStreams.hpp"
    "include/FileStreams.hpp"
    "include/IOReadWrite.hpp"
    "include/IOStreams.hpp")
set(LIBRARY_SOURCES
    "source/NetworkStreams.cpp"
    "source/ConsoleStreams.cpp"
    "source/BufferStreams.cpp"
    "source/FileStreams.cpp"
    "source/IOReadWrite.cpp")

add_library(classy-streams
    SHARED ${LIBRARY_SOURCES})
target_include_directories(classy-streams
    INTERFACE
        "${CMAKE_SOURCE_DIR}/include")

install(
    FILES ${LIBRARY_HEADERS}
    DESTINATION "include/classy-streams"
    COMPONENT classy-streams-headers)
install(
    TARGETS classy-streams
    DESTINATION "lib/"
    COMPONENT classy-streams-binaries)

include(CPack)

set(CPACK_PACKAGE_NAME "classy-streams")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCTIPTION_SUMMARY "an attempt to 'fix' stl iostreams, add basic network capability")
set(CPACK_INSTALL_PREFIX "/usr/local/")

cpack_add_component(classy-streams-headers
    DISPLAY "Header files")
cpack_add_component(classy-streams-binaries
    DISPLAY "Binary files")

set(CXX_WARNINGS "-Wall" "-Wextra" "-Wconversion" "-Werror")

add_executable(test_client_local
    "tests/test_client_local.cpp")
target_compile_options(test_client_local
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_client_local
    PRIVATE
        "include/")

add_executable(test_server_local
    "tests/test_server_local.cpp")
target_compile_options(test_server_local
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_server_local
    PRIVATE
        "include/")

add_executable(test_client_tcp
    "tests/test_client_tcp.cpp")
target_compile_options(test_client_tcp
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_client_tcp
    PRIVATE
        "include/")

add_executable(test_server_tcp
    "tests/test_server_tcp.cpp")
target_compile_options(test_server_tcp
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_server_tcp
    PRIVATE
        "include/")

add_executable(test_binary_io
    "tests/test_binary_io.cpp")
target_compile_options(test_binary_io
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_binary_io
    PRIVATE
        "include/")