add_library(essproto ess_prototype.cpp)
target_link_libraries(essproto cbuf shm)
link_directories("${PROJECT_SOURCE_DIR}/lib/ess_prototype" 
                 "${PROJECT_SOURCE_DIR}/lib/shared_memory")
