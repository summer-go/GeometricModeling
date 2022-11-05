//
// Created by 马西开 on 2020/12/3.
//
#include "Base.hpp"
MyMesh igl_to_openMesh(Eigen::MatrixXd& V, Eigen::MatrixXi &F)
{
    MyMesh mesh;
    for(int i = 0; i < V.rows(); ++i)
    {
        mesh.add_vertex(MyMesh::Point(V(i,0),V(i,1),V(i,2)));
    }
    for(int i = 0; i < F.rows(); ++i)
    {

        MyMesh::VertexHandle vhandle[3];
        vhandle[0] = mesh.vertex_handle(F(i,0));
        vhandle[1] = mesh.vertex_handle(F(i,1));
        vhandle[2] = mesh.vertex_handle(F(i,2));
        std::vector<MyMesh::VertexHandle> face_vhandles;
        face_vhandles.clear();
        face_vhandles.push_back(vhandle[0]);
        face_vhandles.push_back(vhandle[1]);
        face_vhandles.push_back(vhandle[2]);
        mesh.add_face(face_vhandles);
    }
    return mesh;

}
void openMesh_to_igl(MyMesh& mesh, Eigen::MatrixXd& V, Eigen::MatrixXi &F)
{
    V.setZero(mesh.n_vertices(),3);
    F.setZero(mesh.n_faces(),3);
    int i = 0;
    for(MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it!= mesh.vertices_end(); ++v_it,++i)
    {
        V(i,0) = mesh.point(*v_it)[0];
        V(i,1) = mesh.point(*v_it)[1];
        V(i,2) = mesh.point(*v_it)[2];
    }
    i = 0;
    for(MyMesh::FaceIter f_it = mesh.faces_begin(); f_it != mesh.faces_end(); ++f_it,++i)
    {
        int j = 0;
        for(MyMesh::FaceVertexCCWIter fv_it = mesh.fv_ccwiter(*f_it); fv_it.is_valid(); ++fv_it,++j)
        {
            F(i,j) = (*fv_it).idx();
        }
    }
}

double calc_angle(MyMesh::HalfedgeHandle he, MyMesh &mesh)
{
    MyMesh::Point p1,p2,p3;
    p1 = mesh.point(mesh.from_vertex_handle(he));
    p2 = mesh.point(mesh.to_vertex_handle(he));
    p3 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he)));
    MyMesh::Point v1 = p1 - p3;
    MyMesh::Point v2 = p2 - p3;
    double a = OpenMesh::norm(v1);
    double b = OpenMesh::norm(p1 - p2);
    double c = OpenMesh::norm(v2);
    //return acosf((v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) / (p1.norm() * p2.norm()));
    return acos((c * c + a * a - b * b) / (2 * c * a));
}
float calc_anglef(MyMesh::HalfedgeHandle he, MyMesh &mesh)
{
    MyMesh::Point p1,p2,p3;
    p1 = mesh.point(mesh.from_vertex_handle(he));
    p2 = mesh.point(mesh.to_vertex_handle(he));
    p3 = mesh.point(mesh.to_vertex_handle(mesh.next_halfedge_handle(he)));
    MyMesh::Point v1 = p1 - p3;
    MyMesh::Point v2 = p2 - p3;
    float a = OpenMesh::norm(v1);
    float b = OpenMesh::norm(p1 - p2);
    float c = OpenMesh::norm(v2);
    //return acosf((v1[0] * v2[0] + v1[1] * v2[1] + v1[2] * v2[2]) / (p1.norm() * p2.norm()));
    return acosf((c * c + a * a - b * b) / (2 * c * a));
}
Eigen::VectorXd calc_Mean_Curvature(MyMesh &mesh)
{
    Eigen::VectorXd ret(mesh.n_vertices());
    for(MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
    {
        float area = 0.0;
        //1-ring
        MyMesh::Point tp(0.0f,0.0f,0.0f);
        for(MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(*v_it); vh_it.is_valid(); ++vh_it)
        {
            float w = 0.0f;
            if(vh_it->is_valid())
            {
                w += 1.0f / tanf(calc_angle(*vh_it,mesh)) ;
            }


            if(vh_it->opp().is_valid())
            {
                w += 1.0f / tanf(calc_angle(vh_it->opp(),mesh));
            }
            if(w < 0)
                w = 0;
            if(w > 20)
                w = 20;
            tp += w *(mesh.point(*v_it) - mesh.point(vh_it->to()));
            area += w * OpenMesh::sqrnorm(mesh.point(*v_it) - mesh.point(vh_it->to()));
        }
        area /= 8.0f;
        tp /= 2.0 * area;
        float temp = OpenMesh::norm(tp) / 2.0f;
        ret[v_it->idx()] = temp;

    }
    return ret;
}
float get_Voronoi_cell_area(MyMesh::VertexHandle vh, MyMesh &mesh)
{
    float area = 0.0;
    for(MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(vh); vh_it.is_valid(); ++vh_it)
    {
        float w = 0.0f;
        if(vh_it->is_valid())
        {
            w += 1.0f / tan(calc_angle(*vh_it,mesh)) ;
        }


        if(vh_it->opp().is_valid())
        {
            w += 1.0f / tan(calc_angle(vh_it->opp(),mesh));
        }
        if(w < 0)
            w = 0;
        if(w > 20)
            w = 20;
        area += w * OpenMesh::sqrnorm(mesh.point(vh) - mesh.point(vh_it->to()));
    }
    area /= 8.0f;
    return area;
}
Eigen::VectorXd calc_Gaussian_Curvature(MyMesh &mesh)
{
    Eigen::VectorXd ret(mesh.n_vertices());
    for(MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
    {
        float area = get_Voronoi_cell_area(*v_it, mesh);
        float sum_angle = 0.0;
        for(MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(*v_it); vh_it.is_valid(); ++vh_it)
        {
            sum_angle += calc_anglef(vh_it->next(), mesh);
        }
        ret[v_it->idx()] = (2 * M_PI - sum_angle) / area;
    }
    return ret;
}
