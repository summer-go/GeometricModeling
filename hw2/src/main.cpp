#include <iostream>
#include <vector>
#include <algorithm>
#include <future>

#include "glad/glad.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "imgui_internal.h"
#include "GLFW/glfw3.h"

#include "RbfNetwork.h"
#include "MathUtil.h"

GLFWwindow *g_window = nullptr;
int g_width = 1600;
int g_height = 1200;
RbfNetwork g_rbf_nn;

ImVec2 g_canvas_pos_ul = { 0.0f, 0.0f };
ImVec2 g_canvas_pos_br = { 0.0f, 0.0f };
void PlotLineSegments(const std::vector<Eigen::Vector2f> &poss, ImDrawList *draw_list, ImU32 line_col, ImU32 point_col) {
    for (size_t i = 1; i < poss.size(); i++) {
        draw_list->AddLine({ g_canvas_pos_ul.x + poss[i - 1].x(), g_canvas_pos_br.y - poss[i - 1].y() },
            { g_canvas_pos_ul.x + poss[i].x(), g_canvas_pos_br.y - poss[i].y() }, line_col, 2.0f);
    }
    for (const auto &pos : poss) {
        draw_list->AddCircleFilled({ g_canvas_pos_ul.x + pos.x(), g_canvas_pos_br.y - pos.y() }, 5.0f, point_col);
        draw_list->AddCircle({ g_canvas_pos_ul.x + pos.x(), g_canvas_pos_br.y - pos.y() }, 5.0f, point_col);
    }
}

std::vector<Eigen::Vector2f>
TrainRbfNetwork(const std::vector<Eigen::Vector2f> &in_pos, int num_middle, int epochs, float lb, float rb,
    float step, bool &training) {
    auto res = g_rbf_nn.Train(in_pos, num_middle, epochs, lb, rb, step);
    training = false;
    return res;
}

bool Initialize(char*argv[]) {
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    g_window = glfwCreateWindow(g_width, g_height, "GAMES102 hw2", nullptr, nullptr);
    if (g_window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return false;
    }

    glfwMakeContextCurrent(g_window);
    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to load glad" << std::endl;
        glfwTerminate();
        return false;
    }

    glViewport(0, 0, g_width, g_height);
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) <<  std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) <<  std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) <<  std::endl;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    (void) io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(g_window, true);
    ImGui_ImplOpenGL3_Init("#version 330 core");

    Py_Initialize();
//    PySys_SetArgv(0, (wchar_t **)argv);
    PyRun_SimpleString("print('Hello Python!')"); // 网上给的一般是2.7的代码 print没有括号
    if (!g_rbf_nn.Initialize()) {
        std::cout << "Failed to initialize RBF Network" << std::endl;
        return false;
    }

    return true;
}

void BeginFrame() {
    glfwPollEvents();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
}

void EndFrame() {
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    glfwSwapBuffers(g_window);
}

void Finalize() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glfwDestroyWindow(g_window);
    glfwTerminate();

    g_rbf_nn.Finalize();
    if (Py_FinalizeEx() < 0) {
        std::cout << "Failed when Py_FinalizeEx()" << std::endl;
    }
}

