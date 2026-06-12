cmake_minimum_required(VERSION 3.15)
project(crypto_live)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON) 

find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network)

add_executable(crypto_live
    src/main.cpp
    src/mainwindow.cpp
    res.qrc
)

target_include_directories(crypto_live PRIVATE include)

target_link_libraries(crypto_live
    Qt6::Core
    Qt6::Widgets
    Qt6::Network
)