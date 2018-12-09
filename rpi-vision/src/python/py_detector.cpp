#include "py_detector.hpp"

PyDetector::PyDetector() :
    err_msg_("")
{
    Py_Initialize();

    PyObject *pName = PyUnicode_DecodeFSDefault("color_detection_cpp");

    pModule = PyImport_Import(pName);
    if (pModule != NULL) {
        pFunc = PyObject_GetAttrString(pModule, "detection");
        Py_DECREF(pName);
        if (pFunc && PyCallable_Check(pFunc))
            err_msg_ = "";
        else
            err_msg_ = "Function not found!";
    }
    else
        err_msg_ = "Module not found!";


}

PyDetector::~PyDetector() {
    Py_DECREF(pValue);
    Py_DECREF(pFunc);
    Py_DECREF(pModule);
}

int PyDetector::pyDetectFlag() {
    if (err_msg_ == "") {
        pValue = PyObject_CallObject(pFunc, NULL);
        if (pValue != NULL)
            return (int) PyLong_AsLong(pValue);
        else
            std::cerr << "No return value!" << std::endl;
    }
    else
        std::cerr << err_msg_ << std::endl;
    return 0;
}


