add_library(ctrl-sm controller_statemachine.cpp)
target_link_libraries(essproto)
link_directories("${PROJECT_SOURCE_DIR}/lib/ess_prototype"
                 "${PROJECT_SOURCE_DIR}/lib/controller_statemachine")
