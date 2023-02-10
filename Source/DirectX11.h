#pragma once

#include <d3d11.h>
#include <DirectXMath.h>
#include <wrl.h>
#include "Graphics/Graphics.h"

using namespace Microsoft::WRL;
using namespace DirectX;
class DirectX11 : public Graphics
{
    DirectX11() {};
    ~DirectX11();
    XMMATRIX PR{}, OR{}, WO{};
    XMFLOAT4X4 projection{}, ortho{}, world{};
    int GPUMemory{};
    bool vSync{};

    ComPtr<IDXGIFactory>dxFactory;
    ComPtr<IDXGIAdapter>dxAdapter;
    ComPtr<IDXGIOutput>dxAdapterOutput;
    ComPtr<ID3D11Device>dxDevice;
    ComPtr<ID3D11DeviceContext>dxDeviceContext;
    ComPtr<IDXGISwapChain>dxSwapChain;
    ComPtr<ID3D11Texture2D>dxDepthStencilBuffer;
    ComPtr<ID3D11RenderTargetView>dxRenderTargetView;
    ComPtr<ID3D11DepthStencilState>dxDepthStencilState;
    ComPtr<ID3D11DepthStencilView>dxDepthStencilView;

public:
    static DirectX11* Instance()
    {
        static DirectX11 dx;
        return &dx;
    }
    HRESULT Initialize(int Width, int Height, bool VSYNC, HWND hwnd, bool isFullScreen, float Depth, float ScreenNear);
    void Begin(XMFLOAT4 colour);
    void End();
    ID3D11Device* Device()
    {
        return dxDevice.Get();
    }
    ID3D11DeviceContext* DeviceContext()
    {
        return dxDeviceContext.Get();
    }
    XMFLOAT4X4 Projection()
    {
        return projection;
    }
    XMFLOAT4X4 Ortho()
    {
        return ortho;
    }
    XMFLOAT4X4 World()
    {
        return world;
    }
    XMMATRIX ProjectionMatrix()
    {
        return PR;
    }
    XMMATRIX OrthoMatrix()
    {
        return OR;
    }
    XMMATRIX WorldMatrix()
    {
        return WO;
    }

    // ƒ‰ƒCƒ“ƒŒƒ“ƒ_ƒ‰Žæ“¾
    LineRenderer* GetLineRenderer() const { return lineRenderer.get(); }

private:
    std::unique_ptr<LineRenderer>					lineRenderer;
};