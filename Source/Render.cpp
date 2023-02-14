#include "Render.h"
#include "DropManager.h"
#include "atlstr.h"
#include "Shader.h"
#include "Input/Input.h"
#include "Camera.h"
#include "Collision.h"
#include "Graphics/FBXModel.h"

///*------------------------------------------------GUI Class-----------------------------------------------------------------*/
///*--------------------------------------------------------------------------------------------------------------------------*/
///*-------------------------------------------------GUI Initialize()---------------------------------------------------------*/
///*--------------------------------------------------------------------------------------------------------------------------*/

std::shared_ptr<Model>model;
static OPENFILENAME ofn = { 0 };
static TCHAR strFile[MAX_PATH], strCustom[256] = TEXT("Before files\0*.*\0\0");

void GUI::Initialize()
{
}

///*-------------------------------------------------GUI Execute()---------------------------------------------------------*/

void GUI::Execute()
{
    std::string format;

    ImGui::BeginMainMenuBar();
    if (ImGui::BeginMenu("FILE"))
    {
        //モデル開く
        if (ImGui::MenuItem("OPEN MODEL"))
        {
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.lpstrFilter = TEXT("FBX files {*.fbx;*.mdl}\0*.fbx;*.mdl\0")
                              TEXT("OBJ files {*.obj*.mdl}\0*.obj;*.mdl\0");
            ofn.lpstrCustomFilter = strCustom;
            ofn.nMaxCustFilter = 256;
            ofn.nFilterIndex = 0;
            ofn.lpstrFile = strFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST;
            BOOL open = GetOpenFileName(&ofn);
            if (open)
            {
                std::filesystem::path path(model_path);
                std::wstring wpath = strFile;
                std::string s_temp(wpath.begin(), wpath.end());

                model = std::make_shared<Model>();
                HRESULT hr = model->Initialize(s_temp);
                if (hr == S_OK) {
                    model->SetTake(0);
                    model->SetFrame(0);
                }
                Reset();
            }
        }

        //モデルセーブ
        if (ImGui::MenuItem("SAVE MODEL"))
        {
            ofn.lStructSize = sizeof(OPENFILENAME);
            ofn.lpstrFilter = TEXT("mdl files {*.mdl}\0*.mdl\0");
            ofn.lpstrFile = strFile;
            ofn.nMaxFile = MAX_PATH;
            ofn.Flags = OFN_FILEMUSTEXIST | OFN_OVERWRITEPROMPT;
            BOOL save = GetSaveFileName(&ofn);
            if (save)
            {
                format = { ".mdl" };
                std::filesystem::path path(created_path);
                std::wstring wpath = strFile;
                std::string s_temp(wpath.begin(), wpath.end());
                model->resource->Recreate(s_temp);
            }
        }
    ImGui::EndMenu();
    }

    //カメラリセット
    if (ImGui::MenuItem("Reset Camera"))
    {
        Camera::Instance()->ResetToTarget({ 0, 0, 0 });
    }

    //シェーダー変更
    Shader();

    //ブレンドモード変更
    BlendMode();

    ImGui::EndMainMenuBar();

    if (model != nullptr)
    {
        TransformUI();
        AnimationUI();
        MaterialUI();
        TimelineUI();
        BoneListUI();
        MeshUI();
        XMStoreFloat4(&quaternion, (XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));
        model->SetTransformation(scale, quaternion, translation);
        model->UpdateTransform();
    }
}

/*-------------------------------------------------GUI TransformUI()---------------------------------------------------------*/

void GUI::TransformUI()
{
    ImGui::Begin("Transform");
    ImGui::InputFloat3("Scale : ", &scale.x);
    ImGui::InputFloat3("Rotation : ", &rotation.x);
    ImGui::InputFloat3("Translation : ", &translation.x);
    ImGui::End();
}

/*-------------------------------------------------GUI AnimationUI()---------------------------------------------------------*/

