#pragma once

#include "Singleton.h"
#include <DirectXMath.h>
using namespace DirectX;

class Camera : public Singleton<Camera>
{
    float range{};
    XMFLOAT3 position{}, rotation{}, velocity{};
    XMFLOAT3 Eye{};
    XMFLOAT3 target{};
    XMFLOAT3 next_target{};
    XMMATRIX viewMatrix{};
    XMFLOAT4X4 view{};
    bool reset{};
public:
    // ÉrÉÖÅ[çsóÒéÊìæ
    const DirectX::XMFLOAT4X4& GetView() const { return view; }

    void SetPosition(XMFLOAT3 pos)
    {
        position = pos;
    }
    void SetVelocity(XMFLOAT3 vel)
    {
        velocity = vel;
    }
    void SetRotation(XMFLOAT3 rot)
    {
        rotation = rot;
    }
    void SetTarget(XMFLOAT3 t)
    {
        target = t;
    }
    void SetLookAt()
    {
        XMFLOAT3 u{ 0, 1, 0 };
        viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&Eye), XMLoadFloat3(&target), XMLoadFloat3(&u));
    }
    void SetRange(float r)
    {
        range = r;
    }
    void ResetCamera();
    void ResetToTarget(XMFLOAT3 t);

    float Range()
    {
        return range;
    }
    XMFLOAT3 Position()
    {
        return position;
    }
    XMFLOAT3 Velocity()
    {
        return velocity;
    }
    XMFLOAT3 Rotation()
    {
        return rotation;
    }
    XMFLOAT3 EyePosition()
    {
        return Eye;
    }
    XMMATRIX ViewMatrix()
    {
        return viewMatrix;
    }

    void Initialize(XMFLOAT3 Default_Eye_Position, XMFLOAT3 Target);
    void Execute();
    void Render();
};