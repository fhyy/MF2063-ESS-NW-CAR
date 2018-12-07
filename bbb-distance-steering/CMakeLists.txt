################### Build Project #######################################################
cmake_minimum_required (VERSION 2.8)
project(bbb-distance-steering)

set(CMAKE_CXX_FLAGS "-g -std=c++0x")

find_package(vsomeip 2.6.0 REQUIRED)
find_package(Boost 1.62 COMPONENTS system thread log REQUIRED)

include_directories(${Boost_INCLUDE_DIR} ${VSOMEIP_INCLUDE_DIRS})

include_directories("${PROJECT_SOURCE_DIR}/lib/shared_memory")
add_subdirectory(lib/shared_memory) 
include_directories("${PROJECT_SOURCE_DIR}/lib/simple_gpio")
add_subdirectory(lib/simple_gpio) 

add_executable(dist-steer-service "${PROJECT_SOURCE_DIR}/src/dist_steer_service.cpp")
target_link_libraries(dist-steer-service vsomeip cbuf shm pthread rt ${Boost_LIBRARIES})

add_executable(distance "${PROJECT_SOURCE_DIR}/src/distance.cpp")
target_link_libraries(distance gpio cbuf shm pthread rt)
#########################################################################################


################### Build Docs ##########################################################
find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_IN ${CMAKE_CURRENT_LIST_DIR}/Doxyfile.in)
    set(DOXYGEN_OUT ${CMAKE_CURRENT_LIST_DIR}/doc/Doxyfile)
    configure_file(${DOXYGEN_IN} ${DOXYGEN_OUT} @ONLY)

    add_custom_target(doxygen-doc ALL
        COMMAND ${DOXYGEN_EXECUTABLE} ${DOXYGEN_OUT}
        COMMENT "Doxygen is working..."
	SOURCES ${DOXYGEN_OUT})

else()
  message("Doxygen not found. No documentation will be generated")
endif()
#########################################################################################

################### Link to vsomeip.json ################################################
set(target ${CMAKE_CURRENT_LIST_DIR}/vsomeip.json)
set(link ${CMAKE_CURRENT_LIST_DIR}/build/vsomeip.json)

ADD_CUSTOM_TARGET(link_target ALL
                  COMMAND ${CMAKE_COMMAND} -E create_symlink ${target} ${link})
#########################################################################################
