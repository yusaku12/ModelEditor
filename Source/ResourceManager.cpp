#include "ResourceManager.h"
#include "Graphics/Model.h"

std::shared_ptr<FBXModel> ModelResourceManager::Load(std::string model_path)
{
    for (auto& a : ResourceMap)
    {
        if (model_path == a.first)
        {
            if (a.second.expired())
            {
                ResourceMap.erase(a.first);
                return Generate(model_path);
            }
            return a.second.lock();
        }
    }
    return Generate(model_path);
}

std::shared_ptr<FBXModel>ModelResourceManager::Generate(std::string model_path)
{
    std::shared_ptr<FBXModel>m = std::make_shared<FBXModel>(DirectX11::Instance()->Device(), model_path);
    if (!m)
        assert(!"Failed to find model from map");
    ResourceMap.insert(std::make_pair(model_path, m));
    return m ? m : nullptr;
}

HRESULT ModelResourceManager::Recreate(Model* m, std::string new_file_name)
{
    std::filesystem::path path{ new_file_name };
    path.replace_extension(".UEN");
    m->Resource()->Recreate(path.string());
    return S_OK;
}