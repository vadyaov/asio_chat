cmake_minimum_required(VERSION 3.14)

project(asio_chat
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/asio-1.30.2/include)

add_executable(chat_server server.cpp)
add_executable(chat_client client.cpp)