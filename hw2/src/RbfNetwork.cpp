#include "RbfNetwork.h"

#include <iostream>

bool RbfNetwork::Initialize() {
    Py_Initialize();
    PyRun_SimpleString("import sys");
    PyRun_SimpleString("print(sys.version)");
    PyRun_SimpleString("sys.path.append('./')");

    PyObject *module_name = PyUnicode_DecodeFSDefault("rbf_nn");
    py_module = PyImport_Import(module_name);
    Py_DECREF(module_name);
    if (!py_module) {
        std::cout << "Failed to load Python module 'rbf_nn'" << std::endl;
        PyErr_Print();
        return false;
    }
    py_func_train = PyObject_GetAttrString(py_module, "train");
    if (!py_func_train || !PyCallable_Check(py_func_train)) {
        std::cout << "Failed to load Python function 'train'" << std::endl;
        PyErr_Print();
        Py_DECREF(py_module);
        return false;
    }
    return true;
}

void RbfNetwork::Finalize() {
    Py_DECREF(py_func_train);
    Py_DECREF(py_module);
}

std::vector<Eigen::Vector2f> RbfNetwork::Train(const std::vector<Eigen::Vector2f> &in_pos, int num_middle, int epochs,
    float lb, float rb, float step) {
    PyObject *func_args = PyTuple_New(5);

    int n_points_in = in_pos.size();
    PyObject *x_list = PyList_New(n_points_in);
    PyObject *y_list = PyList_New(n_points_in);
    for (size_t i = 0; i < in_pos.size(); i++) {
        PyList_SetItem(x_list, i, PyFloat_FromDouble(in_pos[i].x()));
        PyList_SetItem(y_list, i, PyFloat_FromDouble(in_pos[i].y()));
    }

    std::vector<float> x_pred;
    for (float x = lb; x <= rb; x += step) {
        x_pred.push_back(x);
    }
    PyObject *xp_list = PyList_New(x_pred.size());
    for (size_t i = 0; i < x_pred.size(); i++) {
        PyList_SetItem(xp_list, i, PyFloat_FromDouble(x_pred[i]));
    }

    PyTuple_SetItem(func_args, 0, x_list);
    PyTuple_SetItem(func_args, 1, y_list);
    PyTuple_SetItem(func_args, 2, PyLong_FromLong(num_middle));
    PyTuple_SetItem(func_args, 3, PyLong_FromLong(epochs));
    PyTuple_SetItem(func_args, 4, xp_list);

    PyObject *fn_ret = PyObject_CallObject(py_func_train, func_args);
    PyErr_Print();
    if (!fn_ret) {
        std::cout << "Failed to call Python function 'train'" << std::endl;
        PyErr_Print();
        Py_DECREF(x_list);
        Py_DECREF(y_list);
        Py_DECREF(xp_list);
        Py_DECREF(func_args);
        return {};
    }
    std::vector<Eigen::Vector2f> y_pred(x_pred.size());
    for (size_t i = 0; i < y_pred.size(); i++) {
        y_pred[i] = { x_pred[i], PyFloat_AsDouble(PyList_GetItem(fn_ret, i)) };
    }

    Py_DECREF(x_list);
    Py_DECREF(y_list);
    Py_DECREF(xp_list);
    Py_DECREF(func_args);

    return y_pred;
}