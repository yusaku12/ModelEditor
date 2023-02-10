#pragma once
#include "ImGuiRender.h"
#include "Singleton.h"
#include <string>
#include <d3d11.h>
#include "Graphics/Model.h"

class GUI : public Singleton<GUI>
{
    friend class Model;
    std::string model_path{ "" }, animation_path{ "" }, created_path{ "" }, created_anim_name{ "" }, material_path{ "" };
    bool fbx;
public:

    void Initialize();
    void Execute();
    void Render();
    void TransformUI();
    void MaterialUI();
    void AnimationUI();
    void TimelineUI();
    void MeshUI();
    void Reset();
    void BoneListUI();
    void NodeList();
    void BlendMode();
    void Shader();

    void Finalize();
    void Select();

private:
    int m_selected_item = -1;
    int s_type{};
    int selected_anim{};
    bool empty{ true };
    XMFLOAT3 scale{ 1, 1, 1 }, rotation{ 1,1,1 }, translation{ 1,1,1 };
    std::string model_name;
    XMFLOAT4 quaternion;
    bool created_popup{};
    std::string types[4] = { "DIFFUSE", "NORMAL", "BUMP", "EMPTY" };
    bool colliding{};
    std::shared_ptr<Model>selected;
    bool playAnim{};
    char anim_name[256] = "";
};