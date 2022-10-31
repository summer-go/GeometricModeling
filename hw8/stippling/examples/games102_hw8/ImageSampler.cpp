//
// Created by pupa on 12/17/20.
//

#include "ImageSampler.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>
#include <stdint.h>

#include <iostream>
using Eigen::Vector2d;

ImageSampler::ImageSampler(std::string filename)
{
    if (filename.empty())
        return;

    int width_, height_, n_;
    std::uint8_t* img = stbi_load(filename.c_str(), &width_, &height_, &n_, 1);
    if ( img == nullptr) return ;
    std::cout << "image " << filename << " > " << width_ << ' ' << height_ << ' '<< n_ << std::endl;
    image_.resize(height_, width_);
    for (int j = 0; j < height_; j++)
        for (int i = 0; i < width_; i++)
            image_(j, i) = img[j * width_ + i];

    //    std::copy(img, img + width_ * height_ * sizeof(std::uint8_t), image_.data());
//    stbi_write_png((filename+"1.png").c_str(), width_, height_, 1, image_.data(), width_);
    stbi_image_free(img);
//    Eigen::Matrix<std::uint8_t, -1, -1> M(2048, 2048);
//    for(int i = 0; i < M.rows(); i++ ){
//        for(int j = 0; j< M.cols(); j++) {
//            M(i,j) = this->operator()(double(i)/M.rows(), double(j)/M.cols());
//        }
//    }
//    stbi_write_png((filename+".png").c_str(), M.cols(), M.rows(), 1, M.data(), M.cols());
}

// 对灰度图采样
// 实际上是采样了四个点，很经典的采样插值计算，不好解释，看代码吧
double ImageSampler::operator()(double y, double x) const
{
    // 判断边界
    x = x > 0 ? x : 0;
    x = x > 1.0 ? 1.0 : x;
    y = y > 0 ? y : 0;
    y = y > 1.0 ? 1.0 : y;
    x = (image_.cols() - 1) * x;
    y = (image_.rows() - 1) * y;
    
    size_t floor_x = std::floor(x), floor_y = std::floor(y);
    size_t ceil_x = std::ceil(x), ceil_y = std::ceil(y);

    double _x = x - floor_x, _y = y - floor_y;

    // 这儿 _x  _y 弄反了？？
    double floor_pixel = pixel(floor_y, floor_x) * (1 - _x) + pixel(ceil_y, floor_x) * _x;
    double ceil_pixel = pixel(floor_y, ceil_x) * (1 - _x) + pixel(ceil_y, ceil_x) * _x;
    return (floor_pixel * (1 - _y) + ceil_pixel * _y);
}

double inline area_2(double x1, double y1, double x2, double y2, double x3, double y3)
{
    return abs((x1 - x3) * (y2 - y3) - (x2 - x3) * (y1 - y3));
}


// 对灰度图采样，计算加权质心
// 注意，灰度图单个像素点灰度值越高越白，最后生成的SVG图是以black来填充，所以采样时用1 - color/255，即取反，和svg保持一致
Eigen::Vector3d ImageSampler::centric(double x1, double y1, double x2, double y2, double x3, double y3) {
    // 生成采样矩形 (max - min)/3 为采样间距
    auto [max_x, min_x] = std::minmax({x1, x2, x3});
    auto [max_y, min_y] = std::minmax({y1, y2, y3});
    double area = area_2(x1, y1, x2, y2, x3, y3);
    double dx = (max_x - min_x)/3;
    double dy = (max_y - min_y)/3;
    double integral_x = 0, integral_y = 0;
    double total_d = 0;
    for(size_t i = 0; i <= 3; i++ ) {
        for(size_t j = 0; j <= 3; j++) {
            double x = min_x + i * dx;
            double y = min_y + j * dy;
            // 计算叉乘，以判断采样点是否在三角形内部
            double alpha = area_2(x, y, x2, y2, x3, y3)/area;
            double beta = area_2(x1, y1, x, y, x3, y3)/area;
            double gamma = (1-alpha -beta);
            if( 0 <= alpha && alpha <= 1 &&  beta >= 0 && beta <= 1.0 && gamma >= 0 && gamma <= 1.0 ){
                // 对采样点的值平方，生成每个采样点的权重
                // 这里的平方我没有太理解，猜测可能是为了增加权重的差异度，使点图的轮廓更明显，类似锐化吧
                double d = std::max(0.001, std::pow(1 - this->operator()(y, x)/255, 2));
                integral_x += d * x;
                integral_y += d * y;
                total_d += d;
            }
        }
    }
    return {integral_x, integral_y, total_d};
}
