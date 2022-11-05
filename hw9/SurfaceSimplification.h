//
// Created by 马西开 on 2020/12/25.
//

#ifndef HW_SURFACESIMPLIFICATION_H
#define HW_SURFACESIMPLIFICATION_H

#include "Base.hpp"
#include <OpenMesh/Core/Mesh/Traits.hh>
#include <OpenMesh/Core/Utils/PropertyManager.hh>

struct edge_Collapse_structure
{
    MyMesh::HalfedgeHandle hf;
    MyMesh::Point np;
    MyMesh::VertexHandle vto;
    MyMesh::VertexHandle vfrom;
    //下面两个用来判断该点对是否已被更新过的点对取代
    int vto_flag = 0;
    int vfrom_flag = 0;
    Eigen::Matrix4f Q_new;
    float cost;
    bool operator<(const edge_Collapse_structure& a) const
    {
        return cost > a.cost;
    }
};

// t is a threshold parameter
void Surface_Simplification(MyMesh &mesh, float ratio);
#endif //HW_SURFACESIMPLIFICATION_H
