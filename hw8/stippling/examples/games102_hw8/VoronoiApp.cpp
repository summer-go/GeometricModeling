#include "ImageSampler.h"

#include <vector>
#include <string>
#include <sstream>
#include <iostream>
#include <fstream>

#define JCV_REAL_TYPE double
#define JC_VORONOI_IMPLEMENTATION
#include "jc_voronoi.h"

class CentroidVoronoiDiagram
{
public:
    explicit CentroidVoronoiDiagram(jcv_rect bbox, size_t n_sample, std::string density_image = "")
        : density_(density_image), bbox_(bbox)
    {
        if(density_.is_constant()) points_.resize(n_sample);
        else
            points_.resize(128*128*density_.height()/density_.width());
        srand(0);
        for (size_t i = 0; i < points_.size(); i++)
        {
            points_[i].x = (float)(rand() / (1.0f + RAND_MAX));
            points_[i].y = (float)(rand() / (1.0f + RAND_MAX));
        }
    }

    ~CentroidVoronoiDiagram() { if(diagram_.internal) jcv_diagram_free(&diagram_); }

    jcv_point polygon_centroid(const jcv_graphedge* graph_edge);

    jcv_point varying_polygon_centroid(const jcv_graphedge* graph_edge, size_t n_sample = 30);

    // relax操作
    double relax_points()
    {
        // 获取vonoroi图，每个site代表一个网格
        const jcv_site* sites = jcv_diagram_get_sites(&diagram_);
        // 判断收敛的阈值
        double max_diff = 1e-9;
        for (int i = 0; i < diagram_.numsites; ++i)
        {
            const jcv_site* site = &sites[i];
            const jcv_point pre_p = points_[site->index];
            //  以灰度图为权重来计算，即变密度，这篇文章对应的逻辑走这里
            if (!density_.is_constant())
                // 记录每个点的新坐标
                points_[site->index] = varying_polygon_centroid(site->edges);
            else
                // is_constant表示均匀CVT，即不加权重
                points_[site->index] = polygon_centroid(site->edges);

            max_diff = std::max(max_diff, jcv_point_dist_sq(&pre_p, &points_[site->index]) );
        }
        return max_diff;
    }

    // 变密度lloyd
    double lloyd()
    {
        // 每次操作前，先释放上次的上下文内存占用
        if(diagram_.internal)
            jcv_diagram_free(&diagram_);

        // 养成好习惯，初始化内存
        memset(&diagram_, 0, sizeof(jcv_diagram));

        // 调用Jcash/Voronoi库，生成voronoi图
        jcv_diagram_generate(points_.size(), points_.data(), &bbox_, 0, &diagram_);
        // 进行CVT操作，这个过程也叫relax(放松~~形象)
        return relax_points();
    }

    std::string export_svg(std::string file);
private:
    jcv_diagram            diagram_{0};
    std::vector<jcv_point> points_;
    ImageSampler           density_;
    jcv_rect               bbox_;
};

// 主流程
int main(int argc, char** argv)
{
    jcv_rect bounding_box = {{0.0f, 0.0f}, {1.0f, 1.0f}};
    std::string density_image = argv[1];
    // 1. 加载灰度图
    CentroidVoronoiDiagram cvd( bounding_box, 100, density_image);

    // 2. 迭代1000次变密度CVT
    for(int i = 0; i < 1000; i++)
    {
        double max_diff = cvd.lloyd();
        if((i+1)%4 == 0)
            // 导出svg矢量图
            cvd.export_svg(argv[2]+std::to_string(int(i/4))+".svg");
        // 判断收敛，退出
        if(max_diff < 1e-7) break;
        std::cout << "\r" << "lloyd: " << i << ' '<< max_diff << std::endl ;
    }
}

jcv_point CentroidVoronoiDiagram::polygon_centroid(const jcv_graphedge* graph_edge) {
    double total_det = 0;
    jcv_point center{0, 0};
    for(;graph_edge;graph_edge = graph_edge->next)
    {
        jcv_point p1 = graph_edge->pos[0], p2 = graph_edge->pos[1];
        double det = p1.x * p2.y - p2.x * p1.y;
        total_det += det;
        center.x += (p1.x + p2.x) * det;
        center.y += (p1.y + p2.y) * det;
    }
    center.x /= 3 * total_det;
    center.y /= 3 * total_det;
    return center;
}

