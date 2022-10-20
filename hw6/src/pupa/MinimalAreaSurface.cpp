//
// Created by pupa on 12/2/20.
//

#include <pupa/pmp/MinimalAreaSurface.h>
#include <Eigen/Sparse>
#include <pupa/pmp/Differential.h>
using namespace pmp_pupa;


MinimalAreaSurface::MinimalAreaSurface(SurfaceMesh& mesh) : mesh_(mesh)
{
    eweight_ = mesh_.add_edge_property<double>("fairing:eweight");
    vlaplace_ = mesh_.add_vertex_property<pmp::Point>("fairing:vlaplace_");
    inner_idx_ = mesh_.add_vertex_property<int>("fairing:inner_idx", -1);
    boundary_idx_ = mesh_.add_vertex_property<int>("fairing:boundary_idx", -1);


    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            boundary_idx_[v] = n_boundary_vertices_++;
        else
            inner_idx_[v] = n_inner_vertices_++;

    for (auto e : mesh_.edges())
        eweight_[e] = std::max(0.1, cotan_weight(mesh_, e));

    setup_implicit_L();
}

MinimalAreaSurface::~MinimalAreaSurface()
{
    // remove properties
    mesh_.remove_edge_property(eweight_);
    mesh_.remove_vertex_property(vlaplace_);
    mesh_.remove_vertex_property(inner_idx_);
    mesh_.remove_vertex_property(boundary_idx_);
}

void MinimalAreaSurface::explicit_iterate(float lambda, bool boundary)
{
    if (boundary)
        boundary_explicit_iterate(lambda);

    pmp::Vertex vv;

    for (auto v : mesh_.vertices())
    {
        if (mesh_.is_boundary(v))
            continue;
        double w = 0;
        vlaplace_[v] *= 0;
        for (auto h : mesh_.halfedges(v))
        {
            vv = mesh_.to_vertex(h);
            w += eweight_[mesh_.edge(h)];
            vlaplace_[v] += eweight_[mesh_.edge(h)] * (mesh_.position(vv) - mesh_.position(v));
        }
        vlaplace_[v] /= w;
    }

    for (auto v : mesh_.vertices())
    {
        if (!mesh_.is_boundary(v))
            mesh_.position(v) += lambda * vlaplace_[v];
    }
}

void MinimalAreaSurface::boundary_explicit_iterate(float lambda)
{
    pmp::Point prev_center = boundary_center();
    double boundary_std_ = boundary_std();
    pmp::Vertex vv;
    for (auto v : mesh_.vertices())
    {
        if (!mesh_.is_boundary(v))
            continue;
        double w = 0;
        vlaplace_[v] *= 0;
        for (auto h : mesh_.halfedges(v))
        {
            vv = mesh_.to_vertex(h);
            if (!mesh_.is_boundary(vv))
                continue;
            w += eweight_[mesh_.edge(h)];
            vlaplace_[v] += eweight_[mesh_.edge(h)] * (mesh_.position(vv) - mesh_.position(v));
        }
        vlaplace_[v] /= w;
    }

    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            mesh_.position(v) += lambda * vlaplace_[v];

    // restore original surface boundary to avoid shrinking
    double scale = boundary_std_ / boundary_std();
    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            mesh_.position(v) *= scale;

    pmp::Point t = prev_center - boundary_center();
    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            mesh_.position(v) += t;
}

double MinimalAreaSurface::boundary_std()
{
    pmp::Point center = boundary_center();
    double std_ = 0;
    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            std_ += std::pow(pmp::norm(mesh_.position(v) - center), 2);

    return std::sqrt(std_ / n_boundary_vertices_);
}

pmp::Point MinimalAreaSurface::boundary_center()
{
    pmp::Point center(0, 0, 0);
    for (auto v : mesh_.vertices())
        if (mesh_.is_boundary(v))
            center += mesh_.position(v);
    return center / n_boundary_vertices_;
}

void MinimalAreaSurface::setup_implicit_L()
{
    implicit_L = Eigen::SparseMatrix<double>(n_inner_vertices_, n_inner_vertices_);
    std::vector<Eigen::Triplet<double>> triplets;

    for (auto v : mesh_.vertices())
    {
        if (mesh_.is_boundary(v))
            continue;
        double w = 0, ww = 0;
        for (auto h : mesh_.halfedges(v))
        {
            auto vv = mesh_.to_vertex(h);
            w = eweight_[mesh_.edge(h)];
            ww += eweight_[mesh_.edge(h)];
            if (!mesh_.is_boundary(vv))
                triplets.emplace_back(inner_idx_[v], inner_idx_[vv], w);
        }
        triplets.emplace_back(inner_idx_[v], inner_idx_[v], -ww);
    }
    implicit_L.setFromTriplets(triplets.begin(), triplets.end());
}

void MinimalAreaSurface::implicit_iterate(float lambda)
{
    // solve (I - dt \lambda) * P(n+1) = P(n)
    auto I = Eigen::SparseMatrix<double>(n_inner_vertices_, n_inner_vertices_);
    I.setIdentity();
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(I - lambda * implicit_L);

    Eigen::MatrixX3d P = Eigen::MatrixX3d::Zero(n_inner_vertices_, 3);
    for (auto v : mesh_.vertices())
    {
        if (mesh_.is_boundary(v))
            continue;
        P.row(inner_idx_[v]) = Eigen::Vector3d(mesh_.position(v));
        for (auto h : mesh_.halfedges(v))
        {
            auto vv = mesh_.to_vertex(h);
            double w = eweight_[mesh_.edge(h)];
            if (mesh_.is_boundary(vv))
            {
                P.row(inner_idx_[v]) += lambda * w * Eigen::Vector3d(mesh_.position(vv));
            }
        }
    }

    Eigen::MatrixXd X = solver.solve(P);
    if (solver.info() != Eigen::Success)
    {
        std::cerr << "SurfaceParameterization: Could not solve linear system\n";
    }
    else
        for (auto v : mesh_.vertices())
        {
            if (!mesh_.is_boundary(v))
                mesh_.position(v) = X.row(inner_idx_[v]);
        }
}