#include "MathUtil.h"

namespace {

constexpr float Sqr(float x) {
    return x * x;
}

Eigen::VectorXf LeastSquares(const std::vector<float> &pos_x, const std::vector<float> &pos_y, int m) {
    const int n = pos_x.size();
    Eigen::MatrixXf A(m + 1, m + 1);
    std::vector<float> pow_temp(n, 1.0f);
    for (int i = 0; i < 2 * m + 1; i++) {
        float sum = 0;
        for (int j = 0; j < n; j++) {
            sum += pow_temp[j];
            pow_temp[j] *= pos_x[j];
        }
        for (int j = 0; j <= i; j++) {
            if (j <= m && i - j <= m) {
                A(j, i - j) = sum;
            }
        }
    }

    Eigen::MatrixXf norm = Eigen::MatrixXf::Identity(m + 1, m + 1);

    Eigen::MatrixXf Y(m + 1, 1);
    std::fill(pow_temp.begin(), pow_temp.end(), 1.0f);
    for (int i = 0; i <= m; i++) {
        Y(i, 0) = 0.0f;
        for (int j = 0; j < n; j++) {
            Y(i, 0) += pos_y[j] * pow_temp[j];
            pow_temp[j] *= pos_x[j];
        }
    }

    Eigen::VectorXf B = A.colPivHouseholderQr().solve(Y);
    return B;
}

}

std::vector<float> MathUtil::InterpolationPolygon(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
    float lb, float rb, float step) {
    const int n = pos_x.size();
    std::vector<float> result;
    for (float x = lb; x <= rb; x += step) {
        float y = 0;
        for (int i = 0; i < n; i++) {
            float temp = pos_y[i];
            for (int j = 0; j < n; j++) {
                if (i != j) {
                    temp = temp * (x - pos_x[j]) / (pos_x[i] - pos_x[j]);
                }
            }
            y += temp;
        }
        result.push_back(y);
    }
    return result;
}

std::vector<float> MathUtil::ApproximationPolygon(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
    int m, float lb, float rb, float step) {
    const int n = pos_x.size();
    m = std::min(m, std::max(n - 1, 0));
    Eigen::VectorXf B = LeastSquares(pos_x, pos_y, m);
    std::vector<float> result;
    for (float x = lb; x <= rb; x += step) {
        float y = 0, x_temp = 1.0f;
        for (int i = 0; i <= m; i++) {
            y += B(i, 0) * x_temp;
            x_temp *= x;
        }
        result.push_back(y);
    }
    return result;
}

std::vector<float> MathUtil::CubicSpline(const std::vector<float> &pos_x, const std::vector<float> &pos_y,
    float lb, float rb, float step) {
    const size_t n = pos_x.size();
    if (n == 1) {
        return { 0.0f };
    }

    std::vector<float> diff_x(n - 1);
    for (int i = 0; i < n - 1; i++) {
        diff_x[i] = pos_x[i + 1] - pos_x[i];
    }
    std::vector<float> coe0(n);
    std::vector<float> coe1(n);
    std::vector<float> coe2(n);
    std::vector<float> coe3(n);
    for (int i = 0; i < n; i++) {
        coe0[i] = pos_y[i];
    }

    Eigen::MatrixXf A(n, n);
    Eigen::VectorXf B(n);
    for (size_t i = 0; i < n; i++) {
        for (size_t j = 0; j < n; j++) {
            A(i, j) = 0.0f;
        }
        if (i == 0) {
            A(i, i) = 1.0f;
            B(i, 0) = 0.0f;
        } else if (i == n - 1) {
            A(i, i) = 1.0f;
            B(i, 0) = 0.0f;
        } else {
            A(i, i) = 2 * (diff_x[i] + diff_x[i - 1]);
            A(i, i + 1) = diff_x[i];
            A(i, i - 1) = diff_x[i - 1];
            B(i, 0) = 3.0 / diff_x[i] * (coe0[i + 1] - coe0[i]) - 3.0 / diff_x[i - 1] * (coe0[i] - coe0[i - 1]);
        }
    }

    Eigen::VectorXf C = A.colPivHouseholderQr().solve(B);
    for (int i = 0; i < n; i++) {
        coe2[i] = C(i, 0);
    }
    for (int i = 0; i < n - 1; i++) {
        coe3[i] = (coe2[i + 1] - coe2[i]) / 3.0 / diff_x[i];
        coe1[i] = (coe0[i + 1] - coe0[i]) / diff_x[i] - coe2[i] * diff_x[i] - coe3[i] * diff_x[i] * diff_x[i];
    }

    size_t curr = 0;
    std::vector<float> result;
    for (float x = lb; x <= rb; x += step) {
        if (x > pos_x[curr + 1]) {
            ++curr;
        }
        float X = x - pos_x[curr];
        float y = coe0[curr] + coe1[curr] * X + coe2[curr] * X * X + coe3[curr] * X * X * X;
        result.push_back(y);
    }
    return result;
}

