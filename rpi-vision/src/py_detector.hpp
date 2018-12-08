#include <Python.h>

/**
 * @brief Class that wraps around the Python function that does the image processing
 *
 * @author Leon Fernandez
 */
class PyDetector {
public:
    /**
     * @brief Object representing the Python module that has been loaded.
     *
     */
    PyObject *pModule;

    /**
     * @brief Object representing the Python function that will be called.
     *
     */
    PyObject *pFunc;

    /**
     * @brief Object representing the return value from the called Python function.
     *
     */
    PyObject *pValue;

    /**
     * @brief Constructor that also initializes the Python-objects.
     *
     */
    pyDetector();

    /**
     * @brief Destructor that also properly destroys the Python-objects.
     *
     */
    ~pyDetector();

    /**
     * @brief Method that does a feature extraction.
     * @return the encoded feature extraction.
     *
     * This method wraps around the Python function defined in
     * "color_detection_cpp.py" by using the Python/C API.
     */
    int pyDetectFlag();
}
