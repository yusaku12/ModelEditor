#pragma once

#include "Singleton.h"
#include <string>
#include <fstream>
#include <memory>
#include "DirectX11.h"
#include <map>
#include "Graphics/FBXModel.h"

class Model;

class ModelResourceManager : public Singleton<ModelResourceManager>
{
public:
    std::map<std::string, std::weak_ptr<FBXModel>>ResourceMap;
    /// <summary>
    /// Resource Loader. Loads the resource from the existing map (Identified by file path) or creates and inserts a new Model Resource into the ResourceMap
    /// </summary>
    /// <returns></returns>
    std::shared_ptr<FBXModel>Load(std::string model_path);

    std::shared_ptr<FBXModel>Generate(std::string model_path);
    /// <summary>
    /// Recreates the Model file
    /// </summary>
    /// <param name="m"> : Model to recreate</param>
    /// <param name="new_file_name"> : New File name</param>
    /// <returns></returns>
    HRESULT Recreate(Model* m, std::string new_file_name);

    void Finalize()
    {
    }

    ~ModelResourceManager()
    {
    }
};