cmake_minimum_required(VERSION 3.2)
project(ChatRoom)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES main.cpp)
add_executable(ChatRoom ${SOURCE_FILES} chatroom.cpp chatroom.h)