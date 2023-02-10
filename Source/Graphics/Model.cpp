#include "Model.h"
#include "Rasterizer.h"

HRESULT Model::Initialize(std::string model_path)
{
    resource = ModelResourceManager::Instance()->Load(model_path);
    if (!resource)
        return E_FAIL;
    animationTakes.resize(resource->Animations.size());
    for (int a = 0; a < animationTakes.size(); ++a)
        animationTakes[a] = resource->Animations[a].Name;

    for (int ind = 0; ind < Resource()->Meshes.size(); ++ind)
    {
        std::shared_ptr<BOUNDING_BOX>& b = Boxes.emplace_back();
        b = std::make_shared<BOUNDING_BOX>();
        b->Min.x = resource->Meshes[ind].BOUNDING_BOX[0].x;
        b->Min.y = resource->Meshes[ind].BOUNDING_BOX[0].y;
        b->Min.z = resource->Meshes[ind].BOUNDING_BOX[0].z;
        b->Max.x = resource->Meshes[ind].BOUNDING_BOX[1].x;
        b->Max.y = resource->Meshes[ind].BOUNDING_BOX[1].y;
        b->Max.z = resource->Meshes[ind].BOUNDING_BOX[1].z;
    }
    cur_AnimationTake = 0;

    return S_OK;
}

void Model::UpdateTransform()
{
    XMMATRIX S, R, T;
    S = XMMatrixScalingFromVector(XMLoadFloat3(&scale));
    R = XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion));
    T = XMMatrixTranslationFromVector(XMLoadFloat3(&translation));
    m_Transform = S * R * T;
    XMStoreFloat4x4(&transform, m_Transform);
}

