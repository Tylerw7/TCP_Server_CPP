cmake_minimum_required(VERSION 3.20)

project(http)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)

FetchContent_Declare(
    json
    URL https://github.com/nlohmann/json/releases/download/v3.12.0/json.tar.xz
)
FetchContent_MakeAvailable(json)

# All server logic, minus main(), lives in a reusable library.
add_library(server_lib
    src/HttpParser.cpp
    src/HttpResponseBuilder.cpp
    src/HttpStatus.cpp
    src/Router.cpp
    src/HttpServer.cpp
)

target_include_directories(server_lib PUBLIC include)
target_link_libraries(server_lib PUBLIC nlohmann_json::nlohmann_json)

# Example usage — this is what a user's server looks like.
add_executable(basic_server examples/basic_server.cpp)
target_link_libraries(basic_server PRIVATE server_lib)

# ---- Testing ----
enable_testing()

FetchContent_Declare(
    catch2
    URL https://github.com/catchorg/Catch2/archive/refs/tags/v3.5.2.tar.gz
)
FetchContent_MakeAvailable(catch2)

add_executable(tests
    tests/test_parser.cpp
    tests/test_middleware.cpp
    tests/test_handle_error.cpp
)
target_link_libraries(tests PRIVATE server_lib Catch2::Catch2WithMain)

include(CTest)
include(Catch)
catch_discover_tests(tests)