#include "Render.hpp"
#include "LocalPlayer.hpp"
#include "Player.hpp"
#include "Camera.hpp"
#include "Spectator.hpp"
#include "Aimbot.hpp"
#include "Glow.hpp"
#include "Config.hpp"
#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx11.h"
#include <d3d11.h>
#include <tchar.h>
#include <utility>
#include "Font.h"
#include "DMALibrary/Memory/Memory.h"

//20以上 u8 需要c/c++ 命令行加/Zc:char8_t-

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static UINT                     g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;

// Forward declarations of helper functions
bool CreateDeviceD3D(HWND hWnd);
void CleanupDeviceD3D();
void CreateRenderTarget();
void CleanupRenderTarget();
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

void Helpmarker(const char* Text, ImVec4 Color)
{
    ImGui::TextColored(Color, "(?)");
    if (ImGui::IsItemHovered())
    {
        ImGui::SetTooltip(Text);
    }
}

// Define the display names and corresponding values separately
    const char* keyNamesENG[] = {
        "MouseLeft",
        "MouseRight",
        "MouseMiddle",
        "MouseX1",
        "MouseX2",
        "OFF",
    };
    const char* keyNames[] = {
        u8"鼠标左键",
        u8"鼠标右键",
        u8"鼠标中键",
        u8"鼠标下侧键",
        u8"鼠标上侧键",
        u8"关闭",
    };
int keyValues[] = {
    0x01, // Left Mouse Button
    0x02, // Right Mouse Button
    0x04, // Middle Mouse Button
    0x05, // Side1 Mouse Button
    0x06, // Side2 Mouse Button
    0x00, // OFF
};
constexpr int keyCount = sizeof(keyNames) / sizeof(keyNames[0]);
constexpr int keyCountENG = sizeof(keyNamesENG) / sizeof(keyNamesENG[0]);

const char* glowNames[] = {
    u8"白色以上",
    u8"蓝色以上",
    u8"紫色以上",
    u8"金色以上",
};
const char* glowNamesENG[] = {
    u8"WHITE",
    u8"BLUE",
    u8"PURPLE",
    u8"GOLD",
};
int glowValues[] = {
    34, // White
    35, // Blue
    36, // Purple
    37, // Gold
};
constexpr int glowCount = sizeof(glowNames) / sizeof(glowNames[0]);
constexpr int glowCountENG = sizeof(glowNamesENG) / sizeof(glowNamesENG[0]);

const char* items[] = { "BPro", "Net" };

int FindCurrentSelectionIndex(const int value, const int* keyValues, const int keyCount) {
    for (int i = 0; i < keyCount; ++i) {
        if (value == keyValues[i]) {
            return i;
        }
    }
    return 0; // Default to the first item if not found
}

int FindCurrentSelectionIndexENG(const int value, const int* keyValues, const int keyCountENG) {
    for (int i = 0; i < keyCount; ++i) {
        if (value == keyValues[i]) {
            return i;
        }
    }
    return 0; // Default to the first item if not found
}

