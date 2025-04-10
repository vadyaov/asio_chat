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

set(SERVER_SOURCES
    server/room.cpp
    server/room-mgr.cc
    server/session.cpp
    server/server.cpp
    server/command.cpp
    server/server_main.cpp
    message_parser.hpp
)

set(CLIENT_SOURCES
    client/client.cpp
    client/client_main.cpp
    message_parser.hpp
)

add_executable(server ${SERVER_SOURCES})
add_executable(client ${CLIENT_SOURCES})

if (CMAKE_BUILD_TYPE STREQUAL "Debug")
    target_compile_definitions(server PRIVATE DEBUG_BUILD)
    target_compile_definitions(client PRIVATE DEBUG_BUILD)
endif()