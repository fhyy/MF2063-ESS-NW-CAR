################### Build Project #######################################################
cmake_minimum_required (VERSION 2.8)
project(bbb-camera)

set(CMAKE_CXX_FLAGS "-g -std=c++0x")

find_package(vsomeip 2.6.0 REQUIRED)
find_package(Boost 1.62 COMPONENTS system thread log REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${VSOMEIP_INCLUDE_DIRS})

add_executable(camera-service "${PROJECT_SOURCE_DIR}/src/camera_service.cpp")
target_link_libraries(camera-service vsomeip ${Boost_LIBRARIES})

#########################################################################################

################### Link to vsomeip.json ################################################
set(target ${CMAKE_CURRENT_LIST_DIR}/vsomeip.json)
set(link ${CMAKE_CURRENT_LIST_DIR}/build/vsomeip.json)

ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${target} ${link})
#########################################################################################
################### Link Python source in build for convenience #########################
set(target_py ${CMAKE_CURRENT_LIST_DIR}/src/color_detection_cpp.py)
set(link_py ${CMAKE_CURRENT_LIST_DIR}/build/color_detection_cpp.py)

ADD_CUSTOM_TARGET(link_target_py ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${target_py} ${link_py})
#########################################################################################