bool english = false;
// Main code
void Render(LocalPlayer* Myself, std::vector<Player*>* Players, Camera* GameCamera, Spectator* Spectators, Aimbot* AimAssist, Sense* ESP)
{
    // Create application window
    WNDCLASSEX wc = { sizeof(WNDCLASSEX), CS_CLASSDC, WndProc, 0L, 0L, GetModuleHandle(NULL), NULL, NULL, NULL, NULL, _T("LesousAPEX_DMA"), NULL };
    ::RegisterClassEx(&wc);
    HWND hwnd = ::CreateWindow(wc.lpszClassName, _T("LesousAPEX_DMA"), WS_OVERLAPPEDWINDOW, 100, 100, 1, 1, NULL, NULL, wc.hInstance, NULL);


    // Initialize Direct3D
    if (!CreateDeviceD3D(hwnd))
    {
        CleanupDeviceD3D();
        ::UnregisterClass(wc.lpszClassName, wc.hInstance);
        return;
    }

    // Show the window
    ::ShowWindow(hwnd, SW_HIDE);
    ::UpdateWindow(hwnd);

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;


    ImGui::StyleColorsDark();


    ImGui_ImplWin32_Init(hwnd);
    ImGui_ImplDX11_Init(g_pd3dDevice, g_pd3dDeviceContext);

    io.IniFilename = nullptr;
    ImFontConfig Font_cfg;
    Font_cfg.FontDataOwnedByAtlas = false;

    //ImFont* Font = io.Fonts->AddFontFromFileTTF("..\\ImGui Tool\\Font.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesChineseFull());
    ImFont* Font = io.Fonts->AddFontFromMemoryTTF((void*)Font_data, Font_size, 14.0f, &Font_cfg, io.Fonts->GetGlyphRangesChineseFull());
    ImFont* Font_Big = io.Fonts->AddFontFromMemoryTTF((void*)Font_data, Font_size, 26.0f, &Font_cfg, io.Fonts->GetGlyphRangesChineseFull());

    ImVec4 clear_color = ImVec4(0.45f, 0.55f, 0.60f, 1.00f);

    auto lastFrameTime = std::chrono::high_resolution_clock::now(); // 初始化lastFrameTime
    float RGBtime = 0.0f; // 初始化RGBtime

    // Main loop
    bool done = false;
    while (!done)
    {
        auto currentFrameTime = std::chrono::high_resolution_clock::now();
        float deltaTime = std::chrono::duration<float, std::chrono::seconds::period>(currentFrameTime - lastFrameTime).count();
        lastFrameTime = currentFrameTime;
        RGBtime += deltaTime; // 更新RGBtime
        // Poll and handle messages (inputs, window resize, etc.)
        // See the WndProc() function below for our to dispatch events to the Win32 backend.
        MSG msg;
        while (::PeekMessage(&msg, NULL, 0U, 0U, PM_REMOVE))
        {
            ::TranslateMessage(&msg);
            ::DispatchMessage(&msg);
            if (msg.message == WM_QUIT)
                done = true;
        }
        if (done)
            break;


        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();

        {
            float r = (std::sin(RGBtime) + 1.0f) * 0.5f; // 在0到1之间变化
            float g = (std::sin(RGBtime + 2.09439f) + 1.0f) * 0.5f; // 120度相位偏移
            float b = (std::sin(RGBtime + 4.18879f) + 1.0f) * 0.5f; // 240度相位偏移

            ImVec4 RGB = ImVec4(r, g, b, 1.0f);

            ImGuiStyle& Style = ImGui::GetStyle();
            auto Color = Style.Colors;

            static bool WinPos = true;//用于初始化窗口位置
            int Screen_Width{ GetSystemMetrics(SM_CXSCREEN) };//获取显示器的宽
            int Screen_Heigth{ GetSystemMetrics(SM_CYSCREEN) };//获取显示器的高

            static int InputInt = 0;
            static int Comb = 0;
            static float InputFloat = 0;


            static int Tab = 0;
            enum Tab
            {
                GLOW,
                AIM,
                KM,
                MISC
            };

            enum ColorEnum
            {
                Red,
                Green,
                Blue,
                Orange,
                RGB_,
                Color_Count // 用于计数枚举中的元素数量
            };
            static int Color_ = 0;
            int currentColorIndex = 0; // 存储当前选中的颜色索引
            currentColorIndex = static_cast<int>(Color_);

            // 颜色名称数组，与枚举中的顺序相对应
            const char* colorNames[Color_Count] = { u8"RED", u8"GREEN", u8"BLUE", u8"ORANGE", u8"RGB" };

            switch (Color_)
            {
            case ColorEnum::Red:
                Style.ChildRounding = 8.0f;
                Style.FrameRounding = 5.0f;

                Color[ImGuiCol_Button] = ImColor(192, 51, 74, 255);
                Color[ImGuiCol_ButtonHovered] = ImColor(212, 71, 94, 255);
                Color[ImGuiCol_ButtonActive] = ImColor(172, 31, 54, 255);

                Color[ImGuiCol_FrameBg] = ImColor(54, 54, 54, 150);
                Color[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42, 150);
                Color[ImGuiCol_FrameBgHovered] = ImColor(100, 100, 100, 150);

                Color[ImGuiCol_CheckMark] = ImColor(192, 51, 74, 255);

                Color[ImGuiCol_SliderGrab] = ImColor(192, 51, 74, 255);
                Color[ImGuiCol_SliderGrabActive] = ImColor(172, 31, 54, 255);

                Color[ImGuiCol_Header] = ImColor(192, 51, 74, 255);
                Color[ImGuiCol_HeaderHovered] = ImColor(212, 71, 94, 255);
                Color[ImGuiCol_HeaderActive] = ImColor(172, 31, 54, 255);
                break;
            case ColorEnum::Green:
                Style.ChildRounding = 8.0f;
                Style.FrameRounding = 5.0f;

                Color[ImGuiCol_Button] = ImColor(10, 105, 56, 255);
                Color[ImGuiCol_ButtonHovered] = ImColor(30, 125, 76, 255);
                Color[ImGuiCol_ButtonActive] = ImColor(0, 95, 46, 255);

                Color[ImGuiCol_FrameBg] = ImColor(54, 54, 54, 150);
                Color[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42, 150);
                Color[ImGuiCol_FrameBgHovered] = ImColor(100, 100, 100, 150);

                Color[ImGuiCol_CheckMark] = ImColor(10, 105, 56, 255);

                Color[ImGuiCol_SliderGrab] = ImColor(10, 105, 56, 255);
                Color[ImGuiCol_SliderGrabActive] = ImColor(0, 95, 46, 255);

                Color[ImGuiCol_Header] = ImColor(10, 105, 56, 255);
                Color[ImGuiCol_HeaderHovered] = ImColor(30, 125, 76, 255);
                Color[ImGuiCol_HeaderActive] = ImColor(0, 95, 46, 255);

                break;
            case ColorEnum::Blue:
                Style.ChildRounding = 8.0f;
                Style.FrameRounding = 5.0f;

                Color[ImGuiCol_Button] = ImColor(51, 120, 255, 255);
                Color[ImGuiCol_ButtonHovered] = ImColor(71, 140, 255, 255);
                Color[ImGuiCol_ButtonActive] = ImColor(31, 100, 225, 255);

                Color[ImGuiCol_FrameBg] = ImColor(54, 54, 54, 150);
                Color[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42, 150);
                Color[ImGuiCol_FrameBgHovered] = ImColor(100, 100, 100, 150);

                Color[ImGuiCol_CheckMark] = ImColor(51, 120, 255, 255);

                Color[ImGuiCol_SliderGrab] = ImColor(51, 120, 255, 255);
                Color[ImGuiCol_SliderGrabActive] = ImColor(31, 100, 225, 255);

                Color[ImGuiCol_Header] = ImColor(51, 120, 255, 255);
                Color[ImGuiCol_HeaderHovered] = ImColor(71, 140, 255, 255);
                Color[ImGuiCol_HeaderActive] = ImColor(31, 100, 225, 255);

                break;
            case ColorEnum::Orange://233,87,33
                Style.ChildRounding = 8.0f;
                Style.FrameRounding = 5.0f;

                Color[ImGuiCol_Button] = ImColor(233, 87, 33, 255);
                Color[ImGuiCol_ButtonHovered] = ImColor(253, 107, 53, 255);
                Color[ImGuiCol_ButtonActive] = ImColor(213, 67, 13, 255);

                Color[ImGuiCol_FrameBg] = ImColor(54, 54, 54, 150);
                Color[ImGuiCol_FrameBgActive] = ImColor(42, 42, 42, 150);
                Color[ImGuiCol_FrameBgHovered] = ImColor(100, 100, 100, 150);

                Color[ImGuiCol_CheckMark] = ImColor(233, 87, 33, 255);

                Color[ImGuiCol_SliderGrab] = ImColor(233, 87, 33, 255);
                Color[ImGuiCol_SliderGrabActive] = ImColor(213, 67, 13, 255);

                Color[ImGuiCol_Header] = ImColor(233, 87, 33, 255);
                Color[ImGuiCol_HeaderHovered] = ImColor(253, 107, 53, 255);
                Color[ImGuiCol_HeaderActive] = ImColor(213, 67, 13, 255);

                break;
            case ColorEnum::RGB_:
                Style.ChildRounding = 8.0f;
                Style.FrameRounding = 5.0f;

                // 这里我们使用一个时间变量来创建一个循环变化的颜色效果
                float time = ImGui::GetTime();
                float cos_time = cosf(time * 0.5f);

                int r = (int)(cos_time * 127.5f + 127.5f);
                int g = (int)(sinf(time * 0.5f) * 127.5f + 127.5f);
                int b = (int)(-cos_time * 127.5f + 127.5f);

                ImVec4 rgb_color = ImColor(r, g, b, 255);

                Color[ImGuiCol_Button] = rgb_color;
                Color[ImGuiCol_ButtonHovered] = rgb_color;
                Color[ImGuiCol_ButtonActive] = rgb_color;
                Color[ImGuiCol_FrameBg] = rgb_color;
                Color[ImGuiCol_FrameBgActive] = rgb_color;
                Color[ImGuiCol_FrameBgHovered] = rgb_color;
                Color[ImGuiCol_CheckMark] = rgb_color;
                Color[ImGuiCol_SliderGrab] = rgb_color;
                Color[ImGuiCol_SliderGrabActive] = rgb_color;
                Color[ImGuiCol_Header] = rgb_color;
                Color[ImGuiCol_HeaderHovered] = rgb_color;
                Color[ImGuiCol_HeaderActive] = rgb_color;

                // 注意：这里的颜色设置只是一个示例，你可以根据需要调整颜色和动画效果
                break;
            }


            if (WinPos)//初始化窗口
            {
                ImGui::SetNextWindowPos({ float(Screen_Width - 600) / 2,float(Screen_Heigth - 400) / 2 });
                WinPos = false;//初始化完毕
            }

            ImGui::Begin("LesousAPEX_DMA", NULL, ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings);//开始绘制窗口
            ImGui::SetWindowSize({ 650.0f,400.0f });//设置窗口大小
            if (english)
            {
                ImGui::GetWindowDrawList()->AddLine({ ImGui::GetWindowPos().x + 420.0f,ImGui::GetWindowPos().y + 10.0f }, { ImGui::GetWindowPos().x + 420.0f,ImGui::GetWindowPos().y + 390.0f }, ImColor(100, 100, 100, 255));

                ImGui::SetCursorPos({ 430.0f,20.0f });
                ImGui::PushFont(Font_Big);
                ImGui::TextColored(RGB, u8"   FUN Plus＋ \u9B08");
                ImGui::PopFont();

                ImGui::SetCursorPos({ 430.0f,65.0f });

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::GLOW ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Button(u8"ESP", { 210.0f,40.0f }))
                {
                    Tab = Tab::GLOW;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::AIM ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,115.0f });
                if (ImGui::Button(u8"AIM", { 210.0f,40.0f }))
                {
                    Tab = Tab::AIM;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::KM ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,165.0f });
                if (ImGui::Button(u8"KmBox", { 210.0f,40.0f }))
                {
                    Tab = Tab::KM;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::MISC ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,215.0f });
                if (ImGui::Button(u8"MISC", { 210.0f,40.0f }))
                {
                    Tab = Tab::MISC;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,265.0f });
                if (ImGui::Button(u8"EXIT", { 210.0f,40.0f }))
                {
                    exit(0);
                }
                ImGui::PopStyleColor();

                ImGui::SetCursorPos({ 430.0f,330.0f });
                if (ImGui::Button(u8"切换中文"))
                {
                    english = false;
                }
                ImGui::SameLine();
                ImGui::SetCursorPos({ 505.0f,328.0f });
                ImGui::SetNextItemWidth(130.0f);
                if (ImGui::Combo("", &currentColorIndex, colorNames, Color_Count))
                {
                    // 当选择变更时，currentColorIndex会被更新
                    // 现在我们需要根据当前选择的颜色索引设置颜色
                    Color_ = static_cast<ColorEnum>(currentColorIndex); // 更新颜色枚举值
                    // 根据当前选择的颜色索引设置颜色
                    switch (Color_)
                    {
                    case ColorEnum::Red:
                        // 设置红色的样式
                        break;
                    case ColorEnum::Green:
                        // 设置绿色的样式
                        break;
                    case ColorEnum::Blue:
                        // 设置蓝色的样式
                        break;
                    case ColorEnum::Orange:
                        // 设置橘色的样式
                        break;
                    case ColorEnum::RGB_:
                        // 设置RGB循环变化的样式
                        break;
                    }
                }

                time_t t = time(0);
                char tmp[32] = { NULL };
                strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&t));

                ImGui::SetCursorPos({ 430.0f,365.0f });
                ImGui::TextColored(Color[ImGuiCol_Button], "%s", tmp);
                ImGui::SameLine();
                if (Spectators->TotalSpectators > 0) {
                    ImGui::Text(u8"spector:%d", Spectators->TotalSpectators);
                    for (const auto& name : Spectators->Spectators) {
                        ImGui::Text(u8"%s", name.c_str());
                    }
                }
                else {
                    ImGui::Text(u8"No Spector");
                }
            }
            else
            {
                ImGui::GetWindowDrawList()->AddLine({ ImGui::GetWindowPos().x + 420.0f,ImGui::GetWindowPos().y + 10.0f }, { ImGui::GetWindowPos().x + 420.0f,ImGui::GetWindowPos().y + 390.0f }, ImColor(100, 100, 100, 255));

                ImGui::SetCursorPos({ 430.0f,20.0f });
                ImGui::PushFont(Font_Big);
                ImGui::TextColored(RGB , u8"   FUN Plus＋ \u9B08");
                ImGui::PopFont();

                ImGui::SetCursorPos({ 430.0f,65.0f });

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::GLOW ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                if (ImGui::Button(u8"透视选项", { 210.0f,40.0f }))
                {
                    Tab = Tab::GLOW;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::AIM ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,115.0f });
                if (ImGui::Button(u8"自瞄选项", { 210.0f,40.0f }))
                {
                    Tab = Tab::AIM;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::KM ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,165.0f });
                if (ImGui::Button(u8"KmBox", { 210.0f,40.0f }))
                {
                    Tab = Tab::KM;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, Tab == Tab::MISC ? Color[ImGuiCol_Button] : ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,215.0f });
                if (ImGui::Button(u8"身法及其它", { 210.0f,40.0f }))
                {
                    Tab = Tab::MISC;
                }
                ImGui::PopStyleColor();

                ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.0f, 0.0f, 0.0f, 0.0f));
                ImGui::SetCursorPos({ 430.0f,265.0f });
                if (ImGui::Button(u8"退出程序", { 210.0f,40.0f }))
                {
                    exit(0);
                }
                ImGui::PopStyleColor();

                ImGui::SetCursorPos({ 430.0f,330.0f });
                if (ImGui::Button(u8"English"))
                {
                    english = true;
                }
                ImGui::SameLine();
                ImGui::SetCursorPos({ 505.0f,328.0f });
                ImGui::SetNextItemWidth(130.0f);
                if (ImGui::Combo("", &currentColorIndex, colorNames, Color_Count))
                {
                    // 当选择变更时，currentColorIndex会被更新
                    // 现在我们需要根据当前选择的颜色索引设置颜色
                    Color_ = static_cast<ColorEnum>(currentColorIndex); // 更新颜色枚举值
                    // 根据当前选择的颜色索引设置颜色
                    switch (Color_)
                    {
                    case ColorEnum::Red:
                        // 设置红色的样式
                        break;
                    case ColorEnum::Green:
                        // 设置绿色的样式
                        break;
                    case ColorEnum::Blue:
                        // 设置蓝色的样式
                        break;
                    case ColorEnum::Orange:
                        // 设置橘色的样式
                        break;
                    case ColorEnum::RGB_:
                        // 设置RGB循环变化的样式
                        break;
                    }
                }

                time_t t = time(0);
                char tmp[32] = { NULL };
                strftime(tmp, sizeof(tmp), "%Y-%m-%d %H:%M", localtime(&t));

                ImGui::SetCursorPos({ 430.0f,365.0f });
                ImGui::TextColored(Color[ImGuiCol_Button], "%s", tmp);
                ImGui::SameLine();
                if (Spectators->TotalSpectators > 0) {
                    ImGui::Text(u8"观战人数:%d", Spectators->TotalSpectators);
                    for (const auto& name : Spectators->Spectators) {
                        ImGui::Text(u8"%s", name.c_str());
                    }
                }
                else {
                    ImGui::Text(u8"观战人数:0");
                }
            }

            if (english)
            {
                ImGui::SetCursorPos({ 10.0f,10.0f });
                ImGui::BeginChild("FUN Plus+", { 400.0f,380.0f }, true);
                switch (Tab)
                {
                case Tab::AIM:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"Sticky");
                    ImGui::PopFont();
                    ImGui::SameLine();
                    if (ImGui::Checkbox(u8"##Sticky:", &AimAssist->Sticky)) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"FOV:");
                    if (ImGui::SliderFloat(u8"##FOV:", &AimAssist->FOV, 1.0f, 50.0f, "% .1f")) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Text(u8"Smooth:");
                    if (ImGui::SliderFloat(u8"##Smooth:", &AimAssist->Smooth, 1.0f, 10.0f, "% .1f")) {
                        Config::GetInstance().Save();
                    }
                    // 假设AimAssist->MaxSmoothIncrease的值在0.0到1.0之间
                    float displayValue = AimAssist->MaxSmoothIncrease * 100.0f; // 将内部数值转换为百分比显示值
                    ImGui::Text(u8"MaxSmoothIncrease:");
                    if (ImGui::SliderFloat(u8"##MaxSmoothIncrease:", &displayValue, 0.0f, 100.0f, "%.0f%%")) {
                        AimAssist->MaxSmoothIncrease = displayValue / 100.0f; // 将显示值转换回内部数值
                        Config::GetInstance().Save();
                    }
                    ImGui::Text(u8"RecoilCompensation:");
                    if (ImGui::SliderFloat(u8"##RecoilCompensation:", &AimAssist->RecoilCompensation, 1.0f, 5.0f, "% .2f")) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    // For AimBotKey
                    int aimBotKeyIndex = FindCurrentSelectionIndexENG(AimAssist->AimBotKey, keyValues, keyCount);
                    ImGui::Text(u8"AIMKey1:");
                    if (ImGui::Combo(u8"##AIMKey1", &aimBotKeyIndex, keyNamesENG, keyCount)) {
                        AimAssist->AimBotKey = keyValues[aimBotKeyIndex];
                        Config::GetInstance().Save();
                    }
                    int aimBotKeyIndex2 = FindCurrentSelectionIndexENG(AimAssist->AimBotKey2, keyValues, keyCount);
                    ImGui::Text(u8"AIMKey2:");
                    if (ImGui::Combo(u8"##AIMKey2", &aimBotKeyIndex2, keyNamesENG, keyCount)) {
                        AimAssist->AimBotKey2 = keyValues[aimBotKeyIndex2];
                        Config::GetInstance().Save();
                    }
                    // For AimTriggerKey
                    int aimTriggerKeyIndex = FindCurrentSelectionIndexENG(AimAssist->AimTriggerKey, keyValues, keyCount);
                    ImGui::Text(u8"TriggerKey:");
                    if (ImGui::Combo(u8"##TriggerKey", &aimTriggerKeyIndex, keyNamesENG, keyCount)) {
                        AimAssist->AimTriggerKey = keyValues[aimTriggerKeyIndex];
                        Config::GetInstance().Save();
                    }
                    // For AimFlickKey
                    int aimFlickKeyIndex = FindCurrentSelectionIndexENG(AimAssist->AimFlickKey, keyValues, keyCount);
                    ImGui::Text(u8"FlickKey:");
                    if (ImGui::Combo(u8"##FlickKey", &aimFlickKeyIndex, keyNamesENG, keyCount)) {
                        AimAssist->AimFlickKey = keyValues[aimFlickKeyIndex];
                        Config::GetInstance().Save();
                    }

                    break;
                }
                case Tab::KM:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"Kmbox");
                    ImGui::PopFont();
                    ImGui::Separator();
                    ImGui::Text(u8"KmBoxType:");
                    const char* items[] = { "BPro", "Net" };

                    // 当前选项的索引，这个将根据你选择的选项变化
                    int currentItem = (AimAssist->KmboxType == "BPro") ? 0 : 1;

                    // Combo 下拉选择框
                    if (ImGui::Combo("Kmbox Type", &currentItem, items, IM_ARRAYSIZE(items)))
                    {
                        // 当选择变更时更新 KmboxType 的值
                        AimAssist->KmboxType = items[currentItem];
                        Config::GetInstance().Save();
                    }
                    if (AimAssist->KmboxType == u8"BPro")
                    {
                        ImGui::Text(u8"KmBoxComPort:");
                        ImGui::SameLine();
                        Helpmarker(u8"Change to the baud rate of the device under the Config file", Color[ImGuiCol_Button]);
                        ImGui::InputInt("", &AimAssist->KmboxComPort);
                        Config::GetInstance().Save();
                    }
                    else if (AimAssist->KmboxType == u8"Net")
                    {
                        ImGui::InputText(u8"Kmbox IP", AimAssist->KmboxIP, IM_ARRAYSIZE(AimAssist->KmboxIP));
                        ImGui::InputText(u8"Kmbox Port", AimAssist->KmboxPort, IM_ARRAYSIZE(AimAssist->KmboxPort));
                        ImGui::InputText(u8"Kmbox UUID", AimAssist->KmboxUUID, IM_ARRAYSIZE(AimAssist->KmboxUUID));
                        Config::GetInstance().Save();
                    }
                    // 示范：打印当前选中的 KmboxType
                    // ImGui 按钮
                    if (ImGui::Button(u8"Initialize Kmbox")) {
                        AimAssist->Initialize(); // 手动执行初始化
                    }
                    if (AimAssist->KmboxInitialized) {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), u8"KmBox Initialize Success"); // 绿色显示
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"KmBox Initialize Failed"); // 绿色显示
                    }
                    break;
                }
                case Tab::MISC:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"MISC");
                    ImGui::PopFont();
                    ImGui::Separator();
                    if (ImGui::Button(u8"BigMapRADAR")) { // 如果按下了这个按钮
                        Myself->BigMapRADAR();             // 调用ESP的BigMapRADAR方法
                    }
                    ImGui::SameLine();
                    Helpmarker(u8"Use at your own risk", Color[ImGuiCol_Button]);
                    break;
                }
                case Tab::GLOW:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"Glow");
                    ImGui::PopFont();
                    ImGui::Separator();
                    float ShowColor[3] = {
                    ESP->rnot / 255.0f,
                    ESP->gnot / 255.0f,
                    ESP->bnot / 255.0f
                    };
                    ImGui::Text(u8"Glow Type:");
                    // 创建一个滑动块来控制填充风格
                    if (ImGui::SliderInt(u8"insidetype", &ESP->insidetype, 0, 255)) {
                        Config::GetInstance().Save();
                    }

                    // 创建一个滑动块来控制轮廓风格
                    if (ImGui::SliderInt(u8"outlinetype", &ESP->outlinetype, 0, 255)) {
                        Config::GetInstance().Save();
                    }

                    // 创建一个滑动块来控制轮廓大小
                    if (ImGui::SliderInt(u8"outlinesize", &ESP->outlinesize, 0, 255)) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"Glow Color:");
                    if (ImGui::ColorEdit3(u8"Color Inviz", ShowColor)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->rnot = static_cast<int>(ShowColor[0] * 255.0f);
                        ESP->gnot = static_cast<int>(ShowColor[1] * 255.0f);
                        ESP->bnot = static_cast<int>(ShowColor[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    float ShowColorViz[3] = {
                    ESP->rviz / 255.0f,
                    ESP->gviz / 255.0f,
                    ESP->bviz / 255.0f
                    };
                    if (ImGui::ColorEdit3(u8"Color Viz", ShowColorViz)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->rviz = static_cast<int>(ShowColorViz[0] * 255.0f);
                        ESP->gviz = static_cast<int>(ShowColorViz[1] * 255.0f);
                        ESP->bviz = static_cast<int>(ShowColorViz[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    float AdmColor[3] = {
                    ESP->radm / 255.0f,
                    ESP->gadm / 255.0f,
                    ESP->badm / 255.0f
                    };
                    if (ImGui::ColorEdit3(u8"Dev Color", AdmColor)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->radm = static_cast<int>(AdmColor[0] * 255.0f);
                        ESP->gadm = static_cast<int>(AdmColor[1] * 255.0f);
                        ESP->badm = static_cast<int>(AdmColor[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"ItemGlow:");
                    ImGui::SameLine();
                    if (ImGui::Checkbox(u8"##ItemGlow:", &ESP->ItemGlow)) {
                        Config::GetInstance().Save();
                    }
                    int itemGlowIndex = FindCurrentSelectionIndex(ESP->MinimumItemRarity, glowValues, glowCount);
                    ImGui::Text(u8"MinimumItemRarity:");
                    if (ImGui::Combo(u8"##MinimumItemRarity", &itemGlowIndex, glowNamesENG, glowCount)) {
                        ESP->MinimumItemRarity = glowValues[itemGlowIndex];
                        Config::GetInstance().Save();
                    }
                    //ImGui::Text(u8"武器上色:");
                    //ImGui::SameLine();
                    //ImGui::Checkbox(u8"##武器上色", &ESP->ViewModelGlow);

                    ImGui::Separator();

                    break;
                }
                }
            }
            else
            {
                ImGui::SetCursorPos({ 10.0f,10.0f });
                ImGui::BeginChild("FUN Plus+", { 400.0f,380.0f }, true);
                switch (Tab)
                {
                case Tab::AIM:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"磁性自瞄");
                    ImGui::PopFont();
                    ImGui::SameLine();
                    Helpmarker(u8"QQ1974909658，交流群521089932", Color[ImGuiCol_Button]);
                    ImGui::SameLine();
                    if (ImGui::Checkbox(u8"##磁性自瞄:", &AimAssist->Sticky)) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"自瞄范围:");
                    if (ImGui::SliderFloat(u8"##自瞄范围:", &AimAssist->FOV, 1.0f, 50.0f, "% .1f")) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Text(u8"平滑:");
                    if (ImGui::SliderFloat(u8"##平滑:", &AimAssist->Smooth, 1.0f, 10.0f, "% .1f")) {
                        Config::GetInstance().Save();
                    }
                    // 假设AimAssist->MaxSmoothIncrease的值在0.0到1.0之间
                    float displayValue = AimAssist->MaxSmoothIncrease * 100.0f; // 将内部数值转换为百分比显示值
                    ImGui::Text(u8"最大平滑增加百分比:");
                    if (ImGui::SliderFloat(u8"##最大平滑增加百分比:", &displayValue, 0.0f, 100.0f, "%.0f%%")) {
                        AimAssist->MaxSmoothIncrease = displayValue / 100.0f; // 将显示值转换回内部数值
                        Config::GetInstance().Save();
                    }
                    ImGui::Text(u8"后坐力控制:");
                    if (ImGui::SliderFloat(u8"##后坐力控制:", &AimAssist->RecoilCompensation, 1.0f, 5.0f, "% .2f")) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    // For AimBotKey
                    int aimBotKeyIndex = FindCurrentSelectionIndex(AimAssist->AimBotKey, keyValues, keyCount);
                    ImGui::Text(u8"自瞄按键1:");
                    if (ImGui::Combo(u8"##自瞄按键1", &aimBotKeyIndex, keyNames, keyCount)) {
                        AimAssist->AimBotKey = keyValues[aimBotKeyIndex];
                        Config::GetInstance().Save();
                    }
                    int aimBotKeyIndex2 = FindCurrentSelectionIndex(AimAssist->AimBotKey2, keyValues, keyCount);
                    ImGui::Text(u8"自瞄按键2:");
                    if (ImGui::Combo(u8"##自瞄按键2", &aimBotKeyIndex2, keyNames, keyCount)) {
                        AimAssist->AimBotKey2 = keyValues[aimBotKeyIndex2];
                        Config::GetInstance().Save();
                    }
                    // For AimTriggerKey
                    int aimTriggerKeyIndex = FindCurrentSelectionIndex(AimAssist->AimTriggerKey, keyValues, keyCount);
                    ImGui::Text(u8"扳机按键:");
                    if (ImGui::Combo(u8"##扳机按键", &aimTriggerKeyIndex, keyNames, keyCount)) {
                        AimAssist->AimTriggerKey = keyValues[aimTriggerKeyIndex];
                        Config::GetInstance().Save();
                    }
                    // For AimFlickKey
                    int aimFlickKeyIndex = FindCurrentSelectionIndex(AimAssist->AimFlickKey, keyValues, keyCount);
                    ImGui::Text(u8"甩枪按键:");
                    if (ImGui::Combo(u8"##甩枪按键", &aimFlickKeyIndex, keyNames, keyCount)) {
                        AimAssist->AimFlickKey = keyValues[aimFlickKeyIndex];
                        Config::GetInstance().Save();
                    }

                    break;
                }
                case Tab::KM:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"Kmbox");
                    ImGui::PopFont();
                    ImGui::SameLine();
                    Helpmarker(u8"QQ1974909658，交流群521089932", Color[ImGuiCol_Button]);
                    ImGui::Separator();
                    ImGui::Text(u8"KmBox类型:");
                    const char* items[] = { "BPro", "Net" };

                    // 当前选项的索引，这个将根据你选择的选项变化
                    int currentItem = (AimAssist->KmboxType == "BPro") ? 0 : 1;

                    // Combo 下拉选择框
                    if (ImGui::Combo("Kmbox Type", &currentItem, items, IM_ARRAYSIZE(items)))
                    {
                        // 当选择变更时更新 KmboxType 的值
                        AimAssist->KmboxType = items[currentItem];
                        Config::GetInstance().Save();
                    }
                    if (AimAssist->KmboxType == u8"BPro")
                    {
                        ImGui::Text(u8"KmBoxCom端口:");
                        ImGui::SameLine();
                        Helpmarker(u8"波特率请在Config文件下更改为设备的波特率", Color[ImGuiCol_Button]);
                        ImGui::InputInt("", &AimAssist->KmboxComPort);
                        Config::GetInstance().Save();
                    }
                    else if (AimAssist->KmboxType == u8"Net")
                    {
                        ImGui::InputText(u8"Kmbox IP", AimAssist->KmboxIP, IM_ARRAYSIZE(AimAssist->KmboxIP));
                        ImGui::InputText(u8"Kmbox Port", AimAssist->KmboxPort, IM_ARRAYSIZE(AimAssist->KmboxPort));
                        ImGui::InputText(u8"Kmbox UUID", AimAssist->KmboxUUID, IM_ARRAYSIZE(AimAssist->KmboxUUID));
                        Config::GetInstance().Save();
                    }
                    // 示范：打印当前选中的 KmboxType
                    // ImGui 按钮
                    if (ImGui::Button(u8"初始化Kmbox")) {
                        AimAssist->Initialize(); // 手动执行初始化
                    }
                    if (AimAssist->KmboxInitialized) {
                        ImGui::TextColored(ImVec4(0, 1, 0, 1), u8"KmBox初始化成功"); // 绿色显示
                    }
                    else
                    {
                        ImGui::TextColored(ImVec4(1, 0, 0, 1), u8"KmBox初始化失败"); // 绿色显示
                    }
                    break;
                }
                case Tab::MISC:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"身法和其它");
                    ImGui::PopFont();
                    ImGui::Separator();
                    if (ImGui::Button(u8"开启大地图雷达")) { // 如果按下了这个按钮
                        Myself->BigMapRADAR();             // 调用ESP的BigMapRADAR方法
                    }
                    break;
                }
                case Tab::GLOW:
                {
                    ImGui::PushFont(Font_Big);
                    ImGui::BulletText(u8"发光");
                    ImGui::PopFont();
                    ImGui::Separator();
                    float ShowColor[3] = {
                    ESP->rnot / 255.0f,
                    ESP->gnot / 255.0f,
                    ESP->bnot / 255.0f
                    };
                    ImGui::Text(u8"发光风格:");
                    // 创建一个滑动块来控制填充风格
                    if (ImGui::SliderInt(u8"填充风格", &ESP->insidetype, 0, 255)) {
                        Config::GetInstance().Save();
                    }

                    // 创建一个滑动块来控制轮廓风格
                    if (ImGui::SliderInt(u8"轮廓风格", &ESP->outlinetype, 0, 255)) {
                        Config::GetInstance().Save();
                    }

                    // 创建一个滑动块来控制轮廓大小
                    if (ImGui::SliderInt(u8"轮廓大小", &ESP->outlinesize, 0, 255)) {
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"发光颜色设置:");
                    if (ImGui::ColorEdit3(u8"墙后颜色", ShowColor)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->rnot = static_cast<int>(ShowColor[0] * 255.0f);
                        ESP->gnot = static_cast<int>(ShowColor[1] * 255.0f);
                        ESP->bnot = static_cast<int>(ShowColor[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    float ShowColorViz[3] = {
                    ESP->rviz / 255.0f,
                    ESP->gviz / 255.0f,
                    ESP->bviz / 255.0f
                    };
                    if (ImGui::ColorEdit3(u8"墙外颜色", ShowColorViz)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->rviz = static_cast<int>(ShowColorViz[0] * 255.0f);
                        ESP->gviz = static_cast<int>(ShowColorViz[1] * 255.0f);
                        ESP->bviz = static_cast<int>(ShowColorViz[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    float AdmColor[3] = {
                    ESP->radm / 255.0f,
                    ESP->gadm / 255.0f,
                    ESP->badm / 255.0f
                    };
                    if (ImGui::ColorEdit3(u8"权限颜色", AdmColor)) {
                        // 用户更改了颜色，更新整数颜色分量值
                        ESP->radm = static_cast<int>(AdmColor[0] * 255.0f);
                        ESP->gadm = static_cast<int>(AdmColor[1] * 255.0f);
                        ESP->badm = static_cast<int>(AdmColor[2] * 255.0f);
                        // 标记颜色已被更改，并保存配置
                        Config::GetInstance().Save();
                    }
                    ImGui::Separator();
                    ImGui::Text(u8"物品透视:");
                    ImGui::SameLine();
                    if (ImGui::Checkbox(u8"##物品透视:", &ESP->ItemGlow)) {
                        Config::GetInstance().Save();
                    }
                    int itemGlowIndex = FindCurrentSelectionIndex(ESP->MinimumItemRarity, glowValues, glowCount);
                    ImGui::Text(u8"物品透视等级:");
                    if (ImGui::Combo(u8"##物品透视等级", &itemGlowIndex, glowNames, glowCount)) {
                        ESP->MinimumItemRarity = glowValues[itemGlowIndex];
                        Config::GetInstance().Save();
                    }
                    //ImGui::Text(u8"武器上色:");
                    //ImGui::SameLine();
                    //ImGui::Checkbox(u8"##武器上色", &ESP->ViewModelGlow);

                    ImGui::Separator();

                    break;
                }
                }
            }
            ImGui::EndChild();


            ImGui::End();
        }
        // Rendering
        ImGui::Render();
        const float clear_color_with_alpha[4] = { clear_color.x * clear_color.w, clear_color.y * clear_color.w, clear_color.z * clear_color.w, clear_color.w };
        g_pd3dDeviceContext->OMSetRenderTargets(1, &g_mainRenderTargetView, NULL);
        g_pd3dDeviceContext->ClearRenderTargetView(g_mainRenderTargetView, clear_color_with_alpha);
        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());


        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ImGui::UpdatePlatformWindows();
            ImGui::RenderPlatformWindowsDefault();
        }

        g_pSwapChain->Present(1, 0);
    }
    ImGui_ImplDX11_Shutdown();
    ImGui_ImplWin32_Shutdown();
    ImGui::DestroyContext();

    CleanupDeviceD3D();
    ::DestroyWindow(hwnd);
    ::UnregisterClass(wc.lpszClassName, wc.hInstance);

    return;
}

