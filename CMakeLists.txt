cmake_minimum_required(VERSION 3.20)

project(tcp_server)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)

FetchContent_MakeAvailable(json)

add_executable(tcp_server
    src/main.cpp
    src/HttpParser.cpp
    src/HttpResponseBuilder.cpp
    src/HttpStatus.cpp
    src/Router.cpp
    src/HttpServer.cpp
)

target_include_directories(tcp_server
    PRIVATE
    include
)    

target_link_libraries(tcp_server
    PRIVATE
    nlohmann_json::nlohmann_json
)