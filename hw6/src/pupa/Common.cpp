//
// Created by pupa on 12/5/20.
//
#include <pupa/pmp/SurfaceCurvature.h>
#include <pupa/pmp/Differential.h>

namespace pmp_pupa {

double cotan_weight(const SurfaceMesh& mesh, Edge e)
{
    double weight = 0.0;

    const Halfedge h0 = mesh.halfedge(e, 0);
    const Halfedge h1 = mesh.halfedge(e, 1);

    const dvec3 p0 = (dvec3)mesh.position(mesh.to_vertex(h0));
    const dvec3 p1 = (dvec3)mesh.position(mesh.to_vertex(h1));

    if (!mesh.is_boundary(h0))
    {
        const dvec3 p2 = (dvec3)mesh.position(mesh.to_vertex(mesh.next_halfedge(h0)));
        const dvec3 d0 = p0 - p2;
        const dvec3 d1 = p1 - p2;
        const double area = norm(cross(d0, d1));
        if (area > std::numeric_limits<double>::min())
        {
            const double cot = dot(d0, d1) / area;
            weight += clamp_cot(cot);
        }
    }

    if (!mesh.is_boundary(h1))
    {
        const dvec3 p2 = (dvec3)mesh.position(mesh.to_vertex(mesh.next_halfedge(h1)));
        const dvec3 d0 = p0 - p2;
        const dvec3 d1 = p1 - p2;
        const double area = norm(cross(d0, d1));
        if (area > std::numeric_limits<double>::min())
        {
            const double cot = dot(d0, d1) / area;
            weight += clamp_cot(cot);
        }
    }

    assert(!std::isnan(weight));
    assert(!std::isinf(weight));

    return weight;
}

double voronoi_area(const SurfaceMesh& mesh, Vertex v)
{
    double area(0.0);

    if (!mesh.is_isolated(v))
    {
        Halfedge h0, h1, h2;
        dvec3 p, q, r, pq, qr, pr;
        double dotp, dotq, dotr, triArea;
        double cotq, cotr;

        for (auto h : mesh.halfedges(v))
        {
            h0 = h;
            h1 = mesh.next_halfedge(h0);
            h2 = mesh.next_halfedge(h1);

            if (mesh.is_boundary(h0))
                continue;

            // three vertex positions
            p = (dvec3)mesh.position(mesh.to_vertex(h2));
            q = (dvec3)mesh.position(mesh.to_vertex(h0));
            r = (dvec3)mesh.position(mesh.to_vertex(h1));

            // edge vectors
            (pq = q) -= p;
            (qr = r) -= q;
            (pr = r) -= p;

            // compute and check triangle area
            triArea = norm(cross(pq, pr));
            if (triArea <= std::numeric_limits<double>::min())
                continue;

            // dot products for each corner (of its two emanating edge vectors)
            dotp = dot(pq, pr);
            dotq = -dot(qr, pq);
            dotr = dot(qr, pr);

            // angle at p is obtuse
            if (dotp < 0.0)
            {
                area += 0.25 * triArea;
            }

            // angle at q or r obtuse
            else if (dotq < 0.0 || dotr < 0.0)
            {
                area += 0.125 * triArea;
            }

            // no obtuse angles
            else
            {
                // cot(angle) = cos(angle)/sin(angle) = dot(A,B)/norm(cross(A,B))
                cotq = dotq / triArea;
                cotr = dotr / triArea;

                // clamp cot(angle) by clamping angle to [1,179]
                area += 0.125 * (sqrnorm(pr) * clamp_cot(cotq) + sqrnorm(pq) * clamp_cot(cotr));
            }
        }
    }

    assert(!std::isnan(area));
    assert(!std::isinf(area));

    return area;
}

Point laplace(const SurfaceMesh& mesh, Vertex v)
{
    Point laplace(0.0, 0.0, 0.0);

    if (!mesh.is_isolated(v))
    {
        Scalar weight, sumWeights(0.0);

        for (auto h : mesh.halfedges(v))
        {
            weight = cotan_weight(mesh, mesh.edge(h));
            sumWeights += weight;
            laplace += weight * mesh.position(mesh.to_vertex(h));
        }

        laplace -= sumWeights * mesh.position(v);
        laplace /= Scalar(2.0) * voronoi_area(mesh, v);
    }

    return laplace;
}

//!> angle between prev_h and h
double sector_angle(const SurfaceMesh& mesh, Halfedge h)
{
    auto p_f = mesh.position(mesh.from_vertex(mesh.prev_halfedge(h)));
    auto p = mesh.position(mesh.from_vertex(h));
    auto p_t = mesh.position(mesh.to_vertex(h));
    return std::acos(pmp::dot(p_t - p, p_f - p) / (pmp::norm(p_t - p) * pmp::norm(p - p_f)));
}
//!> normal between prev_h and h
pmp::Normal sector_normal(const SurfaceMesh& mesh, Halfedge h)
{
    auto p_f = mesh.position(mesh.from_vertex(mesh.prev_halfedge(h)));
    auto p = mesh.position(mesh.from_vertex(h));
    auto p_t = mesh.position(mesh.to_vertex(h));
    return pmp::cross(p - p_f, p_t - p) / (pmp::norm(p_t - p) * pmp::norm(p - p_f));
}

//!> angle weighted vertex normal
pmp::Normal normal(const SurfaceMesh& mesh, Vertex v)
{
    pmp::Normal n(0);
    double ww{0}, w;
    for (auto h : mesh.halfedges(v))
    {
        w = sector_angle(mesh, h);
        n += w * sector_normal(mesh, h);
        ww += w;
    }
    return n / ww;
}

//!> Curvature
double mean_curvature(const SurfaceMesh& mesh, Vertex v)
{
    if(mesh.is_boundary(v)) return 0;
    pmp::Normal n = normal(mesh, v);
    pmp::Normal l = laplace(mesh, v);
    double curvature_ = pmp::norm(l) / 2;
    return std::signbit(pmp::dot(n, l)) ? curvature_ : -curvature_;
}

double gaussian_curvature(const SurfaceMesh& mesh, Vertex v)
{
    if(mesh.is_boundary(v) ) return 0;
    double curvature_ = M_PI*2;
    for (auto h : mesh.halfedges(v))
    {
        if(mesh.is_boundary(h)) continue;
        curvature_ -= sector_angle(mesh, h);
    }
    return curvature_;
}

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Wrapper<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
Eigen::VectorXd cotan_weight(const SurfaceMesh& mesh)
{
    Eigen::VectorXd e_cotan(mesh.n_edges());
    for (pmp::Edge e : mesh.edges())
        e_cotan[e.idx()] = cotan_weight(mesh, e);
    return e_cotan;
}

Eigen::VectorXd voronoi_area(const SurfaceMesh& mesh)
{
    Eigen::VectorXd v_voronoi(mesh.n_halfedges());
    for (pmp::Vertex v : mesh.vertices())
        v_voronoi[v.idx()] = voronoi_area(mesh, v);
    return v_voronoi;
}

Eigen::MatrixX3d laplace(const SurfaceMesh& mesh)
{
    Eigen::MatrixX3d v_laplace(mesh.n_vertices(), 3);
    for (pmp::Vertex v : mesh.vertices())
        v_laplace.row(v.idx()) = Eigen::Vector3d(laplace(mesh, v));
    return v_laplace;
}

}

