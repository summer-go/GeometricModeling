//
// Created by pupa on 2020/12/10.
//

#include <pmp/visualization/MeshViewer.h>
#include <pmp/algorithms/SurfaceCurvature.h>
#include <imgui.h>
#include <imgui_internal.h>

#include "harmonic.h"
#include "tangential_smoothing.h"
#include "energy_smoothing.h"

using namespace pmp;

class Viewer : public MeshViewer
{
public:
    Viewer(const char* title, int width, int height);

protected:
    virtual void process_imgui();

};

Viewer::Viewer(const char* title, int width, int height) : MeshViewer(title, width, height) {}

static char* files[]{"armadillo.obj", "Balls.obj", "Bunny_head.obj", "Cat_head.obj", "David328.obj", "Nefertiti_face.obj"};

void Viewer::process_imgui()
{
    MeshViewer::process_imgui();

    ImGui::Spacing();
#if __EMSCRIPTEN__
    static int n = 5;
    ImGui::PushItemWidth(120);

    ImGui::Combo("Files", &n, files, 6);
    ImGui::SameLine();
    if (ImGui::Button("Reload Mesh"))
    {
        load_mesh(files[n]);
        set_draw_mode("Hidden Line");
    }
    ImGui::PopItemWidth();
#endif
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Curvature", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Mean Curvature"))
        {
            SurfaceCurvature analyzer(mesh_);
            analyzer.analyze_tensor(1, true);
            analyzer.mean_curvature_to_texture_coordinates();
            update_mesh();
            mesh_.use_cold_warm_texture();
            set_draw_mode("Texture");
        }
    }

    ImGui::Spacing();
    ImGui::Spacing();

    if (ImGui::CollapsingHeader("Harmonic", ImGuiTreeNodeFlags_DefaultOpen))
    {
        static int boundary_shape = 0;

        ImGui::RadioButton("none", &boundary_shape, 0);
        ImGui::SameLine();
        ImGui::RadioButton("circle", &boundary_shape, 1);
        ImGui::SameLine();
        ImGui::RadioButton("rect(2:1)", &boundary_shape, 2);
        ImGui::SameLine();
        ImGui::RadioButton("square", &boundary_shape, 3);

        if (ImGui::Button("Harmonic Tex"))
        {
            pmp_pupa::HarmonicSurface harmonicSurface(mesh_);
            if (boundary_shape == 1)
                harmonicSurface.mapping_tex_boundary_to_circle();
            else if (boundary_shape == 2)
                harmonicSurface.mapping_tex_boundary_to_rectangle(0.5);
            else if (boundary_shape == 3)
                harmonicSurface.mapping_tex_boundary_to_rectangle(1.0);
            harmonicSurface.harmonic_tex();
            update_mesh();
            view_all();
            mesh_.use_checkerboard_texture();
            set_draw_mode("Texture");
        }
        ImGui::SameLine();
        if (ImGui::Button("Harmonic 3D"))
        {
            pmp_pupa::HarmonicSurface harmonicSurface(mesh_);
            if (boundary_shape == 1)
                harmonicSurface.mapping_3d_boundary_to_circle();
            else if (boundary_shape == 2)
                harmonicSurface.mapping_3d_boundary_to_rectangle(0.5);
            else if (boundary_shape == 3)
                harmonicSurface.mapping_3d_boundary_to_rectangle(1.0);
            harmonicSurface.harmonic_3d();
            update_mesh();
            view_all();
            mesh_.use_checkerboard_texture();
            set_draw_mode("Hidden Line");
            //            set_draw_mode("Hidden Line");
        }


    }



    if (ImGui::CollapsingHeader("optimization", ImGuiTreeNodeFlags_DefaultOpen))
    {
        if (ImGui::Button("Shape Optimization"))
        {
            pmp_pupa::SurfaceTangentialSmoothing optimizer(mesh_);
            optimizer.implicit_smoothing(0.8);
            update_mesh();
        }

        static float alpha = 0.01;
        ImGui::PushItemWidth(100);
        ImGui::SliderFloat("alpha", &alpha, 0.01, 5);
        ImGui::PopItemWidth();

        if (ImGui::Button("Smoothing via Optimization"))
        {
            pmp_pupa::SurfaceEnergySmoothing smoother(mesh_);
            smoother.energy_smoothing(alpha);
            update_mesh();
        }
    }
}

int main(int argc, char** argv)
{
#ifndef __EMSCRIPTEN__
    Viewer window("Game102-HW7", 800, 600);
    if (argc == 2)
        window.load_mesh(argv[1]);
    return window.run();
#else
    Viewer window("Game102-HW7", 800, 600);
    window.load_mesh(argc == 2 ? argv[1] : "Nefertiti_face.obj");
    return window.run();
#endif
}