// Helper functions

bool CreateDeviceD3D(HWND hWnd)
{
    // Setup swap chain
    DXGI_SWAP_CHAIN_DESC sd;
    ZeroMemory(&sd, sizeof(sd));
    sd.BufferCount = 2;
    sd.BufferDesc.Width = 0;
    sd.BufferDesc.Height = 0;
    sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    sd.BufferDesc.RefreshRate.Numerator = 60;
    sd.BufferDesc.RefreshRate.Denominator = 1;
    sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
    sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    sd.OutputWindow = hWnd;
    sd.SampleDesc.Count = 1;
    sd.SampleDesc.Quality = 0;
    sd.Windowed = TRUE;
    sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT createDeviceFlags = 0;
    //createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
    D3D_FEATURE_LEVEL featureLevel;
    const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
    HRESULT res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res == DXGI_ERROR_UNSUPPORTED) // Try high-performance WARP software driver if hardware is not available.
        res = D3D11CreateDeviceAndSwapChain(nullptr, D3D_DRIVER_TYPE_WARP, nullptr, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &g_pSwapChain, &g_pd3dDevice, &featureLevel, &g_pd3dDeviceContext);
    if (res != S_OK)
        return false;

    CreateRenderTarget();
    return true;
}

void CleanupDeviceD3D()
{
    CleanupRenderTarget();
    if (g_pSwapChain) { g_pSwapChain->Release(); g_pSwapChain = nullptr; }
    if (g_pd3dDeviceContext) { g_pd3dDeviceContext->Release(); g_pd3dDeviceContext = nullptr; }
    if (g_pd3dDevice) { g_pd3dDevice->Release(); g_pd3dDevice = nullptr; }
}

