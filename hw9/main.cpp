#include <OpenMesh/Core/IO/MeshIO.hh>
#include <igl/opengl/glfw/Viewer.h>
#include "Base.hpp"
#include "SurfaceSimplification.h"

const int cache_size = 12;
Eigen::MatrixXd V[cache_size];
Eigen::MatrixXi F[cache_size];
bool flag[cache_size] = {false};
MyMesh mesh;
int ti = 1;
bool key_down(igl::opengl::glfw::Viewer& viewer, unsigned char key, int modifier)
{
    // 按1 简化模型
    if(key == '1')
    {
        if(ti < cache_size)
        {
            if(!flag[ti])
            {
                // 生成一个简化的模型，0.5表示简化操作保留一半顶点
                Surface_Simplification(mesh,0.5);
                std::cout << "vertices : " << mesh.n_vertices() << std::endl;;
                std::cout << "faces : " << mesh.n_faces() << std::endl;;
                // 转换成igl接受的点面数据
                openMesh_to_igl(mesh,V[ti],F[ti]);
                flag[ti] = true;
            }
            // clear view里的数据
            viewer.data().clear();
            // 更新模型数据
            viewer.data().set_mesh(V[ti],F[ti]);
            // 模型居中
            viewer.core().align_camera_center(V[ti],F[ti]);
            // ti 记录当前简化的次数
            ti++;
        }

    }

    // 按2，还原模型，从缓存里读取对应的模型数据
    else if(key == '2')
    {
        if(ti > 1)
        {
            ti--;
            viewer.data().clear();
            viewer.data().set_mesh(V[ti],F[ti]);
            viewer.core().align_camera_center(V[ti],F[ti]);
        }
    }
    return false;
}



int main(int argc, char *argv[])
{

    OpenMesh::VPropHandleT< double > test;
    mesh.add_property(test);
    mesh.request_vertex_normals();
    mesh.request_face_normals();
    OpenMesh::IO::Options opt;

    // 读取模型数据
    if(!OpenMesh::IO::read_mesh(mesh,"../models/armadillo.obj",opt))
    {
        std::cout << "can't open file"<<std::endl;
    }
    for(MyMesh::VertexIter v_it = mesh.vertices_begin(); v_it != mesh.vertices_end(); ++v_it)
    {
        mesh.point(*v_it) /= 4.0;
    }
    mesh.update_face_normals();
    mesh.update_vertex_normals();
    igl::opengl::glfw::Viewer viewer;

    // 注册按键回调，用来触发简化、还原操作
    viewer.callback_key_down = &key_down;

    // 转成igl需要的数据，点和面用矩阵来存储
    // V F数组长度为12，即缓存了12组简化结果，方便回溯查看
    openMesh_to_igl(mesh,V[0],F[0]);

    std::cout << "vertices : " <<  mesh.n_vertices() << std::endl;
    std::cout << "faces : " << mesh.n_faces() << std::endl;

    // 模型数据设置到igl的viewer中
    viewer.data().set_mesh(V[0], F[0]);
    // 默认展示线框
    viewer.data().show_lines = true;

    // 打开window
    viewer.launch();

}