void Model::Render(float SamplingRate, XMFLOAT4 colour)
{
    static float factor;
    static const int transition_delay{ 15 };
    static int transition_progress;
    AN::KEYFRAME* cur_kf, * next_kf, output;
    AN* cur_an, * next_an;

    if (cur_AnimationTake == -1)
        cur_AnimationTake = next_AnimationTake;
    if (cur_AnimationTake != next_AnimationTake && !isTransitioning)
    {
        transition_progress = next_Keyframe = 0;
        next_AnimTimer = 0.0f;
        isTransitioning = true;
    }
    else if (!isTransitioning)
        cur_AnimationTake = next_AnimationTake;
    cur_an = &Resource()->Animations.at(cur_AnimationTake);
    next_an = &Resource()->Animations.at(next_AnimationTake);
    SamplingRate = SamplingRate ? SamplingRate : cur_an->SamplingRate;
    if (isTransitioning)
    {
        ++transition_progress;
        transition_progress = (std::min)(transition_delay, transition_progress);
        factor = (float)transition_progress / (float)transition_delay;
    }
    else
        factor = 0.5f;
    if (!animPaused)
    {
        cur_Keyframe = (int)(cur_AnimTimer * SamplingRate);
        next_Keyframe = (int)(next_AnimTimer * SamplingRate);
    }
    if (cur_Keyframe > cur_an->Keyframes.size() - 1)
    {
        cur_Keyframe = 0;
        cur_AnimTimer = 0.0f;
    }
    else
        cur_AnimTimer += FRAMETIME;
    if (next_Keyframe > next_an->Keyframes.size() - 1)
    {
        next_Keyframe = 0;
        next_AnimTimer = 0.0f;
    }
    else
        next_AnimTimer += FRAMETIME;

    if (!isTransitioning)
        next_Keyframe = (std::min)(cur_Keyframe + 1, (int)next_an->Keyframes.size() - 1);

    cur_kf = &cur_an->Keyframes.at(cur_Keyframe);
    next_kf = &next_an->Keyframes.at(next_Keyframe);

    if (factor == 1)
    {
        isTransitioning = false;
        cur_AnimationTake = next_AnimationTake;
    }

    resource->BlendAnimation(cur_kf, next_kf, factor, &output);

    resource->UpdateAnimation(&output);
    resource->Render(DirectX11::Instance()->DeviceContext(), transform, colour, &output);
}
void Model::RenderWireframe(XMFLOAT4 colour)
{
    XMFLOAT3 old_scale{ scale };

    XMVECTOR S = XMVectorScale(XMLoadFloat3(&scale), 1.01f);
    XMStoreFloat3(&scale, S);
    SetScale(scale);
    UpdateTransform();
    DirectX11::Instance()->DeviceContext()->RSSetState(RasterizerManager::Instance()->Retrieve("Wireframe")->Rasterizer().Get());
    Render(0.0f, colour);
    SetScale(old_scale);
}
void Model::ResetTimer()
{
    cur_AnimTimer = 0;
}
bool Model::FinishedAnim()
{
    return cur_Keyframe >= Resource()->Animations.at(cur_AnimationTake).Keyframes.size() - 1;
}
bool Model::InAnim(int start, int end)
{
    return cur_Keyframe >= start && cur_Keyframe <= end;
}
void Model::PauseAnim()
{
    animPaused = true;
}
void Model::ResumeAnim()
{
    animPaused = false;
}
void Model::SetTranslation(XMFLOAT3 t)
{
    translation = t;
}
void Model::SetRotation(XMFLOAT3 r)
{
    rotation = r;
}
void Model::SetScale(XMFLOAT3 s)
{
    scale = s;
}
void Model::SetTransformation(XMFLOAT3 s, XMFLOAT3 r, XMFLOAT3 t)
{
    scale = s;
    rotation.x = XMConvertToRadians(r.x);
    rotation.y = XMConvertToRadians(r.y);
    rotation.z = XMConvertToRadians(r.z);
    XMStoreFloat4(&quaternion, (XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));
    translation = t;
}
void Model::SetTransformation(XMFLOAT3 s, XMFLOAT4 q, XMFLOAT3 t)
{
    scale = s;
    quaternion = q;
    translation = t;
}
void Model::OffsetTransform(XMMATRIX mat)
{
    XMMATRIX cur{ XMLoadFloat4x4(&transform) };
    XMStoreFloat4x4(&transform, cur * mat);
}
void Model::SetTake(int take)
{
    next_AnimationTake = take;
}
void Model::SetFrame(int frame)
{
    cur_Keyframe = frame;
}
int Model::CurrentTake()
{
    return cur_AnimationTake;
}
int Model::CurrentFrame()
{
    return cur_Keyframe;
}
XMFLOAT3 Model::Scale()
{
    return scale;
}
XMFLOAT3 Model::Rotation()
{
    return rotation;
}
XMFLOAT3 Model::Translation()
{
    return translation;
}
XMFLOAT4X4 Model::Transform()
{
    return transform;
}
XMMATRIX Model::TransformMatrix()
{
    return XMMatrixScaling(scale.x, scale.y, scale.z) * XMMatrixRotationQuaternion(XMLoadFloat4(&quaternion)) * XMMatrixTranslation(translation.x, translation.y, translation.z);
    XMFLOAT4 q;
    XMFLOAT3 r{ XMConvertToRadians(rotation.x), XMConvertToRadians(rotation.y), XMConvertToRadians(rotation.z) };
    XMStoreFloat4(&q, (XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))));

    return XMMatrixScalingFromVector(XMLoadFloat3(&scale)) * XMMatrixRotationQuaternion(XMLoadFloat4(&q)) * XMMatrixTranslationFromVector(XMLoadFloat3(&translation));
}
std::vector<std::string>Model::AnimationTakes()
{
    return animationTakes;
}
std::shared_ptr<FBXModel>Model::Resource()
{
    return resource;
}
std::vector<std::shared_ptr<Model::BOUNDING_BOX>>Model::GetBB()
{
    return Boxes;
}
XMFLOAT3 Model::Right()
{
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, TransformMatrix().r[0]);
    return temp;
}
XMFLOAT3 Model::Up()
{
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, TransformMatrix().r[1]);
    return temp;
}
XMFLOAT3 Model::Forward()
{
    XMFLOAT3 temp;
    XMStoreFloat3(&temp, TransformMatrix().r[2]);
    return temp;
}
void Model::RetrieveAxisesQ(XMFLOAT3* r, XMFLOAT3* u, XMFLOAT3* f)
{
    UpdateTransform();
    XMMATRIX temp{ TransformMatrix() };
    if (r)
    {
        XMFLOAT3 t;
        XMStoreFloat3(&t, temp.r[0]);
        *r = t;
    }
    if (u)
    {
        XMFLOAT3 t;
        XMStoreFloat3(&t, temp.r[1]);
        *u = t;
    }
    if (f)
    {
        XMFLOAT3 t;
        XMStoreFloat3(&t, temp.r[2]);
        *f = t;
    }
}