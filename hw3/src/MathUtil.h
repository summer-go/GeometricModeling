#pragma once

#include <vector>

#include "Eigen/Dense"

class MathUtil {
public:
    constexpr static const float Pi = 3.141592653589793f;

    static std::vector<float> InterpolationPolygon(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
        float lb, float rb, float step);

    static std::vector<float> ApproximationPolygon(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
        int m, float lb, float rb, float step);

    static std::vector<float> CubicSpline(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
        float lb, float rb, float step);

    static std::vector<float>
    ParameterizationUniform(const std::vector<float> &pos_x, const std::vector<float> &pos_y);

    static std::vector<float>
    ParameterizationChoral(const std::vector<float> &pos_x, const std::vector<float> &pos_y);

    static std::vector<float>
    ParameterizationCentripetal(const std::vector<float> &pos_x, const std::vector<float> &pos_y);

    static std::vector<float>
    ParameterizationFoley(const std::vector<float> &pos_x, const std::vector<float> &pos_y);
};
