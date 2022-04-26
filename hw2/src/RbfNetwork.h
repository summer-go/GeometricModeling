#pragma once

#include <vector>

#ifdef MINGW
// MinGW will complain that there is no 'hypot', including "math.h" can fix it (but including "cmath" cannot)
#include <math.h>
#endif

#include <math.h>
#define PY_SSIZE_T_CLEAN
#ifdef _DEBUG
#undef _DEBUG
#include "Python.h"
#define _DEBUG
#else
//#include "python3.7m/Python.h"
#include "python3.6m/Python.h"
#endif

#include "Eigen/Dense"

class RbfNetwork {
public:
    bool Initialize();
    void Finalize();

    std::vector<Eigen::Vector2f>
    Train(const std::vector<Eigen::Vector2f> &in_pos, int num_middle, int epochs, float lb, float rb, float step);

private:
    PyObject *py_module = nullptr;
    PyObject *py_func_train = nullptr;
};


