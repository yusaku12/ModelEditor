#include "BlendMode.h"
#include "imgui.h"

HRESULT BlendModeManager::Create(BLEND_MODE m, ID3D11Device* dv)
{
    D3D11_BLEND_DESC bld{};
    bld.AlphaToCoverageEnable = true;
    bld.IndependentBlendEnable = false;
    bld.RenderTarget[0].BlendEnable = true;
    switch (m)
    {
    case BLEND_MODE::ALPHA:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        break;
    }
    case BLEND_MODE::LIGHTEN:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MAX;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MAX;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        break;
    }
    case BLEND_MODE::DARKEN:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_MIN;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_MIN;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        break;
    }
    case BLEND_MODE::ADD:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        break;
    }
    case BLEND_MODE::SUBTRACT:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_REV_SUBTRACT;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_SUBTRACT;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ONE;
        break;
    }
    case BLEND_MODE::MULTIPLY:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_DEST_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        break;
    }
    case BLEND_MODE::REPLACE:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_ZERO;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
        break;
    }
    case BLEND_MODE::SCREEN:
    {
        bld.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
        bld.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;
        bld.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
        bld.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
        bld.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_INV_SRC_ALPHA;
        break;
    }
    default:
    {
        bld.RenderTarget[0].BlendEnable = false;
        break;
    }
    }
    bld.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;
    return dv->CreateBlendState(&bld, dxBlendState.GetAddressOf());
}