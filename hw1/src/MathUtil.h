#pragma once

#include <vector>

#include "Eigen/Dense"

class MathUtil {
public:
    static std::vector<Eigen::Vector2f>
    InterpolationPolygon(const std::vector<Eigen::Vector2f> &in_pos, float lb, float rb, float step);

    static std::vector<Eigen::Vector2f>
    InterpolationGauss(const std::vector<Eigen::Vector2f> &in_pos, float sigma2, int m,
        float lb, float rb, float step);

    static std::vector<Eigen::Vector2f>
    InterpolationGauss1(const std::vector<Eigen::Vector2f> &in_pos, float sigma2, int m,
        float lb, float rb, float step);

    static std::vector<Eigen::Vector2f>
    ApproximationPolygon(const std::vector<Eigen::Vector2f> &in_pos, int m, float lb, float rb, float step);


    static std::vector<Eigen::Vector2f>
    ApproximationPolygon1(const std::vector<Eigen::Vector2f> &in_pos, int m, float lb, float rb, float step);


    static std::vector<Eigen::Vector2f>
    ApproximationNormalized(const std::vector<Eigen::Vector2f> &in_pos, int m, float lambda,
        float lb, float rb, float step);

    static std::vector<Eigen::Vector2f>
    ApproximationNormalized1(const std::vector<Eigen::Vector2f> &in_pos, int m, float lambda,
        float lb, float rb, float step);
};