void GUI::AnimationUI()
{
    ImGui::Begin("Animation");
    std::vector<FBXModel::ANIMATION>& anims = model->Resource()->Animations;
    ImGui::ListBoxHeader("Animations");
    int ind{};
    for (auto& a : anims)
    {
        bool s{};
        if (ImGui::Selectable(a.Name.c_str(), &s)) {
            selected_anim = ind;
            break;
        }
        ++ind;
    }

    ImGui::ListBoxFooter();

    if (selected_anim > -1)
        model->SetTake(selected_anim);

    if (ImGui::Button("Delete Animation") || INPUTMANAGER::Instance()->Keyboard()->Triggered(VK_DELETE))
    {
        anims.erase(anims.begin() + selected_anim);
        selected_anim = 0;
        model->SetTake(0);
        ImGui::End();
        return;
    }

    ImGui::Checkbox("Play Animation ?", &playAnim);
    if (!playAnim)
        model->PauseAnim();
    else
        model->ResumeAnim();

    //アニメーションロード
    if (ImGui::Button("Load animation"))
    {
        ofn.lStructSize = sizeof(OPENFILENAME);
        ofn.lpstrFilter = TEXT("FBX files {*.fbx}\0*.fbx\0");
        ofn.lpstrCustomFilter = strCustom;
        ofn.nMaxCustFilter = 256;
        ofn.nFilterIndex = 0;
        ofn.lpstrFile = strFile;
        ofn.nMaxFile = MAX_PATH;
        ofn.Flags = OFN_FILEMUSTEXIST;
        BOOL open = GetOpenFileName(&ofn);
        if (open)
        {
            std::filesystem::path full_name(animation_path);
            std::filesystem::path name(full_name.filename());
            std::wstring wpath = strFile;
            std::string s_temp(wpath.begin(), wpath.end());
            model->Resource()->AddAnimation(s_temp, s_temp);
        }
    }

    ImGui::InputText("Rename Animation", anim_name, 256);
    if (ImGui::Button("Rename"))
    {
        anims[selected_anim].Name = std::string(anim_name);
    }

    float rate{};
    ImGui::DragFloat("Sampling Rate", &anims[selected_anim].SamplingRate, 0.1f, 0.0f, 120.0f);
    ImGui::End();
}

///*-------------------------------------------------GUI MaterialUI()---------------------------------------------------------*/

void GUI::MaterialUI()
{
    ImGui::Begin("Materials");

    ImGui::ListBoxHeader("Materials");

    for (auto& m : model->Resource()->Materials)
    {
        //const char* cstr = m.second.name.c_str();
        bool s{};
        if (ImGui::Selectable("mat", &s))
        {
            m_selected_item = m.first;
            break;
        }
    }

    ImGui::ListBoxFooter();

    if (m_selected_item < 0)
    {
        ImGui::End();
        return;
    }
    for (auto& t : model->Resource()->Materials.find(m_selected_item)->second.texture_path)
    {
        ImGui::Text(t.c_str());
    }

    FBXModel::MATERIAL& m{ model->Resource()->Materials.find(m_selected_item)->second };
    int ind{};
    if (ImGui::BeginCombo("Material Type", types[s_type].c_str()))
    {
        for (auto& t : types)
        {
            bool s{};
            if (ImGui::Selectable(t.c_str(), &s))
            {
                s_type = ind;
                break;
            }
            ++ind;
        }
        ImGui::EndCombo();
    }

    ImGui::FileBrowser* browser{ IMGUI::Instance()->FileBrowser() };
    static bool openFileM{};

    if (ImGui::Button("Load Texture"))
    {
        browser->Open();
        browser->SetTitle("Open texture file");
        browser->SetTypeFilters({ ".png", ".jpg", ".tga",".DDS", ".*" });
        openFileM = true;
        model->Resource()->AddMaterial(material_path, model_name, (FBXModel::MATERIAL_TYPE)s_type, model->Resource()->Materials.find(m_selected_item)->second);
    }

    if (openFileM)
    {
        browser->Display();
        if (browser->HasSelected())
        {
            material_path = browser->GetSelected().string();
            model->Resource()->AddMaterial(material_path, model_name, (FBXModel::MATERIAL_TYPE)s_type, model->Resource()->Materials.find(m_selected_item)->second);
            browser->Close();
            openFileM = false;
        }
    }

    ImGui::End();
}

///*-------------------------------------------------GUI TimelineUI()---------------------------------------------------------*/

void GUI::TimelineUI()
{
    if (!model)
        return;
    FBXModel::ANIMATION& a{ model->Resource()->Animations.at(selected_anim) };
    ImGui::Begin("Animation Timeline");
    std::string t{ "Animation Name : " };
    t += a.Name;
    ImGui::Text(t.c_str());
    ImGui::SliderInt("Frames : ", &model->cur_Keyframe, 0, a.Keyframes.size() - 1);
    ImGui::End();
}

///*-------------------------------------------------GUI MeshUI()---------------------------------------------------------*/

void GUI::MeshUI()
{
    if (!model)
        return;
    ImGui::Begin("Meshes");
    for (auto& m : model->Resource()->Meshes)
    {
        ImGui::Text(m.Name.c_str());
    }
    ImGui::End();
}

///*-------------------------------------------------GUI Render()---------------------------------------------------------*/

void GUI::Render()
{
    if (model == nullptr)
        return;
    model->Render();
    model->RenderWireframe();
    ImGui::Begin("Collision");
    ImGui::Checkbox("Colliding ? ", &colliding);
    ImGui::End();
}

