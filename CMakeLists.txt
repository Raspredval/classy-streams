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

set(CXX_WARNINGS "-Wall" "-Wextra" "-Wconversion" "-Werror")

add_executable(test_client_local
    "source/test_client_local.cpp")
target_compile_options(test_client_local
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_client_local
    PRIVATE
        "include/")

add_executable(test_server_local
    "source/test_server_local.cpp")
target_compile_options(test_server_local
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_server_local
    PRIVATE
        "include/")

add_test(
    NAME    test_server_local
    COMMAND test_server_local)

add_test(
    NAME    test_client_local
    COMMAND test_client_local
    DEPENDS test_server_local)

add_executable(test_client_tcp
    "source/test_client_tcp.cpp")
target_compile_options(test_client_tcp
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_client_tcp
    PRIVATE
        "include/")

add_executable(test_server_tcp
    "source/test_server_tcp.cpp")
target_compile_options(test_server_tcp
    PRIVATE
        ${CXX_WARNINGS})
target_include_directories(test_server_tcp
    PRIVATE
        "include/")

add_test(
    NAME    test_server_tcp
    COMMAND test_server_tcp)

add_test(
    NAME    test_client_tcp
    COMMAND test_client_tcp
    DEPENDS test_server_tcp)