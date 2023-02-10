#pragma once

#include <cereal\types\memory.hpp>
#include <cereal\archives\binary.hpp>
#include <cereal\types\unordered_map.hpp>
#include <cereal/types/vector.hpp>
#include <DirectXMath.h>
#include <fbxsdk.h>
#include <filesystem>
#include <fstream>
#include <d3d11.h>
#include <wrl.h>
#include "Texture.h"
#include "OBJECT.h"

using namespace DirectX;
using namespace Microsoft::WRL;

static const int MAX_BONES{ 256 };
static const int BONE_INFL{ 4 };

namespace DirectX
{
    template <class T>
    void serialize(T& t, XMFLOAT2& f)
    {
        t(
            cereal::make_nvp("X", f.x),
            cereal::make_nvp("Y", f.y)
        );
    }
    template <class T>
    void serialize(T& t, XMFLOAT3& p)
    {
        t(
            cereal::make_nvp("X", p.x),
            cereal::make_nvp("Y", p.y),
            cereal::make_nvp("Z", p.z)
        );
    }
    template <class T>
    void serialize(T& t, XMFLOAT4& f)
    {
        t(cereal::make_nvp("X", f.x), cereal::make_nvp("Y", f.y), cereal::make_nvp("Z", f.z), cereal::make_nvp("W", f.w));
    }
    template<class T>
    void serialize(T& t, XMFLOAT4X4& f)
    {
        t(
            cereal::make_nvp("_11", f._11), cereal::make_nvp("_12", f._12), cereal::make_nvp("_13", f._13), cereal::make_nvp("14", f._14),
            cereal::make_nvp("_21", f._21), cereal::make_nvp("_22", f._22), cereal::make_nvp("_23", f._23), cereal::make_nvp("24", f._24),
            cereal::make_nvp("_31", f._31), cereal::make_nvp("_32", f._32), cereal::make_nvp("_33", f._33), cereal::make_nvp("34", f._34),
            cereal::make_nvp("_41", f._41), cereal::make_nvp("_42", f._42), cereal::make_nvp("_43", f._43), cereal::make_nvp("44", f._44));
    }
}

namespace Convert
{
    inline XMFLOAT4X4 Identity()
    {
        return { 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1 };
    }
    inline XMFLOAT4X4 ToFloat4x4(const FbxMatrix& m)
    {
        XMFLOAT4X4 f4;
        for (int y = 0; y < 4; ++y)
        {
            for (int x = 0; x < 4; ++x)
            {
                f4.m[y][x] = (float)m[y][x];
            }
        }
        return f4;
    }
    inline XMFLOAT3 ToFloat3(const FbxDouble3& d)
    {
        return { (float)d[0], (float)d[1], (float)d[2] };
    }
    inline XMFLOAT4 ToFloat4(const FbxDouble4& d)
    {
        return { (float)d[0], (float)d[1], (float)d[2], (float)d[3] };
    }
}

namespace SystemTransformation
{
    inline XMFLOAT4X4 RHS_Y_UP()
    {
        return XMFLOAT4X4
        {
            -1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
    }
    inline XMFLOAT4X4 RHS_Z_UP()
    {
        return XMFLOAT4X4
        {
            -1, 0,  0, 0,
             0, 0, -1, 0,
             0, 1,  0, 0,
             0, 0,  0, 1
        };
    }
    inline XMFLOAT4X4 LHS_Y_UP()
    {
        return XMFLOAT4X4
        {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1
        };
    }
    inline XMFLOAT4X4 LHS_Z_UP()
    {
        return XMFLOAT4X4
        {
            1, 0, 0, 0,
            0, 0, 1, 0,
            0, 1, 0, 0,
            0, 0, 0, 1
        };
    }
}

class FBXModel : public OBJECT
{
public:
    struct MESH_CONSTANT_BUFFER
    {
        XMFLOAT4X4 world;
        XMFLOAT4X4 b_Transform[MAX_BONES]{ Convert::Identity() };
        XMFLOAT4 colour;
    };
    struct SUBSET_CONSTANT_BUFFER
    {
        XMFLOAT4 colour;
    };
    struct OUTLINE_CONSTANT_BUFFER
    {
        XMFLOAT4 outline_color{};
        float outline_size{ 0.1f };
        XMFLOAT3 placeholder;
    };
    struct UVSCROLL_CONSTANT_BUFFER
    {
        XMFLOAT2 scrollVal;
        XMFLOAT2 placeholder;
    };
    enum class MATERIAL_TYPE
    {
        DIFFUSE,
        NORMAL,
        BUMP,
    };
#pragma region STRUCTS

#pragma region VERTEX
    struct VERTEX
    {
        XMFLOAT3 position, normal;
        XMFLOAT4 tangent;
        XMFLOAT2 UV;
        struct BONE
        {
            float Weights[BONE_INFL]{ 1, 0, 0, 0 };
            uint32_t Indices[BONE_INFL]{};
            template <class T>
            void serialize(T& t)
            {
                t(Weights, Indices);
            }
        }Bone_Properties;
        template <class T>
        void serialize(T& t)
        {
            t(position, normal, tangent, UV, Bone_Properties);
        }
    };
#pragma endregion
#pragma region SCENE
    struct SCENE
    {
        struct NODE
        {
            uint64_t UID{ 0 };
            std::string Name;
            FbxNodeAttribute::EType Attribute{ FbxNodeAttribute::EType::eUnknown };
            int64_t p_Index{ -1 };          // parent_index
            struct Transform {
                XMFLOAT4 scale;
                XMFLOAT4 rotation;
                XMFLOAT4 translation;

