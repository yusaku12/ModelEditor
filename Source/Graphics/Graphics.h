#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Light.h"
#include "Singleton.h"
#include "LineRender.h"

using namespace DirectX;

class Graphics : public Singleton<Graphics>
{
    Microsoft::WRL::ComPtr<ID3D11Buffer>dxSceneConstantBuffer;
    bool Render();
public:
    struct SCENE_CONSTANT_DATA
    {
        // View projection 
        XMFLOAT4X4 view_proj{};
        // Camera Position
        XMFLOAT4 camera_position{};
        XMFLOAT4 ambientLightColour;
        // Light Data
        DLIGHT_DATA directional;
        //PLIGHT_DATA pointlights[PLIGHT_MAX];
        //SLIGHT_DATA spotlights[SLIGHT_MAX];
        //int pLightCount;
        //int sLightCount;
        //VECTOR2 temp;
    };

    HRESULT Initialize(int Width, int Height, HWND hwnd);
    bool Frame();
    void Finalize();

    // ƒOƒŠƒbƒh•`‰æ
    void DrawGrid(ID3D11DeviceContext* context, int subdivisions, float scale);
};