std::vector<float>
MathUtil::ParameterizationUniform(const std::vector<float> &pos_x, const std::vector<float> &pos_y) {
    const size_t n = pos_x.size();
    if (n == 1) {
        return { 0.0f };
    }
    float inv = 1.0f / (n - 1);
    std::vector<float> result(n);
    for (size_t i = 0 ; i < n; i++) {
        result[i] = i * inv;
    }
    return result;
}

std::vector<float>
MathUtil::ParameterizationChoral(const std::vector<float> &pos_x, const std::vector<float> &pos_y) {
    const size_t n = pos_x.size();
    if (n == 1) {
        return { 0.0f };
    }
    float sum = 0.0f;
    std::vector<float> result(n);
    std::vector<float> dist(n - 1);
    for (size_t i = 1; i < n; i++) {
        dist[i - 1] = (Eigen::Vector2f(pos_x[i - 1], pos_y[i - 1]) - Eigen::Vector2f(pos_x[i], pos_y[i])).norm();
        sum += dist[i - 1];
    }
    result[0] = 0.0f;
    for (size_t i = 1; i < n - 1; i++) {
        result[i] = dist[i - 1] / sum;
        result[i] += result[i - 1];
    }
    result[n - 1] = 1.0f;
    return result;
}

std::vector<float>
MathUtil::ParameterizationCentripetal(const std::vector<float> &pos_x, const std::vector<float> &pos_y) {
    const size_t n = pos_x.size();
    if (n == 1) {
        return { 0.0f };
    }
    float sum = 0.0f;
    std::vector<float> result(n);
    std::vector<float> dist_sqrt(n - 1);
    for (size_t i = 1; i < n; i++) {
        float dist = (Eigen::Vector2f(pos_x[i - 1], pos_y[i - 1]) - Eigen::Vector2f(pos_x[i], pos_y[i])).norm();
        dist_sqrt[i - 1] = std::sqrt(dist);
        sum += dist_sqrt[i - 1];
    }
    result[0] = 0.0f;
    for (size_t i = 1; i < n - 1; i++) {
        result[i] = dist_sqrt[i - 1] / sum;
        result[i] += result[i - 1];
    }
    result[n - 1] = 1.0f;
    return result;
}

std::vector<float>
MathUtil::ParameterizationFoley(const std::vector<float> &pos_x, const std::vector<float> &pos_y) {
    const size_t n = pos_x.size();
    if (n == 1) {
        return { 0.0f };
    }
    std::vector<float> dist(n + 1);
    for (size_t i = 1; i < n; i++) {
        dist[i] = (Eigen::Vector2f(pos_x[i - 1], pos_y[i - 1]) - Eigen::Vector2f(pos_x[i], pos_y[i])).norm();
    }
    dist[0] = dist[n] = 0.0f;
    std::vector<float> angle(n);
    for (size_t i = 1; i < n - 1; i++) {
        Eigen::Vector2f a = Eigen::Vector2f(pos_x[i - 1], pos_y[i - 1]) - Eigen::Vector2f(pos_x[i], pos_y[i]);
        Eigen::Vector2f b = Eigen::Vector2f(pos_x[i + 1], pos_y[i + 1]) - Eigen::Vector2f(pos_x[i], pos_y[i]);
        angle[i] = a.dot(b) / dist[i] / dist[i + 1];
        angle[i] = std::min(Pi - angle[i], Pi / 2.0f);
    }
    angle[0] = angle[n - 1] = 0.0f;
    float sum = 0.0f;
    std::vector<float> diff(n - 1);
    for (size_t i = 1; i < n; i++) {
        diff[i - 1] = dist[i] * (1.0f + 1.5f * (angle[i - 1] * dist[i - 1]) / (dist[i - 1] + dist[i]) +
            1.5f * (angle[i] * dist[i + 1]) / (dist[i] + dist[i + 1]));
        sum += diff[i - 1];
    }
    std::vector<float> result(n);
    result[0] = 0.0f;
    for (size_t i = 1; i < n - 1; i++) {
        result[i] = diff[i - 1] / sum;
        result[i] += result[i - 1];
    }
    result[n - 1] = 1.0f;
    return result;
}