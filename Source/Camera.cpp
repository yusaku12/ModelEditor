#include "Camera.h"
#include "Input/Input.h"
//#include "DEBUG_PRIMITIVE.h"
#include "imgui.h"
//std::shared_ptr<DEBUG_SPHERE>s;

inline float Lerp(float f1, float f2, float factor)
{
    return f1 + (f2 - f1) * factor;
}

using namespace DirectX;
void Camera::Initialize(XMFLOAT3 Default_Eye_Position, XMFLOAT3 Target)
{
    Eye = Default_Eye_Position;
    target = Target;
    XMVECTOR UP{ 0.0f, 1.0f, 0.0f, 0.0f };
    viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&Eye), XMLoadFloat3(&target), UP);
}

void Camera::Execute()
{
    INPUTMANAGER* i = INPUTMANAGER::Instance();
    INPUTMANAGER::MOUSE* m = INPUTMANAGER::Instance()->Mouse().get();
    float wheel{};
    if (m->Wheel().Up().Held())
        wheel = 5.0f;
    if (m->Wheel().Down().Held())
        wheel = -5.0f;
    static XMFLOAT2 clicked_pos{}, move_pos{};
    static XMFLOAT2 movement;
    XMFLOAT2 pos, pos2, drag_pos;
    static bool start{};
    if (i->AltKeys()->State().Held())
    {
        if (m->LButton().Triggered()) {
            clicked_pos.x += m->fPosition().x;
            clicked_pos.y += m->fPosition().y;
            start = true;
        }
        if (m->LButton().Held() && start)
        {
            pos = m->fPosition();
            XMFLOAT2 vector{ pos.x - clicked_pos.x,pos.y - clicked_pos.y };
            vector.x *= 0.1f;
            vector.y *= 0.1f;
            rotation.y += XMConvertToRadians(vector.x);
            rotation.x += -XMConvertToRadians(vector.y);
            clicked_pos = pos;
        }
        if (m->LButton().Released()) {
            clicked_pos = {};
            start = false;
        }
        if (m->RButton().Triggered()) {
            move_pos.x += m->fPosition().x;
            move_pos.y += m->fPosition().y;
            start = true;
        }
        if (m->RButton().Held() && start)
        {
            pos2 = m->fPosition();
            movement.x += -(pos2.x - move_pos.x);
            movement.y += pos2.y - move_pos.y;
            movement.x *= 0.03f;
            movement.y *= 0.03f;
            move_pos = pos2;
        }
        if (m->RButton().Released())
        {
            move_pos = {};
            movement = {};
            start = false;
        }
    }

    XMMATRIX temp{ XMMatrixRotationRollPitchYawFromVector(XMLoadFloat3(&rotation)) };
    XMFLOAT3 horizontol, vertical, forward;
    DirectX::XMStoreFloat3(&horizontol,(temp.r[0]));
    DirectX::XMStoreFloat3(&vertical, (temp.r[1]));
    XMVECTOR VERTICAL = XMVector3Normalize(XMLoadFloat3(&vertical));
    DirectX::XMStoreFloat3(&forward, XMVectorSubtract(XMLoadFloat3(&Eye), XMLoadFloat3(&target)));
    XMVECTOR FORWARD = XMVector3Normalize(XMLoadFloat3(&forward));
    float d = {};
    DirectX::XMStoreFloat(&d, XMVectorScale(XMVector3Normalize(XMLoadFloat3(&horizontol)), movement.x));
    float K = {};
    XMStoreFloat(&K, XMLoadFloat3(&target));
    K += d + vertical.x * movement.y;
    float sq = sqrtf(forward.x * forward.x + forward.y * forward.y + forward.z * forward.z);
    range += -sq * wheel * 0.1f;

    range = (std::max)(1.0f, range);
    XMMATRIX T{ XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, 0) };
    XMVECTOR F{ T.r[2] };
    XMFLOAT3 f;
    DirectX::XMStoreFloat3(&f, F);

    Eye.x = target.x + (f.x * -range);
    Eye.y = target.y + (f.y * range);
    Eye.z = target.z + (f.z * -range);

    SetLookAt();

    if (reset)
    {
        XMStoreFloat3(&target,(XMVectorLerp(XMLoadFloat3(&target), XMLoadFloat3(&next_target), 0.1f)));
        range = Lerp(range, 10.0f, 0.1f);
        XMFLOAT3 Sub = {};
        XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&next_target)));
        float Length = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);
        if (Length < 0.001f && range - 10.0f < 0.001f)
        {
            reset = false;
            target = next_target;
            range = 10.0f;
        }
    }
}

void Camera::Render()
{
    ImGui::Begin("Position");
    ImGui::InputFloat3("Eye", &Eye.x);
    ImGui::InputFloat3("Target", &target.x);
    ImGui::End();
}

void Camera::ResetCamera()
{
    target = {};
    Eye = {};
    rotation = {};
}

void Camera::ResetToTarget(XMFLOAT3 t)
{
    next_target = t;
    reset = true;
}