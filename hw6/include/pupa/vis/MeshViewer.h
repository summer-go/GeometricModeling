//
// Created by pupa on 12/2/20.
//

#pragma once

#include <pmp/SurfaceMesh.h>
#include <pmp/visualization/SurfaceMeshGL.h>
#include <pmp/visualization/TrackballViewer.h>
#include <pmp/algorithms/SurfaceNormals.h>

#include <memory>
#include <imgui.h>

namespace pupa_vis {

struct MeshViewerData;

template <int _cols>
Eigen::Matrix<float, -1, _cols> vertex_property(
    pmp::SurfaceMesh& mesh_, pmp::VertexProperty<pmp::Matrix<float, _cols, 1>> vprob)
{
    Eigen::MatrixXf V(mesh_.n_vertices(), _cols);
    for (auto v : mesh_.vertices())
        V.row(v.idx()) = Eigen::Vector<float, _cols>(vprob[v]);
    return V;
}

class MeshViewer : public pmp::TrackballViewer
{
public:
    //! constructor
    MeshViewer(const char* title, int width, int height, bool showgui = true)
        : TrackballViewer(title, width, height, showgui)
    {
        glfwWindowHint(GLFW_DOUBLEBUFFER, GL_FALSE);
        // setup draw modes
        clear_draw_modes();
        add_draw_mode("Points");
        add_draw_mode("Hidden Line");
        add_draw_mode("Smooth Shading");
        add_draw_mode("Texture");

#ifndef __EMSCRIPTEN__
        add_help_item("W", "Write mesh to 'output.off'", 4);
#endif
    }

    //! destructor
    virtual ~MeshViewer() = default;

    //! load a mesh from file \p filename
    virtual bool load_mesh(const char* filename)
    {
        // load mesh
        if (mesh_.read(filename))
        {
            update_mesh(true);
            view_all();
            // print mesh statistic
            std::cout << "Load " << filename << ": " << mesh_.n_vertices() << " vertices, "
                      << mesh_.n_faces() << " faces\n";
            filename_ = filename;
            return true;
        }

        std::cerr << "Failed to read mesh from " << filename << " !" << std::endl;
        return false;
    }

    void update_mesh(bool update_normal = false)
    {
        pmp::BoundingBox bb = mesh_.bounds();
        center_ = (pmp::vec3)bb.center();
        radius_ = 0.5f * bb.size();

        mesh_.update_opengl_buffers();
    }

    //! draw the scene in different draw modes
    virtual void draw(const std::string& draw_mode) override
    {
        mesh_.draw(projection_matrix_, modelview_matrix_, draw_mode);
    }

    //! handle ImGUI interface
    void process_imgui() override;
    //    {
    //        if (ImGui::CollapsingHeader("Mesh Info", ImGuiTreeNodeFlags_DefaultOpen))
    //        {
    //            // output mesh statistics
    //            ImGui::BulletText("%d vertices", (int)mesh_.n_vertices());
    //            ImGui::BulletText("%d edges", (int)mesh_.n_edges());
    //            ImGui::BulletText("%d faces", (int)mesh_.n_faces());
    //        }
    //    }
    void do_processing() override;

    void curvature_to_texcoord(const pmp::VertexProperty<double>& curvature, double kmax) {
        double kmin = 0;
        auto tex = mesh_.vertex_property<pmp::TexCoord>("v:tex");
        for (auto v : mesh_.vertices())
            tex[v] = pmp::TexCoord((curvature[v] - kmin) / (kmax - kmin), 0.0);
    }

private:


    pmp::SurfaceMeshGL mesh_;
    std::string filename_; //!< the current file

    std::shared_ptr<MeshViewerData> data_;
};

} // namespace pupa_vis
