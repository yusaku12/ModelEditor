#pragma once
#include <Windows.h>
#include <iomanip>
#include "Input/Input.h"
#include "Graphics/Graphics.h"
#include "ImGuiRender.h"

class System
{
    LPCWSTR applicationName;
    HINSTANCE hInstance;
    HWND hwnd;
    bool Frame();
    void InitializeWindows(int& Width, int& Height);
    void ShutdownWindows();
    System();
public:
    static System* Instance()
    {
        static System s;
        return &s;
    }
    ~System();
    bool Initialize();
    void Run();
    void Shutdown();
    LRESULT CALLBACK MessageHandler(HWND, UINT, WPARAM, LPARAM);
};

static LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam);

static System* ApplicationHandle{};