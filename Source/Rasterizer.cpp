#include "Rasterizer.h"
#include <assert.h>
RASTERIZER::RASTERIZER(ID3D11Device* dv, D3D11_RASTERIZER_DESC drd)
{
    assert(Initialize(dv, drd) == S_OK);
}

HRESULT RASTERIZER::Initialize(ID3D11Device* dv, D3D11_RASTERIZER_DESC drd)
{
    return dv->CreateRasterizerState(&drd, rasterizer.GetAddressOf());
}

ComPtr<ID3D11RasterizerState>RASTERIZER::Rasterizer()
{
    return rasterizer;
}

std::string RASTERIZER::Name()
{
    return name;
}

void RasterizerManager::Add(std::string name, ID3D11Device* dv, D3D11_RASTERIZER_DESC drd)
{
    std::shared_ptr<RASTERIZER>temp = std::make_shared<RASTERIZER>(dv, drd);
    rasterizers.insert(std::make_pair(name, temp));
}

std::shared_ptr<RASTERIZER>RasterizerManager::Retrieve(std::string name)
{
    return rasterizers.find(name)->second;
}

void RasterizerManager::Remove(std::string name)
{
    rasterizers.erase(name);
}

std::map<std::string, std::shared_ptr<RASTERIZER>>RasterizerManager::Rasterizers()
{
    return rasterizers;
}