void CreateRenderTarget()
{
    ID3D11Texture2D* pBackBuffer;
    g_pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
    g_pd3dDevice->CreateRenderTargetView(pBackBuffer, nullptr, &g_mainRenderTargetView);
    pBackBuffer->Release();
}

void CleanupRenderTarget()
{
    if (g_mainRenderTargetView) { g_mainRenderTargetView->Release(); g_mainRenderTargetView = nullptr; }
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

// Win32 message handler
// You can read the io.WantCaptureMouse, io.WantCaptureKeyboard flags to tell if dear imgui wants to use your inputs.
// - When io.WantCaptureMouse is true, do not dispatch mouse input data to your main application, or clear/overwrite your copy of the mouse data.
// - When io.WantCaptureKeyboard is true, do not dispatch keyboard input data to your main application, or clear/overwrite your copy of the keyboard data.
// Generally you may always pass all inputs to dear imgui, and hide them from your application based on those two flags.
LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
        return true;

    switch (msg)
    {
    case WM_SIZE:
        if (wParam == SIZE_MINIMIZED)
            return 0;
        g_ResizeWidth = (UINT)LOWORD(lParam); // Queue resize
        g_ResizeHeight = (UINT)HIWORD(lParam);
        return 0;
    case WM_SYSCOMMAND:
        if ((wParam & 0xfff0) == SC_KEYMENU) // Disable ALT application menu
            return 0;
        break;
    case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }
    return ::DefWindowProcW(hWnd, msg, wParam, lParam);
}
