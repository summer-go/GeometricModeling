//
// Created by 马西开 on 2020/12/25.
//
#include "SurfaceSimplification.h"
#include <queue>
void Surface_Simplification(MyMesh &mesh, float ratio)
{
    assert(ratio >=0 && ratio <= 1);
    int it_num = (1.0f - ratio) * mesh.n_vertices();

    //1. Compute the Q matrices for all the initial vertices
    auto Q = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Matrix4f>(mesh);
    auto v = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, Eigen::Vector4f>(mesh);
    auto flag = OpenMesh::makeTemporaryProperty<OpenMesh::VertexHandle, int>(mesh); //与vto_flag vfrom_flag配合判断点对是否还有效
    auto p = OpenMesh::makeTemporaryProperty<OpenMesh::FaceHandle, Eigen::Vector4f>(mesh);

    // 求每个面的表达式，遍历面，取出法线作为平面方程系数
    // 平面：ax + by + cz + d = 0, [a, b, c]为该平面的法线， d 为平面从原点(0, 0, 0) 移动的距离
    for(MyMesh::FaceIter fit = mesh.faces_begin(); fit != mesh.faces_end(); ++fit)
    {
        float a, b, c, d;

        // 获取法线，得到参数 a b c
        a = mesh.normal(*fit)[0];
        b = mesh.normal(*fit)[1];
        c = mesh.normal(*fit)[2];

        // 代入平面上任意一点，求得 d
        MyMesh::Point tp = mesh.point(fit->halfedge().to());
        d = - (a * tp[0] + b * tp[1] + c * tp[2]);
        p[*fit][0] = a;
        p[*fit][1] = b;
        p[*fit][2] = c;
        p[*fit][3] = d;
    }

    // 求每个点的Q.遍历点，求出每个点到面的"距离矩阵"
    for(MyMesh::VertexIter vit = mesh.vertices_begin(); vit != mesh.vertices_end(); ++vit)
    {
        Eigen::Matrix4f mat;
        mat.setZero();
        for(MyMesh::VertexFaceIter vf_it = mesh.vf_iter(*vit); vf_it.is_valid(); ++vf_it)
        {
            mat += p[*vf_it] * p[*vf_it].transpose();
        }
        Q[*vit] = mat;
        v[*vit][0] = mesh.point(*vit)[0];
        v[*vit][1] = mesh.point(*vit)[1];
        v[*vit][2] = mesh.point(*vit)[2];
        v[*vit][3] = 1.0f;
        flag[*vit] = 0;
    }

    // 使用优先级队列，实现一个小顶堆，从小到大排序
    // 每次都用新的q
    std::priority_queue <edge_Collapse_structure, std::vector<edge_Collapse_structure>, std::less<edge_Collapse_structure>> q;
    for(MyMesh::EdgeIter eit = mesh.edges_begin(); eit != mesh.edges_end(); ++eit)
    {
        Eigen::Matrix4f newQ = Q[eit->v0()] + Q[eit->v1()];
        Eigen::Matrix4f tQ = newQ;
        Eigen::Vector4f b(0.0f,0.0f,0.0f,1.0f);

        // 对Q求偏导得到的矩阵和Q区别在于：偏导后最后一行为[0, 0, 0, 1]
        tQ(3,0) = 0.0f;
        tQ(3,1) = 0.0f;
        tQ(3,2) = 0.0f;
        tQ(3,3) = 1.0f;
        Eigen::FullPivLU<Eigen::Matrix4f> lu(tQ);
        Eigen::Vector4f vnew;
        // if is invertible, solve the linear equation
        if(lu.isInvertible())
        {
            vnew = tQ.inverse() * b;
        }
        // else take the midpoint
        else
        {
            vnew = (v[eit->v0()] + v[eit->v1()]) / 2.0f;
        }
        //std::cout << vnew << std::endl;
        edge_Collapse_structure ts;
        ts.hf = eit->halfedge(0);
        ts.cost = vnew.transpose() * newQ * vnew;
        MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
        ts.np = np;
        ts.vto = eit->halfedge(0).to();
        ts.vfrom = eit->halfedge(0).from();
        ts.Q_new = newQ;
        //
        q.push(ts);
    }
    mesh.request_vertex_status();
    mesh.request_edge_status();
    mesh.request_face_status();

    int i = 0;

    // 上面都是准备工作，
    // 真正进入到边收缩阶段，迭代it_num次
    while( i < it_num)
    {
        edge_Collapse_structure s = q.top();
        q.pop();
        if(mesh.status(s.vfrom).deleted() || mesh.status(s.vto).deleted())
            continue;
        if(s.vto_flag != flag[s.vto] || s.vfrom_flag != flag[s.vfrom])
            continue;
        MyMesh::VertexHandle tvh;
        if(mesh.is_collapse_ok(s.hf))
        {
            mesh.collapse(s.hf);
            tvh = s.vto;
            flag[s.vto] ++;
            flag[s.vfrom] ++;
        }

        else if(mesh.is_collapse_ok(mesh.opposite_halfedge_handle(s.hf)))
        {
            mesh.collapse(mesh.opposite_halfedge_handle(s.hf));
            tvh = s.vfrom;
            flag[s.vto] ++;
            flag[s.vfrom] ++;
        }
        else
        {
            continue;
        }

        mesh.set_point(tvh,s.np);
        Q[tvh] = s.Q_new;
        v[tvh][0] = s.np[0];
        v[tvh][1] = s.np[1];
        v[tvh][2] = s.np[2];
        v[tvh][3] = 1.0f;
        for(MyMesh::VertexOHalfedgeIter vh_it = mesh.voh_iter(tvh); vh_it.is_valid(); ++vh_it)
        {
            MyMesh::VertexHandle tt = vh_it->to();
            Eigen::Matrix4f newQ = s.Q_new + Q[tt];
            Eigen::Matrix4f tQ = newQ;
            Eigen::Vector4f b(0.0f,0.0f,0.0f,1.0f);
            tQ(3,0) = 0.0f;
            tQ(3,1) = 0.0f;
            tQ(3,2) = 0.0f;
            tQ(3,3) = 1.0f;
            Eigen::FullPivLU<Eigen::Matrix4f> lu(tQ);
            Eigen::Vector4f vnew;
            // if is invertible, solve the linear equation
            if(lu.isInvertible())
            {
                vnew = tQ.inverse() * b;
            }
                // else take the midpoint
            else
            {
                vnew = (v[tvh] + v[tt]) / 2.0f;
            }
            //std::cout << vnew << std::endl;
            edge_Collapse_structure ts;
            ts.hf = *vh_it;
            ts.cost = vnew.transpose() * newQ * vnew;
            MyMesh::Point np(vnew[0], vnew[1], vnew[2]);
            ts.np = np;
            ts.vto = tt;
            ts.vto_flag = flag[tt];
            ts.vfrom = tvh;
            ts.vfrom_flag = flag[tvh];
            ts.Q_new = newQ;
            q.push(ts);
        }
        i++;

    }
    mesh.garbage_collection();
}
