#include <math.h>
#include <vector>
#include <Eigen/Dense>
#include <iostream>

// 插值点
struct NodeArr {
    NodeArr() :size(0){}
    NodeArr(int _size) : size(_size), xs(std::vector<double>(_size)), ys(std::vector<double>(_size)) {}
    NodeArr(std::vector<double> x, std::vector<double> y) : xs(x), ys(y), size(x.size()) {}
    unsigned int size;
    std::vector<double> xs;
    std::vector<double> ys;
};

// 控制点
struct controlPoint {
    double val = 0;
    // 切线左边的距离
    double ldiff = 0;
    // 切线右边的距离
    double rdiff = 0;
    // 是否固定，如果已经编辑了切线，表示这个点就固定下来了
    bool fixed_diff = false;
};

// 三次采样函数，最重要的函数
std::vector<double> ThreeOrderSample(const std::vector<controlPoint>& ctrlarr, int begin, int end);

class ControlPointArray2D {
public:
    // 计算P段三次函数
    void calculateParam_extend(int p);
    int32_t size() {
        return param.size();
    }
    uint32_t nodenum() {
        assert(xs.size()==ys.size() and ys.size()==fixed.size());
        return fixed.size();
    }

    // 采样插值
    void calculateRange(int p);
    void clear() {
        while (nodenum() != 0) {
            xs.clear();
            ys.clear();
            fixed.clear();
            drawPoints.clear();
            param.clear();
            drawarr.xs.clear();
            drawarr.ys.clear();
            drawarr.size = 0;
            ctrlxs.clear();
            ctrlys.clear();
            ctrl_l_xs.clear();
            ctrl_l_ys.clear();
            ctrl_r_xs.clear();
            ctrl_r_ys.clear();
            controlbars.clear();
        }
    }
    void push_back(double x, double y);
    void delete_at(int pos);
    void setPoint(int p, double x, double y) {

        xs[p].val = x;
        ys[p].val = y;

        // 分段计算曲线，尽力复用
        // 修改一个点，分两段进行计算,p-1 -> p 和 p -> p+1

        // 计算p-1 --> p两个点之间的曲线
        if (p > 0) {
            calculateParam_extend(p - 1);
        }
        else if (p == 0) {  // 计算第一个点
            calculateParam_extend(0);
        }

        // 分段计算，计算p --> p+1 段
        if (p < nodenum() - 1) {
            calculateParam_extend(p);
        }
    }
    void setLDiff(int p, double x, double y) {
        fixed[p] = true;
        xs[p].fixed_diff = true;
        ys[p].fixed_diff = true;
        xs[p].ldiff = (xs[p].val - x)/showpara; //showpara 缩放切线长度，太长了有点乱
        ys[p].ldiff = (ys[p].val - y)/showpara;
        if (p > 0) {
            calculateParam_extend(p - 1);
        }
    }
    void setRDiff(int p, double x, double y) {
        fixed[p] = true;
        xs[p].fixed_diff = true;
        ys[p].fixed_diff = true;
        xs[p].rdiff = (x - xs[p].val)/showpara;
        ys[p].rdiff = (y - ys[p].val)/showpara;
        if (p < fixed.size() - 1) {
            calculateParam_extend(p);
        }
    }
    NodeArr& getDrawPoints() {
        return drawarr;
    }
    std::vector<double>& getControlPointx() {
        return ctrlxs;
    }
    std::vector<double>& getControlPointy() {
        return ctrlys;
    }

    std::vector<std::vector<double>>& getControlBar() { return controlbars; }

    int findSuitableCtrlPoint(double x, double y);
private:
    void setFixDiff(int pos) {
        fixed.at(pos) = true;
        xs.at(pos).fixed_diff = true;
        ys.at(pos).fixed_diff = true;
    }
    
    std::vector<controlPoint> xs;
    std::vector<controlPoint> ys;
    std::vector<bool> fixed;

    //绘图的点
    std::vector<NodeArr> drawPoints;
    std::vector<std::vector<double>> param;
    uint64_t nodePerRange = 100;

    //
    NodeArr drawarr;
    void update_drawarr() {
        drawarr.xs.clear();
        drawarr.ys.clear();
        for (int i = 0; i < size(); ++i) {
            drawarr.xs.insert(drawarr.xs.end(), drawPoints[i].xs.begin(), drawPoints[i].xs.end());
            drawarr.ys.insert(drawarr.ys.end(), drawPoints[i].ys.begin(), drawPoints[i].ys.end());
        }
        drawarr.size = drawarr.xs.size();
    }

    //ctrlpoints
    std::vector<double> ctrlxs;
    std::vector<double> ctrlys;
    double maxChoosedist = 0.02;

    double showpara = 0.4;

