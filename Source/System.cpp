#include "System.h"
#include "DropManager.h"
#include <windowsx.h>
#include "PERFORMANCE_COUNTER.h"
#include "DirectX11.h"

#define BeginQuery PerformanceCounter::Instance()->BeginQuery
#define EndQuery PerformanceCounter::Instance()->EndQuery
bool FullScreen{};
bool vSync{ false };
const float SCREEN_DEPTH{ 1000.0f };
const float SCREEN_NEAR{ 0.1f };

System::System()
{
}

System::~System()
{

}

bool System::Initialize()
{
    int S_W{}, S_H{};
    bool result{};
    InitializeWindows(S_W, S_H);
    IMGUI::Instance()->Initialize(DirectX11::Instance());
    INPUTMANAGER::Instance()->Initialize();
    return true;
}
void System::Shutdown()
{
    Graphics::Instance()->Finalize();
    IMGUI::Instance()->End();
    ShutdownWindows();
    PostQuitMessage(0);
}

void System::Run()
{
    MSG msg{};
    bool done{ false }, result{};

    while (!done)
    {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        else {
            msg.message == WM_QUIT || msg.message == WM_CLOSE ? done = true : result = Frame();
        }
        if (msg.message == WM_QUIT || msg.message == WM_CLOSE)
            done = true;
        result ? done = true : 0;
    }
}

bool System::Frame()
{
    bool result;
    BeginQuery();
    result = Graphics::Instance()->Frame();
    EndQuery();
    SetWindowTextA(hwnd, PerformanceCounter::Instance()->Results().str().c_str());
    if (!result)
        return false;
    return true;
}

LRESULT CALLBACK System::MessageHandler(HWND hwnd, UINT umsg, WPARAM wparam, LPARAM lparam)
{
    if (ImGui_ImplWin32_WndProcHandler(hwnd, umsg, wparam, lparam))
        return true;

    switch (umsg)
    {
    case WM_CREATE:
        RECT r;
        GetClientRect(hwnd, &r);
        Graphics::Instance()->Initialize(r.right - r.left, r.bottom - r.top, hwnd);
        break;
    case WM_CLOSE:
        System::Instance()->Shutdown();
        PostQuitMessage(0);
        return 0;
    case WM_DROPFILES:
    {
        TCHAR text[256] = TEXT("");
        HDROP drop{ (HDROP)wparam };
        DragQueryFile(drop, 0, text, 256);
        DROPMANAGER::Instance()->Load(text);
        return 0;
    }
    case WM_KEYDOWN:
        INPUTMANAGER::Instance()->Keyboard()->KeyDown((unsigned int)wparam);
        return 0;
    case WM_KEYUP:
        INPUTMANAGER::Instance()->Keyboard()->KeyUp((unsigned int)wparam);
        if (GetAsyncKeyState(VK_ESCAPE))
            PostMessage(hwnd, WM_CLOSE, 0, 0);
        return 0;
    case WM_SYSKEYDOWN:
        INPUTMANAGER::Instance()->AltKeys()->OnPress();
        return 0;
    case WM_SYSKEYUP:
        INPUTMANAGER::Instance()->AltKeys()->OnRelease();
        return 0;
    case WM_MOUSEMOVE:
        INPUTMANAGER::Instance()->Mouse()->SetPosition(GET_X_LPARAM(lparam), GET_Y_LPARAM(lparam));
        return 0;
    case WM_LBUTTONDOWN:
        INPUTMANAGER::Instance()->Mouse()->OnPressLButton();
        return 0;
    case WM_LBUTTONUP:

        INPUTMANAGER::Instance()->Mouse()->OnReleaseLButton();
        return 0;
    case WM_RBUTTONDOWN:
        INPUTMANAGER::Instance()->Mouse()->OnPressRButton();
        return 0;

    case WM_RBUTTONUP:
        INPUTMANAGER::Instance()->Mouse()->OnReleaseRButton();
        return 0;
    case WM_MBUTTONDOWN:
        INPUTMANAGER::Instance()->Mouse()->OnPressMButton();
        return 0;
    case WM_MBUTTONUP:
        INPUTMANAGER::Instance()->Mouse()->OnReleaseMButton();
        return 0;
    case WM_MOUSEWHEEL:
        if (GET_WHEEL_DELTA_WPARAM(wparam) > 0)
            INPUTMANAGER::Instance()->Mouse()->OnMousewheelUp();
        else if (GET_WHEEL_DELTA_WPARAM(wparam) < 0)
            INPUTMANAGER::Instance()->Mouse()->OnMouseWheelDown();
        return 0;
    default:
        return DefWindowProc(hwnd, umsg, wparam, lparam);
    }
}

