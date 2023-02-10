#include "Shader.h"

SHADERS::SHADERS(std::wstring shader_path, ID3D11Device* dv, D3D11_INPUT_ELEMENT_DESC* ied, UINT ied_count)
{
    HRESULT hr{ S_OK };
    ID3D10Blob* vs{}, * ps{}, * error{};
    DWORD shaderFlags = D3DCOMPILE_ENABLE_STRICTNESS;
#ifdef _DEBUG
    shaderFlags |= D3DCOMPILE_DEBUG;
#endif

    hr = D3DCompileFromFile(shader_path.c_str(), NULL, NULL, "VS_MAIN", "vs_5_0", shaderFlags, NULL, &vs, &error);
    if (FAILED(hr))
    {
        std::string er = (const char*)error->GetBufferPointer();
        assert(!er.c_str());
    }
    hr = D3DCompileFromFile(shader_path.c_str(), NULL, NULL, "PS_MAIN", "ps_5_0", shaderFlags, NULL, &ps, &error);
    if (FAILED(hr))
    {
        std::string er = (const char*)error->GetBufferPointer();
        assert(!er.c_str());
    }
    dv->CreateVertexShader(vs->GetBufferPointer(), vs->GetBufferSize(), nullptr, dxVertexShader.GetAddressOf());
    if (FAILED(hr))
    {
        assert(!"Failed to create Vertex Shader");
    }
    dv->CreatePixelShader(ps->GetBufferPointer(), ps->GetBufferSize(), nullptr, dxPixelShader.GetAddressOf());
    if (FAILED(hr))
    {
        assert(!"Failed to create Pixel Shader");
    }
    hr = dv->CreateInputLayout(ied, ied_count, vs->GetBufferPointer(), vs->GetBufferSize(), dxInputLayout.GetAddressOf());
    if (FAILED(hr))
    {
        assert(!"Failed to create Input Layout");
    }
    vs->Release();
    ps->Release();

    D3D11_SAMPLER_DESC dsd{};
    dsd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
    dsd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
    dsd.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
    dsd.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    dsd.MipLODBias = 0.0f;
    dsd.MaxAnisotropy = 1;
    dsd.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
    dsd.MinLOD = 0;
    dsd.MaxLOD = D3D11_FLOAT32_MAX;
    dsd.BorderColor[0] = 0;
    dsd.BorderColor[1] = 0;
    dsd.BorderColor[2] = 0;
    dsd.BorderColor[3] = 0;
    hr = dv->CreateSamplerState(&dsd, dxSamplerState.GetAddressOf());
    if (FAILED(hr))
        assert(!"Failed to create sampler state for Texture Shader Class");
}

HRESULT ShaderManager::Insert(std::wstring shader_path, D3D11_INPUT_ELEMENT_DESC* ied, UINT ied_count)
{
    std::shared_ptr<SHADERS>s = std::make_shared<SHADERS>(shader_path, DirectX11::Instance()->Device(), ied, ied_count);
    std::filesystem::path path(shader_path);
    path = path.filename();
    std::wstring name = path.wstring();
    if (!s)
        assert(!"Failed to create Shader");
    shaders.insert({ name, s });
    return s ? S_OK : E_FAIL;
}
HRESULT ShaderManager::Initialize()
{
    D3D11_INPUT_ELEMENT_DESC ied[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV",          0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"B_WEIGHTS",   0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"B_INDICES",   0, DXGI_FORMAT_R32G32B32A32_UINT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    int ied_Size{ ARRAYSIZE(ied) };

    D3D11_INPUT_ELEMENT_DESC ied_2d[] =
    {
        {"SV_POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"COLOR",          0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV",             0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    int ied_2d_size{ ARRAYSIZE(ied_2d) };

    D3D11_INPUT_ELEMENT_DESC ied_p[] =
    {
        {"POSITION",    0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"NORMAL",      0, DXGI_FORMAT_R32G32B32_FLOAT,     0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"TANGENT",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"UV",          0, DXGI_FORMAT_R32G32_FLOAT,        0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"WEIGHTS",     0, DXGI_FORMAT_R32G32B32A32_FLOAT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
        {"BONES",     0, DXGI_FORMAT_R32G32B32A32_UINT,   0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0},
    };
    int ied_p_size{ ARRAYSIZE(ied_p) };
    D3D11_INPUT_ELEMENT_DESC ied_d[] =
    {
        {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0}
    };
    int ied_d_size{ ARRAYSIZE(ied_d) };

    Insert(L"HLSL/Lambert.fx", ied, ied_Size);
    Insert(L"HLSL/Phong.fx", ied_p, ied_p_size);
    Insert(L"HLSL/Outline.fx", ied_p, ied_p_size);

    return S_OK;
}
void ShaderManager::Clear()
{
    shaders.clear();
}
std::shared_ptr<SHADERS>ShaderManager::Retrieve(std::wstring name)
{
    for (auto& s : shaders) {
        if (name == s.first)
            return s.second;
    }

    assert(!"No Shader Found!");
    return 0;
}
std::map<std::wstring, std::shared_ptr<SHADERS>>ShaderManager::Shaders()
{
    return shaders;
}