///*-------------------------------------------------GUI Finalize()---------------------------------------------------------*/

void GUI::Finalize()
{
}

///*-------------------------------------------------GUI Select()---------------------------------------------------------*/

void GUI::Select()
{
    if (!INPUTMANAGER::Instance()->Keyboard()->Held(VK_CONTROL))
    {
        return;
    }
    if (model != nullptr)
        return;
    XMFLOAT3 cur_pos;
    cur_pos.x = INPUTMANAGER::Instance()->Mouse()->fPosition().x;
    cur_pos.y = INPUTMANAGER::Instance()->Mouse()->fPosition().y;
    D3D11_VIEWPORT vp;
    UINT num{ 1 };
    DirectX11::Instance()->DeviceContext()->RSGetViewports(&num, &vp);
    XMVECTOR Near, Far;
    cur_pos.z = 0;

    Near = XMVector3Unproject(XMLoadFloat3(&cur_pos), vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth, DirectX11::Instance()->ProjectionMatrix(), Camera::Instance()->ViewMatrix(), model->TransformMatrix());
    cur_pos.z = 1.0f;
    Far = XMVector3Unproject(XMLoadFloat3(&cur_pos), vp.TopLeftX, vp.TopLeftY, vp.Width, vp.Height, vp.MinDepth, vp.MaxDepth, DirectX11::Instance()->ProjectionMatrix(), Camera::Instance()->ViewMatrix(), model->TransformMatrix());

    XMFLOAT3 N, F;
    XMStoreFloat3(&N, Near);
    XMStoreFloat3(&F, Far);

    COLLIDERS::RAYCASTDATA rcd{};

    if (INPUTMANAGER::Instance()->Mouse()->LButton().Triggered())
    {
        if (COLLIDERS::RayCast(N, F, model.get(), rcd))
        {
            int a = 0;
            colliding = true;
            selected = model;
        }
        else {
            colliding = false;
            selected.reset();
        }
    }
}

///*-------------------------------------------------GUI Reset()---------------------------------------------------------*/

void GUI::Reset()
{
    material_path = model_path = animation_path = created_anim_name = created_path = "";
    m_selected_item = -1;
    s_type = selected_anim = {};
}

///*-------------------------------------------------GUI BoneListUI()---------------------------------------------------------*/

void GUI::BoneListUI()
{
    if (ImGui::Begin("Bones"));
    {
        for (auto& m : model->Resource()->Meshes)
        {
            if (ImGui::TreeNode(m.Name.c_str()))
            {
                for (auto& b : m.Bind_Pose.Bones)
                {
                    ImGui::Text(b.Name.c_str());
                }
                ImGui::TreePop();
            }
        }
        ImGui::End();
    }
}

void OutputChild(int node_index)
{
    FBXModel::SCENE::NODE node = model->Resource()->Scenes.Nodes[node_index];
    int parent_node_index = node.p_Index;
    if (parent_node_index != -1) {
        ImGui::Text(node.Name.c_str());
        node_index = parent_node_index;
        OutputChild(node_index);
    }
    return;
}

void GUI::NodeList()
{
    if (ImGui::Begin("Nodes"))
    {
        for (auto& node : model->resource->Scenes.Nodes)
        {
            if (ImGui::TreeNode(node.Name.c_str()))
            {
                for (auto& child_node : model->resource->Scenes.Nodes)
                {
                    OutputChild(child_node.p_Index);
                }
            }
        }
    }
}

void GUI::BlendMode()
{
    if (ImGui::BeginMenu("BLENDMODE"))
    {
        if (ImGui::MenuItem("ALPHA"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::ALPHA, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("LIGHTEN"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::LIGHTEN, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("DARKEN"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::DARKEN, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("ADD"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::ADD, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("SUBTRACT"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::SUBTRACT, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("MULTIPLY"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::MULTIPLY, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("REPLACE"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::REPLACE, DirectX11::Instance()->Device());
        }
        if (ImGui::MenuItem("SCREEN"))
        {
            BlendModeManager::Instance()->Create(BlendModeManager::BLEND_MODE::SCREEN, DirectX11::Instance()->Device());
        }
        ImGui::EndMenu();
    }
}

void GUI::Shader()
{
    if (ImGui::BeginMenu("SHADER"))
    {
        if (ImGui::MenuItem("LAMBERT"))
        {
            model->Resource()->InsertShader(L"Lambert.fx");
        }
        else if (ImGui::MenuItem("PHONG"))
        {
            model->Resource()->InsertShader(L"Phong.fx");
        }
        ImGui::EndMenu();
    }
}