int main(int argc, char *argv[]) {
    if (!Initialize(argv)) {
        return -1;
    }

    std::vector<Eigen::Vector2f> in_pos;
    struct {
        std::vector<Eigen::Vector2f> pos;
        int m = 0;
        int m_temp = 0;
        float sigma2 = 10000.0f;
        float sigma2_temp = 10000.0f;
        bool visible = false;
        bool update = false;
    } inter_gauss;
    struct {
        std::vector<Eigen::Vector2f> pos;
        int m = 0;
        int m_temp = 0;
        bool visible = false;
        bool update = false;
    } approx_poly;
    struct {
        std::vector<Eigen::Vector2f> pos;
        int num_middle = 0;
        int num_middle_temp = 0;
        int epochs = 2000;
        int epochs_temp = 2000;
        bool visible = false;
        bool update = false;
        bool training = false;
        bool finished = true;
        std::future<std::vector<Eigen::Vector2f>> future;
    } rbf_nn;

    ImGuiIO &io = ImGui::GetIO();
    ImFontConfig font_config;
    font_config.SizePixels = 24.0f;
    io.Fonts->AddFontDefault(&font_config);

    while (!glfwWindowShouldClose(g_window)) {
        BeginFrame();
        if (glfwGetKey(g_window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            break;
        }

        if (ImGui::Begin("Config")) {
            ImGui::Checkbox("Interpolation - Gauss (Green)", &inter_gauss.visible);
            ImGui::SameLine();
            ImGui::PushItemWidth(500.0f);
            ImGui::InputInt("m##1", &inter_gauss.m_temp);
            inter_gauss.m_temp = std::clamp(inter_gauss.m_temp, 0, std::max<int>(0, in_pos.size() - 1));
            if (inter_gauss.m_temp != inter_gauss.m) {
                inter_gauss.m = inter_gauss.m_temp;
                inter_gauss.update = true;
            }
            ImGui::SameLine();
            ImGui::InputFloat("sigma2", &inter_gauss.sigma2_temp);
            inter_gauss.sigma2_temp = std::max(inter_gauss.sigma2_temp, 1.0f);
            if (inter_gauss.sigma2_temp != inter_gauss.sigma2) {
                inter_gauss.sigma2 = inter_gauss.sigma2_temp;
                inter_gauss.update = true;
            }

            ImGui::Checkbox("Fitting - Polygon (Blue)", &approx_poly.visible);
            ImGui::SameLine();
            ImGui::InputInt("m##2", &approx_poly.m_temp);
            approx_poly.m_temp = std::clamp(approx_poly.m_temp, 0, std::max<int>(0, in_pos.size() - 1));
            if (approx_poly.m_temp != approx_poly.m) {
                approx_poly.m = approx_poly.m_temp;
                approx_poly.update = true;
            }

            ImGui::Checkbox("RBF Network - Gauss (Red)", &rbf_nn.visible);
            ImGui::SameLine();
            rbf_nn.update = ImGui::Button("train");
            if (rbf_nn.training) {
                rbf_nn.update = false;
            }
            ImGui::SameLine();
            ImGui::LabelText("##1", "%s", rbf_nn.training ? "(training)" : "");
            ImGui::LabelText("##2", "# middle: %d", rbf_nn.num_middle);
            ImGui::SameLine();
            ImGui::InputInt("##3", &rbf_nn.num_middle_temp);
            rbf_nn.num_middle_temp = std::max(1, rbf_nn.num_middle_temp);
            ImGui::LabelText("##4", "# epochs: %d", rbf_nn.epochs);
            ImGui::SameLine();
            ImGui::InputInt("##5", &rbf_nn.epochs_temp);
            rbf_nn.epochs_temp = std::max(1, rbf_nn.epochs_temp);

            ImGui::PopItemWidth();
            ImGui::End();
        }

        if (ImGui::Begin("Canvas")) {
            g_canvas_pos_ul = ImGui::GetCursorScreenPos();
            ImVec2 canvas_size = ImGui::GetContentRegionAvail();
            if (canvas_size.x < 50.0f) {
                canvas_size.x = 50.0f;
            }
            if (canvas_size.y < 50.0f) {
                canvas_size.y = 50.0f;
            }
            g_canvas_pos_br = ImVec2(g_canvas_pos_ul.x + canvas_size.x, g_canvas_pos_ul.y + canvas_size.y);

            ImGuiIO &io = ImGui::GetIO();
            ImDrawList *draw_list = ImGui::GetWindowDrawList();
            draw_list->AddRectFilled(g_canvas_pos_ul, g_canvas_pos_br, IM_COL32(50, 50, 50, 255));
            draw_list->AddRect(g_canvas_pos_ul, g_canvas_pos_br, IM_COL32(255, 255, 255, 255));

            float step = 20.0f;
            float lb = step;
            float rb = g_canvas_pos_br.x - step - g_canvas_pos_ul.x;
            ImGui::InvisibleButton("canvas", canvas_size);
            const bool is_hovered = ImGui::IsItemHovered();
            if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                in_pos.emplace_back(io.MousePos.x - g_canvas_pos_ul.x, g_canvas_pos_br.y - io.MousePos.y);
                std::sort(in_pos.begin(), in_pos.end(), [](const Eigen::Vector2f &a, const Eigen::Vector2f &b) {
                    return a.x() < b.x();
                });

                inter_gauss.update = true;
                approx_poly.update = true;
            } else if (is_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
                Eigen::Vector2f pos(io.MousePos.x - g_canvas_pos_ul.x, g_canvas_pos_br.y - io.MousePos.y);
                size_t index = 0;
                float min_dist = std::numeric_limits<float>::max();
                for (size_t i = 0; i < in_pos.size(); i++) {
                    float dist = (pos - in_pos[i]).squaredNorm();
                    if (dist < min_dist) {
                        min_dist = dist;
                        index = i;
                    }
                }
                if (min_dist <= 100.0f) {
                    in_pos.erase(in_pos.begin() + index);
                    inter_gauss.update = true;
                    approx_poly.update = true;
                }
            }
            if (inter_gauss.update) {
                inter_gauss.pos = MathUtil::InterpolationGauss(in_pos, inter_gauss.sigma2, inter_gauss.m, lb, rb, step);
                inter_gauss.update = false;
            }
            if (approx_poly.update) {
                approx_poly.pos = MathUtil::ApproximationPolygon(in_pos, approx_poly.m, lb, rb, step);
                approx_poly.update = false;
            }
            if (rbf_nn.update) {
                rbf_nn.num_middle = rbf_nn.num_middle_temp;
                rbf_nn.epochs = rbf_nn.epochs_temp;
                // rbf_nn.pos = g_rbf_nn.Train(in_pos, rbf_nn.num_middle, rbf_nn.epochs, lb, rb, step);
                rbf_nn.update = false;
                rbf_nn.training = true;
                rbf_nn.finished = false;
                rbf_nn.future = std::async(std::launch::async, TrainRbfNetwork, in_pos, rbf_nn.num_middle,
                    rbf_nn.epochs, lb, rb, step, std::ref(rbf_nn.training));
            }
            if (!rbf_nn.training && !rbf_nn.finished) {
                rbf_nn.pos = rbf_nn.future.get();
                rbf_nn.finished = true;
            }

            if (inter_gauss.visible) {
                PlotLineSegments(inter_gauss.pos, draw_list, IM_COL32(50, 255, 50, 255), IM_COL32(80, 255, 80, 255));
            }
            if (approx_poly.visible) {
                PlotLineSegments(approx_poly.pos, draw_list, IM_COL32(50, 50, 255, 255), IM_COL32(80, 80, 255, 255));
            }
            if (rbf_nn.visible) {
                PlotLineSegments(rbf_nn.pos, draw_list, IM_COL32(255, 50, 50, 255), IM_COL32(255, 80, 80,255));
            }
            PlotLineSegments(in_pos, draw_list, IM_COL32(255, 255, 255, 0), IM_COL32(255, 255, 255, 255));

            ImGui::End();
        }

        EndFrame();
    }

    Finalize();
    return 0;
}