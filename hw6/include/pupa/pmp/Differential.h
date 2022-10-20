//
// Created by pupa on 12/5/20.
//
#pragma once

#include <pmp/SurfaceMesh.h>
#include <Eigen/Dense>

//! clamp cotangent values as if angles are in [1, 179]
inline double clamp_cot(const double v)
{
    const double bound = 19.1; // 3 degrees
    return (v < -bound ? -bound : (v > bound ? bound : v));
}

namespace pmp_pupa {

double cotan_weight(const SurfaceMesh& mesh, Edge e);

double voronoi_area(const SurfaceMesh& mesh, Vertex v);

Point laplace(const SurfaceMesh& mesh, Vertex v);

//!> angle between prev_h and h
double sector_angle(const SurfaceMesh& mesh, Halfedge h);
//!> normal between prev_h and h
pmp::Normal sector_normal(const SurfaceMesh& mesh, Halfedge h);

//!> angle weighted vertex normal
pmp::Normal normal(const SurfaceMesh& mesh, Vertex v);

//!> Curvature
double mean_curvature(const SurfaceMesh& mesh, Vertex v);
double gaussian_curvature(const SurfaceMesh& mesh, Vertex v);
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>Wrapper<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
Eigen::VectorXd cotan_weight(const SurfaceMesh& mesh);
Eigen::VectorXd voronoi_area(const SurfaceMesh& mesh);

Eigen::MatrixX3d laplace(const SurfaceMesh& mesh);
//<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

} // namespace pmp_pupa