// 先计算每个网格的几何中心pc，即不加权重
// 以pc为原点，剖分该网格，生成多个三角形，对每个三角形进行带权重的计算质心
// 权重从输入的灰度图采样，每个三角形采样4 * 4 个点(注意判断该采样点必须在三角形内)，逻辑很像三角形光栅化时的抗锯齿操作
jcv_point CentroidVoronoiDiagram::varying_polygon_centroid(const jcv_graphedge* graph_edge,
                                                                  size_t n_sample)
{
    // 先计算每个网格的几何中心pc，即不加权重
    jcv_point pc = polygon_centroid(graph_edge);
    jcv_point center{0, 0};
    double W = 0;
    for(;graph_edge;graph_edge = graph_edge->next)
    {
        jcv_point p1 = graph_edge->pos[0], p2 = graph_edge->pos[1];
        // 以pc为原点，剖分该网格，生成多个三角形，对每个三角形进行带权重的计算质心
        Eigen::RowVector3d centroid = density_.centric(p1.x, p1.y, p2.x, p2.y, pc.x, pc.y);
        center.x += centroid.x();
        center.y += centroid.y();
        W += centroid.z();
    }
    center.x /= W;
    center.y /= W;
    return center;
}

// 导出svg图，导出512的分辨率图
std::string  CentroidVoronoiDiagram::export_svg(std::string output_file) {
    std::stringstream svg_str;
    double w = 512;

    // 均值输出512 * 512， 这里有输入的灰度图，即变密度
    double h = density_.is_constant() ? 512: density_.height()/double(density_.width())*512;
    // svg 头格式
    svg_str << "<svg width=\"" << w << "\" height=\"" << h << "\" viewBox=\"0 0 " << w << ' ' << h
            << R"(" xmlns="http://www.w3.org/2000/svg" >)"
            << "<rect width=\"100%\" height=\"100%\" fill=\"white\"/>\n";

    // 获取voronoi图
    const jcv_site* sites = jcv_diagram_get_sites(&diagram_);
    // 如果是均匀的cvt，输出网格图边框，这次又输入的灰度图，不走这里的逻辑
    for (size_t i = 0; i < diagram_.numsites && density_.is_constant(); i++)
    {
        svg_str << "<polygon points=\"";
        jcv_graphedge* graph_edge = sites[i].edges;
        while (graph_edge)
        {
            jcv_point p1 = graph_edge->pos[0], p2 = graph_edge->pos[1];
            svg_str << p1.x*w << ',' << p1.y*h << ' ' << p2.x*w << ',' << p2.y*h << ' ';
            graph_edge = graph_edge->next;
        }
        svg_str << R"(" fill="#1d1d9b" stroke="white" stroke-width="2" />)" << "\n";
    }

    // 网格值心点转成svg的point值
    for( int i = 0; i < diagram_.numsites; ++i )
    {
        const jcv_site* site = &sites[i];
        jcv_point p = site->p, pc = polygon_centroid(site->edges);
        // 均匀cvt逻辑
        if(density_.is_constant()){
            svg_str << "<circle cx=\"" << pc.x * w  << "\" cy=\"" << pc.y*h << R"(" r="1" fill="red"/>)" << '\n';
            svg_str << "<circle cx=\"" << p.x * w  << "\" cy=\"" << p.y*h << R"(" r="1" fill="yellow"/>)" << '\n';
        }else
            // 变密度cvt逻辑
            svg_str << "<circle cx=\"" << p.x * w  << "\" cy=\"" << p.y*h << R"(" r="1" fill="black"/>)" << '\n';
    }

    svg_str << "</svg>";

    // 写文件
    if(output_file.size() != 0)
    {
        std::ofstream svg_file(output_file);
        svg_file << svg_str.str();
        svg_file.close();
    }

    return svg_str.str();
}
