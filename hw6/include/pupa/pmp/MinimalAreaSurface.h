//
// Created by pupa on 12/1/20.
//

#pragma once

#include <pmp/SurfaceMesh.h>
#include <Eigen/Sparse>

namespace pmp_pupa {

using namespace pmp;

class MinimalAreaSurface
{
public:
    //! Construct with mesh to be processed.
    MinimalAreaSurface(SurfaceMesh& mesh);

    // destructor
    ~MinimalAreaSurface();

    void explicit_iterate(float lambda, bool boundary);

    void implicit_iterate(float lambda);

private:

    void boundary_explicit_iterate(float lambda);

    double boundary_std();

    pmp::Point boundary_center();

    void setup_implicit_L();

    SurfaceMesh& mesh_; //!< the mesh
    size_t n_inner_vertices_{0};
    size_t n_boundary_vertices_{0};

    // property handles
    EdgeProperty<double>        eweight_;
    VertexProperty<pmp::Point>  vlaplace_;

    VertexProperty<int> inner_idx_;
    VertexProperty<int> boundary_idx_;

    Eigen::SparseMatrix<double> implicit_L;
};

} // namespace pmp_pupa