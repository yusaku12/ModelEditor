#pragma once
#include <DirectXMath.h>
#include <d3d11.h>
#include <wrl.h>
#include <iostream>
#include <vector>
#include <map>
#include "Texture.h"
#include "BlendMode.h"
#include "Shader.h"

using namespace Microsoft::WRL;
using namespace DirectX;
class OBJECT
{
protected:

    std::map<std::wstring, std::shared_ptr<SHADERS>>shaders;
    int* indices{};
    int indexCount{}, vertexCount{};
    virtual void Render(ID3D11DeviceContext* dc) {};
    virtual ~OBJECT()
    {
        shaders.clear();
    }
public:
    /// <summary>
    /// Insert the shader to be used on the model
    /// </summary>
    /// <param name="shader_name"></param>
    /// <returns></returns>
    HRESULT InsertShader(std::wstring shader_name)
    {
        shaders.emplace(shader_name, ShaderManager::Instance()->Retrieve(shader_name));
        return shaders.find(shader_name)->second ? S_OK : E_INVALIDARG;
    }
    /// <summary>
    /// Removes the shader from the model
    /// </summary>
    /// <param name="shader_name"></param>
    /// <returns></returns>
    void RemoveShader(std::wstring shader_name)
    {
        shaders.erase(shader_name);
    }
    //HRESULT SetShaderAt(std::wstring shader_name, int index = 0)
    //{
    //    shaders[index] = ShaderManager::Instance()->Retrieve(shader_name);
    //}
};
