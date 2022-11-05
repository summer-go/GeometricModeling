//
// Created by 马西开 on 2020/12/3.
//

#ifndef EXAMPLE_BASE_HPP
#define EXAMPLE_BASE_HPP
#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <vector>
#include <igl/opengl/glfw/Viewer.h>
typedef OpenMesh::TriMesh_ArrayKernelT<> MyMesh;
MyMesh igl_to_openMesh(Eigen::MatrixXd& V, Eigen::MatrixXi &F);
void openMesh_to_igl(MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi &F);
double calc_angle(MyMesh::HalfedgeHandle he, MyMesh &mesh);
float calc_anglef(MyMesh::HalfedgeHandle he, MyMesh &mesh);
float get_Voronoi_cell_area(MyMesh::VertexHandle vh);
Eigen::VectorXd calc_Mean_Curvature(MyMesh &mesh);
Eigen::VectorXd calc_Gaussian_Curvature(MyMesh &mesh);
#endif //EXAMPLE_BASE_HPP
