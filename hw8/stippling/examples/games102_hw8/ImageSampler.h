//
// Created by pupa on 12/17/20.
//

#pragma once

#include <string>
#include <vector>
#include <Eigen/Dense>
#include <stdint.h>
#include <iostream>

class ImageSampler
{
public:
    explicit ImageSampler(std::string filename);

    double operator()(double y, double x) const;

    Eigen::Vector3d centric(double x1, double y1, double x2, double y2, double x3, double y3);

    bool is_constant() { return !image_.size(); }

    size_t width() { return image_.cols(); }

    size_t height() { return image_.rows(); }

private:

    std::uint8_t pixel(size_t row, size_t col) const {
        return image_(row, col);
    }


    Eigen::Matrix<std::uint8_t, -1, -1> image_;
};
