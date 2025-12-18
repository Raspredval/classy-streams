cmake_minimum_required(VERSION 3.16)

project(classy-streams LANGUAGES CXX VERSION 0.1)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(classy-streams
    INTERFACE)
file(GLOB LIBRARY_HEADERS "${CMAKE_SOURCE_DIR}/include/*.hpp")
target_include_directories(classy-streams
    INTERFACE
        $<BUILD_INTERFACE:${CMAKE_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:${CMAKE_SOURCE_DIR}/include>)

install(
    FILES ${LIBRARY_HEADERS}
    DESTINATION "include/classy-streams"
    COMPONENT classy-streams-headers)

include(CPack)

set(CPACK_PACKAGE_NAME "classy-streams")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_DESCTIPTION_SUMMARY "an attempt to 'fix' stl iostreams, add basic network capability")
set(CPACK_INSTALL_PREFIX "/usr/local/")

cpack_add_component(classy-streams-headers
    DISPLAY "Header files")

include(CTest)

add_executable(classy-tests
    "source/tests.cpp")
target_compile_options(classy-tests
    PRIVATE
        "-Wall" "-Wextra" "-Wconversion" "-Werror")
target_include_directories(classy-tests
    PRIVATE
        "include/")

add_test(
    NAME    classy-tests
    COMMAND classy-tests)