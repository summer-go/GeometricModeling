//
// Created by $Pupa on 2020/12/11.
//
#include "harmonic.h"

using namespace pmp_pupa;

void HarmonicSurface::harmonic_tex(bool uniform_weight) {
    std::vector<pmp::Vertex> inner_v;
    for (auto v : mesh_.vertices())
    {
        if (!mesh_.is_boundary(v))
        {
            v_idx[v] = inner_v.size();
            inner_v.push_back(v);
        }
    }

    Eigen::MatrixX2d B = Eigen::MatrixX2d::Zero(inner_v.size(), 2);
    std::vector<Eigen::Triplet<double>> L_triplets;
    for (auto v : mesh_.vertices())
    {
        if (mesh_.is_boundary(v))
            continue;
        double w = 0;
        double ww = 0;
        for (auto h : mesh_.halfedges(v))
        {
            w = uniform_weight ? 1: e_cotan[mesh_.edge(h)];
            ww += w;
            auto vv = mesh_.to_vertex(h);
            if (v_idx[vv] == -1)
                B.row(v_idx[v]) -= w * Eigen::Vector2d(v_tex[vv]);
            else
                L_triplets.push_back({v_idx[v], v_idx[vv], w});
        }
        L_triplets.push_back({v_idx[v], v_idx[v], -ww});
    }

    Eigen::SparseMatrix<double> L(inner_v.size(), inner_v.size());
    L.setFromTriplets(L_triplets.begin(), L_triplets.end());
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(L);
    Eigen::MatrixXd X = solver.solve(B);
    if (solver.info() != Eigen::Success)
    {
        std::cerr << "SurfaceSmoothing: Could not solve linear system\n";
    }
    else
    {
        // copy solution
        for (auto v : inner_v)
            v_tex[v] = X.row(v_idx[v]);
    }
    for(auto v: mesh_.vertices())
        v_tex[v] = (v_tex[v]/2)+pmp::TexCoord(0.5, 0.5);

    mesh_.remove_vertex_property(v_idx);
}

void HarmonicSurface::harmonic_3d(bool uniform_weight) {
    std::vector<pmp::Vertex> inner_v;
    for (auto v : mesh_.vertices())
    {
        if (!mesh_.is_boundary(v))
        {
            v_idx[v] = inner_v.size();
            inner_v.push_back(v);
        }
    }

    Eigen::MatrixX3d B = Eigen::MatrixX3d::Zero(inner_v.size(), 3);
    std::vector<Eigen::Triplet<double>> L_triplets;
    for (auto v : mesh_.vertices())
    {
        if (mesh_.is_boundary(v))
            continue;
        double w = 0;
        double ww = 0;
        for (auto h : mesh_.halfedges(v))
        {
            w = uniform_weight ? 1: e_cotan[mesh_.edge(h)];
            ww += w;
            auto vv = mesh_.to_vertex(h);
            if (v_idx[vv] == -1)
                B.row(v_idx[v]) -= w * Eigen::Vector3d(mesh_.position(vv));
            else
                L_triplets.push_back({v_idx[v], v_idx[vv], w});
        }
        L_triplets.push_back({v_idx[v], v_idx[v], -ww});
    }

    Eigen::SparseMatrix<double> L(inner_v.size(), inner_v.size());
    L.setFromTriplets(L_triplets.begin(), L_triplets.end());
    Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(L);
    Eigen::MatrixXd X = solver.solve(B);
    if (solver.info() != Eigen::Success)
    {
        std::cerr << "SurfaceSmoothing: Could not solve linear system\n";
    }
    else
    {
        // copy solution
        for (auto v : inner_v)
            mesh_.position(v) = X.row(v_idx[v]);
    }
    for(auto v: mesh_.vertices())
    {
        v_tex[v] = pmp::TexCoord(mesh_.position(v)[0], mesh_.position(v)[1]);
        v_tex[v] = (v_tex[v]/2)+pmp::TexCoord(0.5, 0.5);
    }

    mesh_.remove_vertex_property(v_idx);
}

void HarmonicSurface::sort_boundary(std::vector<std::tuple<pmp::Vertex, double>>& b_v) {
    pmp::Halfedge b_h;
    for(auto h: mesh_.halfedges())
        if(mesh_.is_boundary(h)) {
            b_h = h;
            break;
        }
    double boundary_len = mesh_.edge_length(mesh_.edge(b_h));
    b_v.push_back({mesh_.from_vertex(b_h), boundary_len});
    for(auto h = mesh_.next_halfedge(b_h); h != b_h; h = mesh_.next_halfedge(h)) {
        boundary_len += mesh_.edge_length(mesh_.edge(h));
        b_v.push_back({mesh_.from_vertex(h), boundary_len});
    }
    for(auto& [v, w]: b_v)
        w /= boundary_len;
}