                template <class T>
                void serialize(T& t)
                {
                    t(scale, rotation, translation);
                }
            }transform;
            template<class T>
            void serialize(T& t)
            {
                t(UID, Name, p_Index, transform);
            }
        };
        std::vector<NODE>Nodes;
        template <class T>
        void serialize(T& t)
        {
            t(Nodes);
        }

        int64_t indexof(uint64_t unique_id)const
        {
            int64_t index{};
            for (const NODE& n : Nodes)
            {
                if (n.UID == unique_id)
                    return index;
                ++index;
            }
            return -1;
        }
    }Scenes;
#pragma endregion
#pragma region MATERIAL
    struct MATERIAL
    {
        // Unique ID
        uint64_t uid{};

        // Ambient Light
        XMFLOAT4 Ka{ 0.2f, 0.2f, 0.2f, 1.0f };

        // Diffuse Light
        XMFLOAT4 Kd{ 0.8f, 0.8f, 0.8f, 1.0f };

        // Full Reflection Light (Specular)
        XMFLOAT4 Ks{ 1.0f, 1.0f, 1.0f, 1.0f };

        std::string name;
        std::string texture_path[4];
        std::shared_ptr<TEXTURE>Textures[4];

        template<class T>
        void serialize(T& t)
        {
            t(uid, Ka, Kd, Ks, name, texture_path[0], texture_path[1], texture_path[2], texture_path[3]);
        }
    };
#pragma endregion
#pragma region AXISES
    struct AXISES
    {
        FbxAxisSystem::EFrontVector FrontAxis;
        FbxAxisSystem::EUpVector UpAxis;
        FbxAxisSystem::ECoordSystem AxisSystem;
        XMFLOAT4X4 AxisCoords;

        template<class T>
        void serialize(T& t)
        {
            t(AxisCoords);
        }

    }Axises;
#pragma endregion
#pragma region BONE_INFLEUNCE
    struct BONE_INFLUENCE
    {
        uint32_t b_Index; // Bone Index
        float b_Weight; // Bone Weight
        template <class T>
        void serialize(T& t)
        {
            t(b_Index, b_Weight);
        }
    };
#pragma endregion
#pragma region SKELETON
    struct SKELETON
    {
        struct BONE
        {
            uint64_t UID{};                                                                 // Unique ID
            int64_t p_Index{ -1 };                                                          // Parent Index (-1 is orphan)
            int64_t n_Index{};                                                              // Node Index

            std::string Name;                                                               // Name of bone
            XMFLOAT4X4 o_Transform{ 1, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0, 0,0 ,1 };        // Bone offset Transform

            template <class T>
            void serialize(T& t)
            {
                t(UID, p_Index, n_Index, Name, o_Transform);
            }
        };
        std::vector<BONE>Bones;

        int64_t indexof(uint64_t uid) const
        {
            int64_t ind{};
            for (auto& b : Bones)
            {
                if (uid == b.UID)
                    return ind;
                ++ind;
            }
            return -1;
        }
        template <class T>
        void serialize(T& t)
        {
            t(Bones);
        }
    };
#pragma endregion
#pragma region MESH
    struct MESH
    {
        XMFLOAT4X4 BaseTransform{};
        struct SUBSET
        {
            // Unique ID
            uint64_t m_UID{};
            // At which index does the subset starts
            uint32_t first_index{};
            // How many indices use this subset
            uint32_t i_Count{};

            // Material Name
            std::string m_Name{};

            std::vector<int> indices;
            ComPtr<ID3D11Buffer>subsetIndexBuffer;
            template<class T>
            void serialize(T& t)
            {
                t(m_UID, first_index, i_Count, m_Name, indices);
            }
        };
        XMFLOAT3 BOUNDING_BOX[2] = { {D3D11_FLOAT32_MAX, D3D11_FLOAT32_MAX , D3D11_FLOAT32_MAX }, {-D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX, -D3D11_FLOAT32_MAX} };

        ComPtr<ID3D11Buffer>dxVertexBuffer;
        ComPtr<ID3D11Buffer>dxIndexBuffer;
        // Vertex Buffer Desc and Index Buffer Desc
        D3D11_BUFFER_DESC vbd{}, ibd{};

