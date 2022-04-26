#include "MathUtil.h"

namespace {

constexpr float Sqr(float x) {
    return x * x;
}

Eigen::MatrixXf LeastSquares(const std::vector<Eigen::Vector2f> &in_pos, int m, float lambda = 0.0f) {
    const int n = in_pos.size();
    Eigen::MatrixXf A(m + 1, m + 1);
    std::vector<float> pow_temp(n, 1.0f);
    for (int i = 0; i < 2 * m + 1; i++) {
        float sum = 0;
        for (int j = 0; j < n; j++) {
            sum += pow_temp[j];
            pow_temp[j] *= in_pos[j].x();
        }
        for (int j = 0; j <= i; j++) {
            if (j <= m && i - j <= m) {
                A(j, i - j) = sum;
            }
        }
    }

    Eigen::MatrixXf norm = Eigen::MatrixXf::Identity(m + 1, m + 1);
    A += lambda * norm;

    Eigen::MatrixXf Y(m + 1, 1);
    std::fill(pow_temp.begin(), pow_temp.end(), 1.0f);
    for (int i = 0; i <= m; i++) {
        Y(i, 0) = 0.0f;
        for (int j = 0; j < n; j++) {
            Y(i, 0) += in_pos[j].y() * pow_temp[j];
            pow_temp[j] *= in_pos[j].x();
        }
    }

    Eigen::MatrixXf B = A.inverse() * Y;
    return B;
}

}

std::vector<Eigen::Vector2f>
MathUtil::InterpolationGauss(const std::vector<Eigen::Vector2f> &in_pos, float sigma2, int m,
    float lb, float rb, float step) {
    const int n = in_pos.size();
    m = std::min(m, std::max(n - 1, 0));

    Eigen::MatrixXf B_poly = LeastSquares(in_pos, m);
    std::vector<float> y_approx(n);
    for (int i = 0; i < n; i++) {
        float y = 0, x_temp = 1.0f;
        for (int j = 0; j <= m; j++) {
            y += B_poly(j, 0) * x_temp;
            x_temp *= in_pos[i].x();
        }
        y_approx[i] = y;
    }

    Eigen::MatrixXf A(n, n);
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            A(i, j) = std::exp(-Sqr(in_pos[i].x() - in_pos[j].x()) / (2 * sigma2));
        }
    }

    Eigen::MatrixXf Y(n, 1);
    for (int i = 0; i < n; i++) {
        Y(i, 0) = in_pos[i].y() - y_approx[i];
    }

    Eigen::MatrixXf B = A.inverse() * Y;
    std::vector<Eigen::Vector2f> result;
    for (float x = lb; x <= rb; x += step) {
        float y = 0, x_temp = 1.0f;
        for (int i = 0; i <= m; i++) {
            y += B_poly(i, 0) * x_temp;
            x_temp *= x;
        }
        for (int i = 0; i < n; i++) {
            y += B(i, 0) * std::exp(-Sqr(x - in_pos[i].x()) / (2 * sigma2));
        }
        result.emplace_back(x, y);
    }
    return result;
}

std::vector<Eigen::Vector2f>
MathUtil::ApproximationPolygon(const std::vector<Eigen::Vector2f> &in_pos, int m, float lb, float rb, float step) {
    const int n = in_pos.size();
    m = std::min(m, std::max(n - 1, 0));
    Eigen::MatrixXf B = LeastSquares(in_pos, m);
    std::vector<Eigen::Vector2f> result;
    for (float x = lb; x <= rb; x += step) {
        float y = 0, x_temp = 1.0f;
        for (int i = 0; i <= m; i++) {
            y += B(i, 0) * x_temp;
            x_temp *= x;
        }
        result.emplace_back(x, y);
    }
    return result;
}