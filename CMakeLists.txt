cmake_minimum_required(VERSION 3.20)

project(tcp_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcp_server
    src/main.cpp
    src/HttpParser.cpp
    src/HttpResponseBuilder.cpp
    src/HttpStatus.cpp
    src/Router.cpp
)

target_include_directories(tcp_server
    PRIVATE
    include
)    