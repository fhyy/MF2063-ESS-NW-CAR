cmake_minimum_required(VERSION 2.6)
set(CMAKE_CXX_STANDARD 11)  # enable C++11 standard
set (CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++1y -Wall")

project(bbb-car-controller)
add_executable(test_sm.c car_controller_statemachine.h car_controller_statemachine.c)
#add_library(car_controller_statemachine.h car_controller_statemachine.c)
