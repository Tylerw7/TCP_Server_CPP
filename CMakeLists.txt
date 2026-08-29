cmake_minimum_required(VERSION 3.20)

project(tcp_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tcp_server
    main.cpp
    HttpParser.cpp
    HttpResponseBuilder.cpp
    HttpStatus.cpp
    )