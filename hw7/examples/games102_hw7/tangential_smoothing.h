//
// Created by pupa on 12/9/20.
//

#pragma once


#include <pmp/SurfaceMesh.h>
#include <pmp/algorithms/DifferentialGeometry.h>

#include <Eigen/Sparse>

namespace pmp_pupa {

class SurfaceTangentialSmoothing
{
public:
    SurfaceTangentialSmoothing(pmp::SurfaceMesh& mesh): mesh_(mesh) {
        e_cotan = mesh_.add_edge_property<double>("e:cotan");
        v_voronoi = mesh_.add_vertex_property<double>("v:voronoi");
        initialize();
    }

    ~SurfaceTangentialSmoothing() {
        mesh_.remove_edge_property(e_cotan);
        mesh_.remove_vertex_property(v_voronoi);
    }


    void implicit_smoothing(double alpha = 0.01) {
        size_t n = mesh_.n_vertices();
        Eigen::SparseMatrix<double> Lc(n, n);
        Eigen::SparseMatrix<double> Lu(n, n);
        Eigen::SparseMatrix<double> M(n, n);
        Eigen::MatrixX3d B(n, 3);
        std::vector<Eigen::Triplet<double> > Lc_triplets, M_triplets, Lu_triplets;
        for(auto v: mesh_.vertices()) {
            double w = 0;
            double ww = 0, c = 0;
            for(auto h: mesh_.halfedges(v)) {
                w = e_cotan[mesh_.edge(h)];
                c += 1;
                ww  += w;
                Lc_triplets.push_back({v.idx(), mesh_.to_vertex(h).idx(), w});
                Lu_triplets.push_back({v.idx(), mesh_.to_vertex(h).idx(), 1});
            }
            B.row(v.idx()) = Eigen::Vector3d(mesh_.position(v));
            Lc_triplets.push_back({v.idx(), v.idx(), -ww});
            Lu_triplets.push_back({v.idx(), v.idx(), -c});
            M_triplets.push_back({v.idx(), v.idx(), v_voronoi[v]});
        }

        Lc.setFromTriplets(Lc_triplets.begin(), Lc_triplets.end());
        Lu.setFromTriplets(Lu_triplets.begin(), Lu_triplets.end());
        M.setFromTriplets(M_triplets.begin(), M_triplets.end());

        Eigen::SparseMatrix<double> A = Lu.transpose() * M * Lu + alpha * M;
        Eigen::SimplicialLDLT<Eigen::SparseMatrix<double>> solver(A);
        Eigen::MatrixXd X = solver.solve((Lu.transpose() * M * Lc + alpha * M) * B);
        if (solver.info() != Eigen::Success)
            std::cerr << "SurfaceSmoothing: Could not solve linear system\n";
        else
        {
            // copy solution
            for (unsigned int i = 0; i < n; ++i)
                mesh_.position(pmp::Vertex(i)) = X.row(i);
        }
    }

    //! Initialize edge and vertex weights.
    void initialize()
    {
        for (auto e : mesh_.edges())
            e_cotan[e] = std::max(0.0, pmp::cotan_weight(mesh_, e));

        for (auto v : mesh_.vertices())
            v_voronoi[v] = pmp::voronoi_area(mesh_, v);
    }

private:

    //! the mesh
    pmp::SurfaceMesh& mesh_;
    pmp::EdgeProperty<double> e_cotan;
    pmp::VertexProperty<double> v_voronoi;
};

} // namespace pmp