pmp::VertexProperty<double>& pmp_pupa::SurfaceCurvature::update_gauss_curvature()
{
    for (auto v : mesh_.vertices())
        gauss_curvature_[v] = fabs(pmp_pupa::gaussian_curvature(mesh_, v));
    return gauss_curvature_;
}

pmp_pupa::VertexProperty<double>& pmp_pupa::SurfaceCurvature::update_mean_curvature()
{
    for (auto v : mesh_.vertices())
        mean_curvature_[v] = fabs(pmp_pupa::mean_curvature(mesh_, v));
    return mean_curvature_;
}

double pmp_pupa::SurfaceCurvature::max_gauss_curvature() {
    if(gauss_kmax_ > 0) return gauss_kmax_;
    std::vector<double> values(mesh_.n_vertices());
    for (auto v : mesh_.vertices())
        values[v.idx()] = gauss_curvature_[v];

    std::sort(values.begin(), values.end());
    gauss_kmax_ = values[values.size() - 1 - int(values.size()/20)];
    return gauss_kmax_;
}
double pmp_pupa::SurfaceCurvature::max_mean_curvature() {
    if(mean_kmax_ > 0) return mean_kmax_;
    std::vector<double> values(mesh_.n_vertices());
    for (auto v : mesh_.vertices())
        values[v.idx()] = mean_curvature_[v];

    std::sort(values.begin(), values.end());
    mean_kmax_ = values[values.size() - 1 - int(values.size()/20)];
    return mean_kmax_;
}