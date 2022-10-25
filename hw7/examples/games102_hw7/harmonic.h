//
// Created by pupa on 2020/12/10.
//

#pragma once

#include <pmp/SurfaceMesh.h>
#include <pmp/algorithms/DifferentialGeometry.h>

#include <Eigen/Sparse>

namespace pmp_pupa {

class HarmonicSurface
{
public:
    //! Construct with mesh to be smoothed.
    HarmonicSurface(pmp::SurfaceMesh& mesh) : mesh_(mesh)
    {
        e_cotan = mesh_.add_edge_property<double>("e:cotan");
        v_voronoi = mesh_.add_vertex_property<double>("v:voronoi");
        v_idx = mesh_.add_vertex_property<int>("v:idx", -1);
        if(mesh_.has_vertex_property("v:tex"))
            v_tex = mesh_.get_vertex_property<pmp::TexCoord>("v:tex");
        else
            v_tex = mesh_.add_vertex_property<pmp::TexCoord>("v:tex", pmp::TexCoord(0, 0));
        initialize();
    }

    // destructor
    ~HarmonicSurface() {
        mesh_.remove_edge_property(e_cotan);
        mesh_.remove_vertex_property(v_voronoi);
        mesh_.remove_vertex_property(v_idx);
        mesh_.remove_vertex_property(v_tex);
    }

    void harmonic_tex(bool uniform_weight=false) ;

    void harmonic_3d(bool uniform_weight=false) ;

    void sort_boundary(std::vector<std::tuple<pmp::Vertex, double>>& b_v) ;

    void mapping_3d_boundary_to_circle() ;

    void mapping_3d_boundary_to_rectangle(double ratio = 0.5) ;

    void mapping_tex_boundary_to_circle() ;

    void mapping_tex_boundary_to_rectangle(double ratio = 0.5) ;

private:
    //! Initialize edge and vertex weights.
    void initialize()
    {
        for (auto e : mesh_.edges())
            e_cotan[e] = std::max(0.0, pmp::cotan_weight(mesh_, e));

        for (auto v : mesh_.vertices())
            v_voronoi[v] = pmp::voronoi_area(mesh_, v);
    }

    //! the mesh
    pmp::SurfaceMesh& mesh_;
    pmp::EdgeProperty<double> e_cotan;
    pmp::VertexProperty<double> v_voronoi;
    pmp::VertexProperty<int> v_idx;
    pmp::VertexProperty<pmp::TexCoord> v_tex;
};
} // namespace pmp_pupa