void System::InitializeWindows(int& S_W, int& S_H)
{
    WNDCLASSEX wnd;
    DEVMODE dmScreenSettings;
    int posX{}, posY{};

    ApplicationHandle = this;
    hInstance = GetModuleHandle(NULL);
    applicationName = L"ModelEditer";

    wnd.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    wnd.lpfnWndProc = WndProc;
    wnd.cbClsExtra = 0;
    wnd.cbWndExtra = 0;
    wnd.hInstance = hInstance;
    wnd.hIcon = LoadIcon(NULL, IDI_WINLOGO);
    wnd.hIconSm = wnd.hIcon;
    wnd.hCursor = LoadCursor(NULL, IDC_ARROW);
    wnd.hbrBackground = (HBRUSH)(BLACK_BRUSH);
    wnd.lpszMenuName = NULL;
    wnd.lpszClassName = applicationName;
    wnd.cbSize = sizeof(WNDCLASSEX);
    RegisterClassEx(&wnd);
    FullScreen = false;
    S_W = GetSystemMetrics(SM_CXSCREEN);
    S_H = GetSystemMetrics(SM_CYSCREEN);
    if (FullScreen)
    {
        memset(&dmScreenSettings, 0, sizeof(dmScreenSettings));
        dmScreenSettings.dmSize = sizeof(dmScreenSettings);
        dmScreenSettings.dmPelsWidth = (unsigned long)S_W;
        dmScreenSettings.dmPelsHeight = (unsigned long)S_H;
        dmScreenSettings.dmBitsPerPel = 32;
        dmScreenSettings.dmFields = DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;
        ChangeDisplaySettings(&dmScreenSettings, CDS_FULLSCREEN);
    }
    else
    {
        S_W = 1240;
        S_H = 720;
        posX = (GetSystemMetrics(SM_CXSCREEN) - S_W) / 2;
        posY = (GetSystemMetrics(SM_CYSCREEN) - S_H) / 2;
    }
    hwnd = CreateWindowEx(WS_EX_CLIENTEDGE, applicationName, applicationName, WS_OVERLAPPEDWINDOW, posX, posY, S_W, S_H, NULL, NULL, hInstance, NULL);

    ShowWindow(hwnd, SW_SHOW);
    SetForegroundWindow(hwnd);
    SetFocus(hwnd);
    DragAcceptFiles(hwnd, TRUE);

    IMGUI::Instance()->Initialize(hwnd);
}

void System::ShutdownWindows()
{
    ShowCursor(true);
    if (FullScreen)
        ChangeDisplaySettings(NULL, 0);
    hwnd = {};

    UnregisterClass(applicationName, hInstance);
    hInstance = {};

    ApplicationHandle = {};
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT umessage, WPARAM wparam, LPARAM lparam)
{
    switch (umessage)
    {
    case WM_DESTROY:
        System::Instance()->Shutdown();
        PostQuitMessage(0);
        return 0;
    case WM_CLOSE:
        System::Instance()->Shutdown();
        PostQuitMessage(0);
        return 0;
    default:
        return ApplicationHandle->MessageHandler(hwnd, umessage, wparam, lparam);
    }
}