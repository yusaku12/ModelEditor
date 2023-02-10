#pragma once
#include <d3d11.h>
#include <memory>
#include <DirectXMath.h>
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"
#include "imfilebrowser.h"
#include "Singleton.h"
#include "DROPMANAGER.h"
#include "DirectX11.h"

using namespace DirectX;

class IMGUI : public Singleton<IMGUI>
{
    ImGuiContext* imContext;
    ImGui::FileBrowser fileBrowser;
    ImGui::FileBrowser fileCreator;
    ImVec4 default_colour;
    ImVec4 highlighted_colour;
    ImVec4 colour;
public:
    // Called when initializing window handler to initialize imgui_win32
    HRESULT Initialize(HWND hwnd)
    {
        imContext = { ImGui::CreateContext() };
        ImGui::SetCurrentContext(imContext);
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_ViewportsEnable;
        ImGui::GetIO().ConfigFlags |= ImGuiConfigFlags_DockingEnable;
        default_colour = ImGui::GetStyleColorVec4(ImGuiCol_FrameBg);
        highlighted_colour = { default_colour.x * 1.1f, default_colour.w * 1.1f , default_colour.x * 1.1f , default_colour.w };
        colour = default_colour;
        fileCreator = ImGui::FileBrowser(ImGuiFileBrowserFlags_CreateNewDir | ImGuiFileBrowserFlags_EnterNewFilename);

        //色変化
        ImGuiStyle& style = ImGui::GetStyle();
        {
            //文字色
            style.Colors[ImGuiCol_Text] = { 1,1,0,1 };

            //バーの色変化
            style.Colors[ImGuiCol_MenuBarBg] = { 0.6f,0.5f,0.7f,1.0f };

            //チェックマーク色変化
            style.Colors[ImGuiCol_CheckMark] = { 1,0,0,1 };

            //ボタン色変化
            style.Colors[ImGuiCol_Button] = { 0,1,1,0 };

            //縁
            style.Colors[ImGuiCol_Border] = { 2,1,3,1 };

            //閉じたときの色変化
            style.Colors[ImGuiCol_BorderShadow] = { 1,0,1,1 };

            //スライダーの色変化
            style.Colors[ImGuiCol_SliderGrab] = { 1,0,1,1 };

            //separetの色変化
            style.Colors[ImGuiCol_Separator] = { 0,2,2,1 };

            //下の△の色変化
            style.Colors[ImGuiCol_ResizeGrip] = { 2,0,0,1 };
        }

        return ImGui_ImplWin32_Init(hwnd) ? S_OK : E_FAIL;
        //return s ? S_OK : E_FAIL;
    }

    // Called when initializing window handler to initialize imgui_dx11
    HRESULT Initialize(DirectX11* dx)
    {
        return ImGui_ImplDX11_Init(dx->Device(), dx->DeviceContext()) ? S_OK : E_FAIL;
    }

    // Called at the start of every frame
    void Execute()
    {
        ImGui_ImplDX11_NewFrame();
        ImGui_ImplWin32_NewFrame();
        ImGui::NewFrame();
    }


    void Render()
    {
        //Execute();
        ImGui::Render();

        ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
        if (!(ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_ViewportsEnable))
            return;
        ImGui::UpdatePlatformWindows();
        ImGui::RenderPlatformWindowsDefault();
    }

    // Cleans the objects created by ImGui
    void End()
    {
        ImGui_ImplWin32_Shutdown();
        ImGui_ImplDX11_Shutdown();
        ImGui::DestroyContext(imContext);
        //delete imContext;
    };

    // Creates a popup based on error_string. Perform status(bool) check outside and put the pointer of it here
    void CreatePopup(std::string error_string, bool* status)
    {
        if (!*status)
            return;
        ImGui::OpenPopup("Error");

        if (ImGui::BeginPopupModal("Error")) {
            ImGui::Text(error_string.c_str());
            if (ImGui::Button("Ok")) {
                ImGui::CloseCurrentPopup();

                *status = false;
            }
            ImGui::EndPopup();
        }

    }

    // Drag and drop string
    void InputText(std::string label, std::string* str)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, default_colour);
        ImGui::InputText(label.c_str(), (char*)str->c_str(), 256);
        if (ImGui::IsItemHovered())
        {
            //colour = highlighted_colour;
            ImVec2 min, max;
            min = ImGui::GetItemRectMin();
            max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(0, 255, 255, 100));
            if (DROPMANAGER::Instance()->Loaded())
                *str = DROPMANAGER::Instance()->Path();
        }
        else
            colour = default_colour;
        ImGui::PopStyleColor();
    }

    // Drag and drop wstring
    void InputText(std::string label, std::wstring* str)
    {
        ImGui::PushStyleColor(ImGuiCol_FrameBg, default_colour);
        ImGui::InputText(label.c_str(), (char*)str->c_str(), 256);
        if (ImGui::IsItemHovered())
        {
            //colour = highlighted_colour;
            ImVec2 min, max;
            min = ImGui::GetItemRectMin();
            max = ImGui::GetItemRectMax();
            ImGui::GetWindowDrawList()->AddRectFilled(min, max, IM_COL32(0, 255, 255, 100));
            if (DROPMANAGER::Instance()->Loaded())
                *str = DROPMANAGER::Instance()->WPath();
        }
        else
            colour = default_colour;
        ImGui::PopStyleColor();
    }

    // Display file browser, output string
    void DisplayBrowser(std::string* output_string, bool* openFlag)
    {
        if (openFlag)
        {
            fileBrowser.Display();
            if (fileBrowser.HasSelected())
            {
                *output_string = fileBrowser.GetSelected().string();
                fileBrowser.Close();
                *openFlag = false;
            }
        }
    }

    // Display file browser, output wstring
    void DisplayBrowser(std::wstring* output_string, bool* openFlag)
    {
        if (openFlag)
        {
            fileBrowser.Display();
            if (fileBrowser.HasSelected())
            {
                *output_string = fileBrowser.GetSelected().wstring();
                fileBrowser.Close();
                *openFlag = false;
            }
        }
    }

    ImGui::FileBrowser* FileBrowser()
    {
        return &fileBrowser;
    }
    ImGui::FileBrowser* FileCreator()
    {
        return &fileCreator;
    }
};