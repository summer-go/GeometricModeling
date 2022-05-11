#include "myapp.h"

static void glfw_error_callback(int error, const char* description) {
    std::cerr << "Glfw error"+std::to_string(error)+
                 ": "+std::string(description) << std::endl;
}
int g_width = 1200;
int g_height = 1200;
void MyApp::initAll() {
    //glfwSetErrorCallback(glfw_error_callback);

    glfwInit();

    const char* glsl_version = "#version 330 core";
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);  // 3.2+ only
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);            // 3.0+ only
    //glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); 

    window = glfwCreateWindow(g_width, g_height, "Hw4 by 天哥", NULL, NULL);

    if (window == nullptr) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return ;
    }

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc) glfwGetProcAddress)) {
        std::cout << "Failed to load glad" << std::endl;
        glfwTerminate();
        return;
    }
//    glfwSwapInterval(1);

//    gladLoadGL();

    glViewport(0, 0, g_width, g_height);
    std::cout << "GL_VERSION: " << glGetString(GL_VERSION) <<  std::endl;
    std::cout << "GL_VENDOR: " << glGetString(GL_VENDOR) <<  std::endl;
    std::cout << "GL_RENDERER: " << glGetString(GL_RENDERER) <<  std::endl;

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImPlot::CreateContext();
    ImGuiIO &io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init(glsl_version);
}

void MyApp::cleanUp() {
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImPlot::DestroyContext();
    ImGui::DestroyContext();

    glfwDestroyWindow(window);
    glfwTerminate();
}

void MyApp::mainLoop() {
    //预定变量
    ControlPointArray2D arr;

    //control para
    bool changeTan = false;
    int moveNodeNum = -1;

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        {            
            ImGui::SetNextWindowSize(ImVec2(1000,1000), ImGuiCond_Appearing);
            ImGui::Begin("main");
            bool cal = false;
            if (ImPlot::BeginPlot("", "", "", ImVec2(900, 900))) {
                // changeTan == true时，打开切线编辑，进入编辑模式，此时只能拖动点，不能插入点
                if (changeTan) {
                    // 暂无已经选择的控制点
                    if (moveNodeNum==-1 and ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(0)) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        // 获取最近的点作为选择点
                        moveNodeNum = arr.findSuitableCtrlPoint(pt.x, pt.y);
                    }
                    // 已经选择了控制点，再点一次，则取消选择点
                    else if (moveNodeNum!=-1 and ImPlot::IsPlotHovered() and ImGui::IsMouseClicked(0)) {
                        moveNodeNum = -1;
                    }

                    // 如果已经有选择的点，鼠标正在移动，则将该点进行挪动
                    // moveNodeNum 分三个范围段，[0, arr.nodenum],  [arr.nodenum,  2 * arr.nodenum], [2 * arr.nodenum,  3 * arr.nodenum]，通过这种方式区分，没有特殊的含义
                    if (moveNodeNum != -1) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        if (moveNodeNum < arr.nodenum())
                            // 如果选择的点小于 arr.nodenum，则表示选择的是插入的控制点，则更新点的坐标
                            arr.setPoint(moveNodeNum, pt.x, pt.y);
                        else if (moveNodeNum < 2 * arr.nodenum())
                            // 如果选择的点小于 2 * arr.nodenum，则表示选择的是切线的左控制点，更新左切线
                            arr.setLDiff(moveNodeNum - arr.nodenum(), pt.x, pt.y);
                        else
                            // 表示选择的是切线的右控制点，更新切线
                            arr.setRDiff(moveNodeNum - 2 * arr.nodenum(), pt.x, pt.y);
                    }
                }
                else {
                    // 如果不是编辑模式，只能插入点，不能拖动编辑
                    if (ImPlot::IsPlotHovered() && ImGui::IsMouseClicked(0)) {
                        ImPlotPoint pt = ImPlot::GetPlotMousePos();
                        // 插入一个点
                        arr.push_back(pt.x, pt.y);
                    }
                }

                {
                    // 获取已经插值好的曲线点
                    NodeArr& draw_arr = arr.getDrawPoints();
                    // 获取插入的离散的点
                    std::vector<double> ctrlxs = arr.getControlPointx();
                    std::vector<double> ctrlys = arr.getControlPointy();
                    // 获取切线点(每组三个点)
                    auto ctrlbar = arr.getControlBar();

                    // 绘制插值曲线
                    ImPlot::PlotLine("A-chazhi", draw_arr.xs.data(), draw_arr.ys.data(), draw_arr.size, 0, sizeof(double));

                    // 设置离散的插入点的风格为方形，size为-1，表示很小吧
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, -01.0f);
                    // 绘制离散的控制点
                    ImPlot::PlotScatter("B-kongzhidian", ctrlxs.data(), ctrlys.data(), ctrlxs.size(), 0, sizeof(double));
                    ImPlot::SetNextMarkerStyle(ImPlotMarker_Square, -1.0f);
                    if (changeTan) {
                        for (int i = 0; i < ctrlbar.size(); ++i) {
                            ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, -1.0f);
                            // 绘制切线(控制编辑线)
                            ImPlot::PlotLine("C-qulvkongzhi", &ctrlbar[i][0], &ctrlbar[i][1], 3, 0, 2 * sizeof(double));
                        }
                    }
                    //ImPlot::PlotLine("InterPolation_Gaussian", &arr2.xs[0], &arr2.ys[0], arr2.size, 0, sizeof(double));
                    //ImPlot::PlotLine("Least_Square", &arr3.xs[0], &arr3.ys[0], arr3.size, 0, sizeof(double));
                    //ImPlot::PlotLine("Ridge_Regression", &arr4.xs[0], &arr4.ys[0], arr4.size, 0, sizeof(double));
                }
                
                ImPlot::EndPlot();
            }
            // 增加按钮开关，是否打开编辑
            ImGui::Checkbox("edit curve", &changeTan);
            ImGui::SameLine();
            // 增加按钮开关，清除数据
            if (ImGui::Button("clear")) {
                arr.clear();
            }
            ImGui::End();
        }

        ImGui::Render();
        int _width, _height;
        glfwGetFramebufferSize(window, &_width, &_height);
        glViewport(0, 0, _width, _height);
        glClearColor(0.45f, 0.55f, 0.60f, 1.00f);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);
    }
}