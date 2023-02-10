#pragma once

#include <d3d11.h>
#include <wrl.h>
#include <map>
#include <string>
#include <memory>
#include "Singleton.h"

using namespace Microsoft::WRL;

class RASTERIZER
{
    std::string name;
    ComPtr<ID3D11RasterizerState>rasterizer;
public:

    RASTERIZER(ID3D11Device* dv, D3D11_RASTERIZER_DESC drd);
    HRESULT Initialize(ID3D11Device* dv, D3D11_RASTERIZER_DESC drd);
    ComPtr<ID3D11RasterizerState>Rasterizer();
    std::string Name();
};

class RasterizerManager : public Singleton<RasterizerManager>
{
    std::map < std::string, std::shared_ptr<RASTERIZER>>rasterizers;

public:
    void Add(std::string name, ID3D11Device* dv, D3D11_RASTERIZER_DESC drd);
    void Remove(std::string name);

    std::shared_ptr<RASTERIZER> Retrieve(std::string name);
    std::map < std::string, std::shared_ptr<RASTERIZER>>Rasterizers();
};