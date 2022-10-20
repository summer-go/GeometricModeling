//
// Created by pupa on 12/1/20.
//

#pragma once

#include <pmp/SurfaceMesh.h>
#include <Eigen/Sparse>

namespace pmp_pupa {

using namespace pmp;

class SurfaceCurvature
{
public:
    //! Construct with mesh to be processed.
    SurfaceCurvature(SurfaceMesh& mesh) : mesh_(mesh)
    {
        mean_curvature_ = mesh_.add_vertex_property<double>("fairing:mean_curv");
        gauss_curvature_ = mesh_.add_vertex_property<double>("fairing:gauss_curv");
    }

    // destructor
    ~SurfaceCurvature()
    {
        mesh_.remove_vertex_property(mean_curvature_);
        mesh_.remove_vertex_property(gauss_curvature_);
    }

    VertexProperty<double>& update_gauss_curvature();
    VertexProperty<double>& update_mean_curvature();

    double max_gauss_curvature();
    double max_mean_curvature();
private:
    SurfaceMesh& mesh_; //!< the mesh

    VertexProperty<double> mean_curvature_;
    VertexProperty<double> gauss_curvature_;

    double gauss_kmax_{-1};
    double mean_kmax_{-1};
};

} // namespace pmp_pupa