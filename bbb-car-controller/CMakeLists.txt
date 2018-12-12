################### Build Project #######################################################
cmake_minimum_required (VERSION 2.8)
project(bbb-car-controller)

set(CMAKE_CXX_FLAGS "-g -std=c++0x")

find_package(vsomeip 2.6.0 REQUIRED)
find_package(Boost 1.62 COMPONENTS system thread log REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${VSOMEIP_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/lib/shared_memory")
add_subdirectory(lib/shared_memory)
include_directories("${PROJECT_SOURCE_DIR}/lib/ess_prototype")
add_subdirectory(lib/ess_prototype)
include_directories("${PROJECT_SOURCE_DIR}/lib/controller_statemachine")
add_subdirectory(lib/controller_statemachine)

add_executable(car-ctrl-client "${PROJECT_SOURCE_DIR}/src/car_ctrl_client.cpp")
target_link_libraries(car-ctrl-client vsomeip cbuf shm pthread rt ${Boost_LIBRARIES})

add_executable(main "${PROJECT_SOURCE_DIR}/src/main.cpp")
target_link_libraries(main cbuf shm essproto ctrl-sm pthread rt)

add_executable(ess_shell "${PROJECT_SOURCE_DIR}/src/ess_shell.cpp")
target_link_libraries(ess_shell cbuf shm essproto pthread rt)
#########################################################################################

################### Link to vsomeip.json ################################################
set(target ${CMAKE_CURRENT_LIST_DIR}/vsomeip.json)
set(link ${CMAKE_CURRENT_LIST_DIR}/build/vsomeip.json)

ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${target} ${link})
#########################################################################################
