cmake_minimum_required(VERSION 3.14)

project(asio_net
    VERSION 1.0
    LANGUAGES CXX
)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

include_directories(${PROJECT_SOURCE_DIR}/asio-1.30.2/include)

add_executable(main1 main.cpp)
add_executable(main2 main2.cpp)

add_executable(daytime1 daytime1.cpp)
add_executable(daytime2 daytime2.cpp)
add_executable(daytime3 daytime3.cpp)

add_executable(daytime4 daytime4.cpp)