    // 更新插入点，及切线控制点
    void updateCtrlPoints() {
        ctrlxs.resize(xs.size());
        ctrlys.resize(xs.size());
        ctrl_l_xs.resize(xs.size());
        ctrl_l_ys.resize(xs.size());
        ctrl_r_xs.resize(xs.size());
        ctrl_r_ys.resize(xs.size());
        controlbars.resize(xs.size());
        for (int i = 0; i < xs.size(); ++i) {
            ctrlxs[i] = xs[i].val;
            ctrlys[i] = ys[i].val;
            ctrl_l_xs[i] = xs[i].val - xs[i].ldiff * showpara;
            ctrl_l_ys[i] = ys[i].val - ys[i].ldiff * showpara;
            ctrl_r_xs[i] = xs[i].val + xs[i].rdiff * showpara;
            ctrl_r_ys[i] = ys[i].val + ys[i].rdiff * showpara;
            controlbars[i] = { ctrl_l_xs[i], ctrl_l_ys[i], ctrlxs[i], ctrlys[i], ctrl_r_xs[i], ctrl_r_ys[i] };
        }
        
    }

    //
    std::vector<double> ctrl_l_xs;
    std::vector<double> ctrl_l_ys;
    std::vector<double> ctrl_r_xs;
    std::vector<double> ctrl_r_ys;
    std::vector<std::vector<double>> controlbars;
};
/*
void Normalize(std::vector<double> &arr) {
    double len = arr.back();
    for (auto &x : arr)
        x /= len;
}


std::vector<double> Parametrization_uniform(const NodeArr &src) {
    using namespace std;
    assert(src.size > 1);
    vector<double> res(src.size, 0);
    for (int i=0; i<res.size(); ++i) {
        res[i] = i;
    }
    Normalize(res);
    return res;
}

std::vector<double> Parametrization_chordal(const NodeArr &src) {
    using namespace std;
    assert(src.size > 1);
    vector<double> res(src.size, 0);
    for (int i=1; i<res.size(); ++i) {
        res[i] = res[i-1] + sqrt(pow(src.xs[i]-src.xs[i-1], 2) + 
                                pow(src.ys[i]-src.ys[i-1], 2));
    }
    Normalize(res);
    return res;
}

std::vector<double> Parametrization_centripetal(const NodeArr &src) {
    using namespace std;
    assert(src.size > 1);
    vector<double> res(src.size, 0);
    for (int i=1; i<res.size(); ++i) {
        res[i] = res[i-1] + sqrt(sqrt(pow(src.xs[i]-src.xs[i-1], 2) + 
                                pow(src.ys[i]-src.ys[i-1], 2)));
    }
    Normalize(res);
    return res;
}

double foley_dist(double x1, double y1, double x2, double y2) {
    return sqrt(pow(x2-x1, 2) + pow(y2-y1, 2));
}

double foley_alpha_hat(double x1, double y1, double x2, double y2, double x3, double y3) {
    double dx1 = x2-x1, dy1 = y2-y1, dx2 = x3-x2, dy2 = y3-y2;
    double dot = -dx1*dx2 - dy1*dy2;
    double l1 = foley_dist(x1, y1, x2, y2), l2 = foley_dist(x2, y2, x3, y3);
    double angle = acos(dot/(l1*l2));
    angle = std::min(4*atan(1)-angle, 2*atan(1));
    return angle;
}


std::vector<double> Parametrization_foley(const NodeArr &src) {
    using namespace std;
    assert(src.size > 3);
    vector<double> res(src.size, 0);
    for (int i=1; i<res.size(); ++i) {
        bool has_left = true, has_right = true;
        double angle_l, angle_r, length, length_l, length_r;
        if (i == 1) has_left = false;
        if (i == res.size()-1) has_right = false;
        length = foley_dist(src.xs[i-1], src.ys[i-1], src.xs[i], src.ys[i]);
        if (has_left) {
            angle_l = foley_alpha_hat(src.xs[i-2], src.ys[i-2], src.xs[i-1], src.ys[i-1], src.xs[i], src.ys[i]);
            length_l = foley_dist(src.xs[i-2], src.ys[i-2], src.xs[i-1], src.ys[i-1]);
        }
        if (has_right) {
            angle_r = foley_alpha_hat(src.xs[i-1], src.ys[i-1], src.xs[i], src.ys[i], src.xs[i+1], src.ys[i+1]);
            length_r = foley_dist(src.xs[i], src.ys[i], src.xs[i+1], src.ys[i+1]);
        }
        double base_len = length;
        double mul = 1;
        if (has_left) {
            mul += 1.5*angle_l*length_l/(length_l+length);
        }
        if (has_right) {
            mul += 1.5*angle_r*length_r/(length_r+length);
        }
        res[i] = res[i-1] + mul*base_len;
    }
    Normalize(res);
    return res;
}


// 多项式
void InterPolation_1(NodeArr &src, NodeArr &tar) {
    using namespace Eigen;
    const int num_pow = src.size;
    Matrix<double, Dynamic, Dynamic> A(num_pow, num_pow);
    VectorXd x(num_pow), b(num_pow);
    
    // build A and b
    for (int i=0; i<num_pow; ++i) {
        for (int j=0; j<num_pow; ++j) {
            A(i, j) = pow(src.xs[i], j);
        }
    }

    for (int i=0; i<num_pow; ++i) {
        b[i] = src.ys[i];
    }

    x = A.colPivHouseholderQr().solve(b);
    
    // calculate fit curve
    double st_region = src.xs[0], ed_region = src.xs[src.size-1];
    std::vector<double> resxs, resys;
    for (double xx=st_region; xx<ed_region; xx+=0.001) {
        double yy = 0;
        for (int i=0; i<num_pow; ++i) {
            yy += x(i)*std::pow(xx, i);
        }
        resxs.push_back(xx);
        resys.push_back(yy);
    }
    tar.xs.swap(resxs);
    tar.ys.swap(resys);
    tar.size = tar.xs.size();
    return;
}

// 高斯基函数
void InterPolation_2(NodeArr &src, NodeArr &tar) {
    using namespace Eigen;
    const int num_pow = src.size;
    Matrix<double, Dynamic, Dynamic> A(num_pow+1, num_pow+1);
    VectorXd x(num_pow+1), b(num_pow+1);
    double para1 = (src.xs.back()-src.xs.front())/(src.size-1);
    
    // build A and b
    for (int i=0; i<num_pow; ++i) {
        for (int j=0; j<num_pow; ++j) {
            A(i, j) = exp(-pow(src.xs[i]-src.xs[j], 2)/(2*pow(para1, 2)));
        }
        A(i, num_pow) = 1;
    }
    for (int j=0; j<num_pow; ++j) {
        A(num_pow, j) = 1;
    }

    for (int i=0; i<num_pow; ++i) {
        b[i] = src.ys[i];
    }
    b[num_pow] = 0;

    x = A.colPivHouseholderQr().solve(b);
    
    // calculate fit curve
    double st_region = src.xs[0], ed_region = src.xs[src.size-1];
    std::vector<double> resxs, resys;
    for (double xx=st_region; xx<ed_region; xx+=0.001) {
        double yy = 0;
        for (int i=0; i<num_pow; ++i) {
            yy += x(i)*exp(-pow(xx-src.xs[i], 2)/(2*para1*para1));
        }
        yy += x(num_pow);
        resxs.push_back(xx);
        resys.push_back(yy);
    }
    tar = NodeArr(resxs, resys);
}

void LeastSquare(NodeArr &src, NodeArr &tar, int order) {
    using namespace Eigen;
    int nodenum = src.size;
    Matrix<double, Dynamic, Dynamic> A(order, order);
    VectorXd x(order), b(order);
    
    // build A and b
    for (int i=0; i<order; ++i) {
        for (int j=0; j<order; ++j) {
            A(i, j) = 0;
            for (int k=0; k<nodenum; ++k) {
                A(i, j) += pow(src.xs[k], i+j);
            }
        }
    }

    for (int i=0; i<order; ++i) {
        b(i) = 0;
        for (int k=0; k<nodenum; ++k) {
            b(i) += pow(src.xs[k], i)*src.ys[k];
        }
    }

    x = A.colPivHouseholderQr().solve(b);
    
    // calculate fit curve
    double st_region = src.xs[0], ed_region = src.xs[src.size-1];
    std::vector<double> resxs, resys;
    for (double xx=st_region; xx<ed_region; xx+=0.001) {
        double yy = 0;
        for (int i=0; i<order; ++i) {
            yy += x(i)*pow(xx, i);
        }
        resxs.push_back(xx);
        resys.push_back(yy);
    }
    tar = NodeArr(resxs, resys);
}

void Ridge_Regression(NodeArr &src, NodeArr &tar, int order, double l) {
    using namespace Eigen;
    int nodenum = src.size;
    Matrix<double, Dynamic, Dynamic> A(order, order);
    VectorXd x(order), b(order);
    
    // build A and b
    for (int i=0; i<order; ++i) {
        for (int j=0; j<order; ++j) {
            A(i, j) = 0;
            for (int k=0; k<nodenum; ++k) {
                A(i, j) += pow(src.xs[k], i+j);
            }
            if (i == j) {
                A(i, j) += 2*l;
            }
        }
    }

    for (int i=0; i<order; ++i) {
        b(i) = 0;
        for (int k=0; k<nodenum; ++k) {
            b(i) += pow(src.xs[k], i)*src.ys[k];
        }
    }

    x = A.colPivHouseholderQr().solve(b);
    
    // calculate fit curve
    double st_region = src.xs[0], ed_region = src.xs[src.size-1];
    std::vector<double> resxs, resys;
    for (double xx=st_region; xx<ed_region; xx+=0.001) {
        double yy = 0;
        for (int i=0; i<order; ++i) {
            yy += x(i)*pow(xx, i);
        }
        resxs.push_back(xx);
        resys.push_back(yy);
    }
    tar = NodeArr(resxs, resys);
}
*/