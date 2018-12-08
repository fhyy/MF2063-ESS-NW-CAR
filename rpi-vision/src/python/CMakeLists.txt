find_package(PythonLibs REQUIRED)
include_directories(${PYTHON_INCLUDE_DIRS})
add_library(pydetect py_detector.cpp)
target_link_libraries(pydetect ${PYTHON_LIBRARIES})

