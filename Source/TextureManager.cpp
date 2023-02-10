#include "TextureManager.h"
#include "DirectX11.h"

/*--------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------TextureManager Class----------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------------TextureManager Insert()---------------------------------------------------------------*/

std::shared_ptr<TEXTURE>TextureManager::Insert(std::wstring file_path)
{
    if (file_path == L"Default_Diffuse")
        return GenerateEmpty();
    else if (file_path == L"Default_Normal")
        return GenerateNormal();

    std::shared_ptr<TEXTURE>entity;

    if (file_path == L"") {
        entity = std::make_shared<TEXTURE>(DirectX11::Instance()->Device());
        textureMap.emplace(file_path, entity);
    }
    entity = std::make_shared<TEXTURE>(file_path, DirectX11::Instance()->Device());
    textureMap.emplace(file_path, entity);

    return entity;
}

/*-------------------------------------------TextureManager GenerateEmpty()---------------------------------------------------------------*/

std::shared_ptr<TEXTURE>TextureManager::GenerateEmpty()
{
    std::shared_ptr<TEXTURE>entity;
    entity = std::make_shared<TEXTURE>(0xFFFFFFFF);
    textureMap.emplace(L"Default_Diffuse", entity);
    return entity;
}

/*-------------------------------------------TextureManager GenerateNormal()---------------------------------------------------------------*/

std::shared_ptr<TEXTURE>TextureManager::GenerateNormal()
{
    std::shared_ptr<TEXTURE>entity;
    entity = std::make_shared<TEXTURE>(0xFFFF7F7F);
    textureMap.emplace(L"Default_Normal", entity);
    return entity;
}

/*-------------------------------------------TextureManager Retrieve()---------------------------------------------------------------*/

std::shared_ptr<TEXTURE>TextureManager::Retrieve(std::wstring file_path)
{
    // Returns empty texture if path is blank
    if (file_path == L"")
        return Retrieve(L"Default_Diffuse");

    // Search the map for existing texture
    // マップに既存のテキスチャを検査
    for (auto& t : textureMap)
    {
        if (t.second.expired())
            textureMap.erase(file_path);
        if (file_path == t.first)
            return t.second.lock();
    }

    // Generate new texture if not found
    // 見つからない場合に新しいテキスチャを生成
    return Insert(file_path);
}