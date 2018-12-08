#include "py_detect_flag.cpp"

PyDetector::PyDetector() {
    Py_Initialize();
    sysPath = PySys_GetObject((char*)"path");
    PyList_Append(sysPath, PyString_FromString("."));
    PyObject *pName = PyUnicode_DecodeFSDefault("color_detection_cpp");
    pModule = PyImport_Import(pName);
    Py_DECREF(pName);

    pFunc = PyObject_GetAttrString(pModule, "detection");
}

PyDetector::~PyDetector() {
    Py_DECREF(pValue);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
    Py_DECREF(sysPath);
}

int PyDetector::pyDetectFlag() {
    pValue = PyObject_CallObject(pFunc, NULL);
    
    return (int) PyLong_AsLong(pValue);
}