        // Subresource Data for Vertex and Index Buffer
        D3D11_SUBRESOURCE_DATA vd{}, id{};

        int index_Count{};
        int64_t UID{}, n_Index{};
        std::string Name;
        std::vector<VERTEX>Vertices;
        std::vector<int>Indices;
        std::vector<SUBSET>Subsets;
        SKELETON Bind_Pose;

        template <class T>
        void serialize(T& t)
        {
            t(BaseTransform, index_Count, UID, n_Index, Name, Vertices, Indices, Subsets, Bind_Pose, BOUNDING_BOX);
        }
    };
#pragma endregion
#pragma region ANIMATIONS

    struct ANIMATION
    {
        float SamplingRate{};
        std::string Name{ "" };
#pragma region KEYFRAME

        struct KEYFRAME
        {
#pragma region NODE
            struct NODE
            {
                XMFLOAT4X4 g_Transform{ Convert::Identity() };          // Global Transform
                XMFLOAT3 Scaling{ 1, 1, 1 };                            // Scale
                XMFLOAT4 Rotation{ 0, 0, 0, 1 };                        // Quaternion rotation
                XMFLOAT3 Translation{ 0, 0, 0 };

                template <class T>
                void serialize(T& t)
                {
                    t(g_Transform, Scaling, Rotation, Translation);
                }
            };
#pragma endregion
            std::vector<KEYFRAME::NODE>Nodes;
            template <class T>
            void serialize(T& t)
            {
                t(Nodes);
            }
        };

#pragma endregion
        std::vector<KEYFRAME>Keyframes;
        template <class T>
        void serialize(T& t)
        {
            t(SamplingRate, Name, Keyframes);
        }
    };

#pragma endregion

#pragma endregion

    ComPtr<ID3D11Buffer>meshConstantBuffer;
    ComPtr<ID3D11Buffer>subsetConstantBuffer;
    ComPtr<ID3D11Buffer>outlineConstantBuffer;
    std::vector<MESH>Meshes;
    std::vector<ANIMATION>Animations;
    std::unordered_map<uint64_t, MATERIAL>Materials;

#pragma region PROPERTY RETRIEVAL FUNCTION HEADERS
    void RetrieveMeshes(FbxScene* scene);
    void RetrieveMaterials(FbxScene* scene);
    void RetrieveInfleunces(FbxMesh* m, std::vector<std::vector<BONE_INFLUENCE>>& bi);
    void RetrieveSkeleton(FbxMesh* m, SKELETON& s);
    void RetrieveAnimations(FbxScene* s, float samplingRate = 0);
#pragma endregion

    FBXModel(ID3D11Device* dv, std::string model_path, bool Triangulate = true);
    void CreateBuffers(ID3D11Device* dv, const char* model_path);
    void UpdateAnimation(ANIMATION::KEYFRAME* kf)
    {
        size_t n_Count{ kf->Nodes.size() };
        for (size_t n_Index = 0; n_Index < n_Count; ++n_Index)
        {
            ANIMATION::KEYFRAME::NODE& n{ kf->Nodes.at(n_Index) };
            XMMATRIX S{ XMMatrixScaling(n.Scaling.x, n.Scaling.y, n.Scaling.z) };
            XMMATRIX R{ XMMatrixRotationQuaternion(XMLoadFloat4(&n.Rotation)) };
            XMMATRIX T{ XMMatrixTranslation(n.Translation.x, n.Translation.y, n.Translation.z) };

            int64_t p_Index{ Scenes.Nodes.at(n_Index).p_Index };
            XMMATRIX P{ p_Index >= 0 ? XMLoadFloat4x4(&kf->Nodes.at(p_Index).g_Transform) : XMMatrixIdentity() };

            XMStoreFloat4x4(&n.g_Transform, S * R * T * P);
        }
    }
    void BlendAnimation(ANIMATION::KEYFRAME* start, ANIMATION::KEYFRAME* end, float factor, ANIMATION::KEYFRAME* output);
    void Render(ID3D11DeviceContext* dc, XMFLOAT4X4 world, XMFLOAT4 colour, const ANIMATION::KEYFRAME* kf);

    template <class T>
    void serialize(T& t)
    {
        t(Scenes, Axises, Meshes, Materials, Animations);
    }

    bool AddAnimation(std::string anim_path, std::string animationName = "", float SamplingRate = 0);
    HRESULT AddMaterial(std::string mat_path, std::string model_name, MATERIAL_TYPE type, MATERIAL& m);
    void Recreate(std::string file_path)
    {
        std::filesystem::path path(file_path);
        if (std::filesystem::exists(file_path))
        {
            std::filesystem::remove(file_path);
        }
        std::ofstream ofs(path, std::ios::binary);
        cereal::BinaryOutputArchive boa(ofs);
        boa(Scenes, Axises, Meshes, Materials, Animations);
    }
    void ResetDefaultTransform(MESH* m);
};