void HarmonicSurface::mapping_3d_boundary_to_circle(){
    std::vector<std::tuple<pmp::Vertex, double>> b_v;
    sort_boundary(b_v);
    for(auto& [v, w]: b_v)
        mesh_.position(v) = pmp::Point(std::cos(w*2*M_PI), std::sin(w*2*M_PI), 0);
}

void HarmonicSurface::mapping_3d_boundary_to_rectangle(double ratio){
    std::vector<std::tuple<pmp::Vertex, double>> b_v;
    sort_boundary(b_v);
    mesh_.position(std::get<0>(b_v[0])) = pmp::Point(1, 0,  0);
    for(size_t i = 1; i < b_v.size(); i++)
    {
        auto& v = std::get<0>(b_v[i]);
        auto& w = std::get<1>(b_v[i]);
        auto& w1 = std::get<1>(b_v[i-1]);
        if(w1 < 1.0/8 && w > 1.0/8)
            mesh_.position(v) = pmp::Point(1, ratio*1,  0);
        else if(w1 < 3.0/8 && w > 3.0/8)
            mesh_.position(v) = pmp::Point(-1, ratio*1,  0);
        else if(w1 < 5.0/8 && w > 5.0/8)
            mesh_.position(v) = pmp::Point(-1, ratio*-1,  0);
        else if(w1 < 7.0/8 && w > 7.0/8)
            mesh_.position(v) = pmp::Point(1, ratio*-1,  0);
        else if(w < 1.0/8 || w > 7.0/8)
            mesh_.position(v) = pmp::Point(1, ratio*std::sin(w*2*M_PI),  0);
        else if (w >= 3.0/8 && w < 5.0/8)
            mesh_.position(v) = pmp::Point(-1, ratio*std::sin(w*2*M_PI),  0);
        else if (w >= 1.0/8 && w < 3.0/8)
            mesh_.position(v) = pmp::Point(std::cos(w*2*M_PI), ratio*1, 0);
        else if (w >= 5.0/8 && w < 7.0/8)
            mesh_.position(v) = pmp::Point(std::cos(w*2*M_PI), -ratio*1, 0);
    }
}

void HarmonicSurface::mapping_tex_boundary_to_circle(){
    std::vector<std::tuple<pmp::Vertex, double>> b_v;
    sort_boundary(b_v);
    for(auto& [v, w]: b_v)
        v_tex[v] = pmp::TexCoord (std::cos(w*2*M_PI), std::sin(w*2*M_PI));
}

void HarmonicSurface::mapping_tex_boundary_to_rectangle(double ratio){
    std::vector<std::tuple<pmp::Vertex, double>> b_v;
    sort_boundary(b_v);
    v_tex[std::get<0>(b_v[0])] = pmp::TexCoord(1, 0);
    for(size_t i = 1; i < b_v.size(); i++)
    {
        auto& v = std::get<0>(b_v[i]);
        auto& w = std::get<1>(b_v[i]);
        auto& w1 = std::get<1>(b_v[i-1]);
        if(w1 < 1.0/8 && w > 1.0/8)
            v_tex[v] = pmp::TexCoord(1, ratio*1);
        else if(w1 < 3.0/8 && w > 3.0/8)
            v_tex[v] = pmp::TexCoord(-1, ratio*1);
        else if(w1 < 5.0/8 && w > 5.0/8)
            v_tex[v] = pmp::TexCoord(-1, ratio*-1);
        else if(w1 < 7.0/8 && w > 7.0/8)
            v_tex[v] = pmp::TexCoord(1, ratio*-1);
        else if(w < 1.0/8 || w > 7.0/8)
            v_tex[v] = pmp::TexCoord(1, ratio*std::sin(w*2*M_PI));
        else if (w >= 3.0/8 && w < 5.0/8)
            v_tex[v] = pmp::TexCoord(-1, ratio*std::sin(w*2*M_PI));
        else if (w >= 1.0/8 && w < 3.0/8)
            v_tex[v] = pmp::TexCoord(std::cos(w*2*M_PI), ratio*1);
        else if (w >= 5.0/8 && w < 7.0/8)
            v_tex[v] = pmp::TexCoord(std::cos(w*2*M_PI), -ratio*1);
    }
}
