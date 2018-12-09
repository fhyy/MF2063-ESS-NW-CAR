################### Build Project #######################################################
cmake_minimum_required (VERSION 2.8)
project(bbb-motor-speed)

set(CMAKE_CXX_FLAGS "-g -std=c++0x")

find_package(vsomeip 2.6.0 REQUIRED)
find_package(Boost 1.62 COMPONENTS system thread log REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${VSOMEIP_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/lib/shared_memory")
add_subdirectory(lib/shared_memory) 
include_directories("${PROJECT_SOURCE_DIR}/lib/simple_gpio")
add_subdirectory(lib/simple_gpio) 

add_executable(motor-speed-service "${PROJECT_SOURCE_DIR}/src/motor_speed_service.cpp")
target_link_libraries(motor-speed-service vsomeip cbuf shm pthread rt ${Boost_LIBRARIES})

add_executable(speed "${PROJECT_SOURCE_DIR}/src/speed.cpp")
target_link_libraries(speed gpio cbuf shm pthread rt)
#########################################################################################

################### Link to vsomeip.json ################################################
set(target ${CMAKE_CURRENT_LIST_DIR}/vsomeip.json)
set(link ${CMAKE_CURRENT_LIST_DIR}/build/vsomeip.json)

ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${target} ${link})
#########################################################################################
