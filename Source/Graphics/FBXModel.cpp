#include "FBXModel.h"
#include <functional>
#include <sstream>
#include <filesystem>
#include "BlendMode.h"
#include "Rasterizer.h"
#include "DirectX11.h"
#include "TextureManager.h"
#include "Shader.h"

std::wstring texture_names[] = { L"Default_Diffuse", L"Default_Normal", L"Default_Diffuse" , L"Default_Diffuse" };

/*------------------------------------------------------------------------------------------------------------------*/
/*----------------------------------------MODEL_RESOURCES class-----------------------------------------------------*/
/*------------------------------------------------------------------------------------------------------------------*/
/*-------------------------------------- - MODEL_RESOURCES Constructor----------------------------------------------*/

FBXModel::FBXModel(ID3D11Device* dv, std::string model_path, bool Triangulate)
{
#pragma region CHECK IF SERIALIZED FILE EXISTS

    std::filesystem::path path(model_path);

    if (path.extension() == ".mdl")
    {
        std::ifstream ifs(path, std::ios::binary);
        cereal::BinaryInputArchive in(ifs);
        in(Scenes, Axises, Meshes, Materials, Animations);
    }
    else
    {
#pragma endregion
#pragma region CREATE SERIALIZED FILE IF DOES NOT EXIST

        FbxManager* manager{ FbxManager::Create() };
        FbxImporter* importer{ FbxImporter::Create(manager, "") };
        FbxScene* scene{ FbxScene::Create(manager, "Scene") };

        bool import_status{ importer->Initialize(model_path.c_str()) };
        if (!import_status)
            assert(!"Failed to import model");

        import_status = importer->Import(scene);
        if (!import_status)
            assert(!"Failed to import Scene");

        FbxGeometryConverter converter(manager);
        if (Triangulate)
        {
            converter.Triangulate(scene, true, false);
            converter.RemoveBadPolygonsFromMeshes(scene);
        }

        FbxAxisSystem axis{ scene->GetGlobalSettings().GetAxisSystem() };
        int up, front;
        Axises.AxisSystem = axis.GetCoorSystem();
        Axises.UpAxis = axis.GetUpVector(up);
        Axises.FrontAxis = axis.GetFrontVector(front);
        if (Axises.AxisSystem == FbxAxisSystem::ECoordSystem::eRightHanded && Axises.UpAxis == FbxAxisSystem::EUpVector::eYAxis)Axises.AxisCoords = SystemTransformation::RHS_Y_UP();
        if (Axises.AxisSystem == FbxAxisSystem::ECoordSystem::eLeftHanded && Axises.UpAxis == FbxAxisSystem::EUpVector::eYAxis)Axises.AxisCoords = SystemTransformation::LHS_Y_UP();
        if (Axises.AxisSystem == FbxAxisSystem::ECoordSystem::eRightHanded && Axises.UpAxis == FbxAxisSystem::EUpVector::eZAxis)Axises.AxisCoords = SystemTransformation::RHS_Z_UP();
        if (Axises.AxisSystem == FbxAxisSystem::ECoordSystem::eLeftHanded && Axises.UpAxis == FbxAxisSystem::EUpVector::eZAxis)Axises.AxisCoords = SystemTransformation::LHS_Z_UP();

        std::function<void(FbxNode*)>Traverse
        {
        [&](FbxNode* cNode)
            {
                SCENE::NODE node;
                node.Attribute = cNode->GetNodeAttribute() ? cNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown;
                node.Name = cNode->GetName();
                node.UID = cNode->GetUniqueID();
                node.p_Index = Scenes.indexof(cNode->GetParent() ? cNode->GetParent()->GetUniqueID() : 0);
                node.transform.scale = Convert::ToFloat4(cNode->EvaluateLocalScaling());
                node.transform.rotation = Convert::ToFloat4(cNode->EvaluateLocalRotation());
                node.transform.translation = Convert::ToFloat4(cNode->EvaluateLocalTranslation());
                Scenes.Nodes.emplace_back(node);

                for (int cIndex = 0; cIndex < cNode->GetChildCount(); ++cIndex)
                {
                    Traverse(cNode->GetChild(cIndex));
                }
            }
        };
        Traverse(scene->GetRootNode());
#ifdef _DEBUG
        for (const SCENE::NODE& n : Scenes.Nodes)
        {
            FbxNode* fNode{ scene->FindNodeByName(n.Name.c_str()) };
            // Display node data in the output window as debug
            std::string name{ fNode->GetName() };
            uint64_t uid{ fNode->GetUniqueID() };
            uint64_t p_uid{ fNode->GetParent() ? fNode->GetParent()->GetUniqueID() : 0 };
            int32_t type{ fNode->GetNodeAttribute() ? fNode->GetNodeAttribute()->GetAttributeType() : FbxNodeAttribute::EType::eUnknown };
            std::stringstream debug_s;
            debug_s << name << " : " << uid << " : " << p_uid << " : " << type << '\n';
            OutputDebugStringA(debug_s.str().c_str());
        }
#endif
        RetrieveMeshes(scene);
        RetrieveMaterials(scene);
        RetrieveAnimations(scene);
        path.replace_extension(".mdl");
        if (std::filesystem::exists(path))
            std::filesystem::remove(path);
        std::ofstream ofs(path, std::ios::binary);
        cereal::BinaryOutputArchive boa(ofs);
        boa(Scenes, Axises, Meshes, Materials, Animations);
        manager->Destroy();
    }
#pragma endregion
    CreateBuffers(dv, model_path.c_str());
    InsertShader(L"Phong.fx");
}

/*-------------------------------------- - MODEL_RESOURCES RetrieveMeshes()----------------------------------------------*/

void FBXModel::RetrieveMeshes(FbxScene* scene)
{
    for (auto& n : Scenes.Nodes)
    {
        FbxNode* cur_node{ scene->FindNodeByName(n.Name.c_str()) };
        if (n.Attribute != FbxNodeAttribute::EType::eMesh)continue;

        FbxNode* fNode{ scene->FindNodeByName(n.Name.c_str()) };
        FbxMesh* fMesh{ fNode->GetMesh() };
        if (!fMesh)continue;

        // For inserting into Meshes vector
        MESH m;
        m.UID = fNode->GetUniqueID();
        const char* test = fMesh->GetNode()->GetName();
        m.Name = fMesh->GetNode()->GetName();
        m.n_Index = Scenes.indexof(fMesh->GetNode()->GetUniqueID());
        XMMATRIX parent_matrix{ XMMatrixIdentity() };

        int64_t parent_index{ n.p_Index };
        while (parent_index > -1) {
            SCENE::NODE parentNode{ Scenes.Nodes.at(parent_index) };

            FbxNode* parent{ scene->FindNodeByName(parentNode.Name.c_str()) };
            XMFLOAT4X4 p{ Convert::ToFloat4x4(parent->EvaluateLocalTransform()) };
            XMMATRIX local_tran{ XMLoadFloat4x4(&p) };
            parent_matrix *= local_tran;
            parent_index = parentNode.p_Index;
        }

        XMFLOAT4 geo_s, geo_r, geo_t;
        geo_s = { Convert::ToFloat4(fNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot)) };
        geo_r = { Convert::ToFloat4(fNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot)) };
        geo_t = { Convert::ToFloat4(fNode->GetGeometricScaling(FbxNode::EPivotSet::eSourcePivot)) };

        float g_x, g_y, g_z;
        g_x = XMConvertToRadians(geo_r.x);
        g_y = XMConvertToRadians(geo_r.y);
        g_z = XMConvertToRadians(geo_r.z);

        geo_r = { g_x, g_y, g_z, geo_r.w };

        XMMATRIX geometric = { XMMatrixScalingFromVector(XMLoadFloat4(&geo_s)) * XMMatrixRotationRollPitchYawFromVector(XMLoadFloat4(&geo_r)) * XMMatrixTranslationFromVector(XMLoadFloat4(&geo_t)) };

        XMVECTOR s{ XMLoadFloat4(&n.transform.scale) };
        float x, y, z;
        x = XMConvertToRadians(n.transform.rotation.x);
        y = XMConvertToRadians(n.transform.rotation.y);
        z = XMConvertToRadians(n.transform.rotation.z);

        XMFLOAT3 K = { x,y,z };
        XMVECTOR r{ XMLoadFloat3(&K) };
        XMVECTOR t{ XMLoadFloat4(&n.transform.translation) };
        XMMATRIX local{ XMMatrixScalingFromVector(s) * XMMatrixRotationRollPitchYawFromVector(r) * XMMatrixTranslationFromVector(t) };

        XMStoreFloat4x4(&m.BaseTransform, parent_matrix * local * geometric);

        for (int ind = 0; ind < scene->GetNodeCount(); ++ind)
        {
            FbxNode* n{ scene->GetNode(ind) };
            UINT uid = n->GetUniqueID();
            std::string name{ n->GetNameOnly() };
            if (n->GetUniqueID() == m.UID)m.Name = n->GetName();
        }

        std::vector<std::vector<BONE_INFLUENCE>>Influences;
        RetrieveInfleunces(fMesh, Influences);
        RetrieveSkeleton(fMesh, m.Bind_Pose);

        // Polygon count (Referenced from Fbx Mesh)   Material Count (Referenced from FbxNode)
        const int p_Count{ fMesh->GetPolygonCount() }, m_Count{ fNode->GetMaterialCount() };
        m.Subsets.resize(m_Count ? m_Count : 1);
        for (int ind = 0; ind < m_Count; ++ind)
        {
            const FbxSurfaceMaterial* fMaterial{ fNode->GetMaterial(ind) };
            m.Subsets.at(ind).m_Name = fMaterial->GetName();
            m.Subsets.at(ind).m_UID = fMaterial->GetUniqueID();
        }

        if (m_Count)
        {
            for (int ind = 0; ind < p_Count; ++ind)
            {
                const int m_Index{ fMesh->GetElementMaterial()->GetIndexArray().GetAt(ind) };
                m.Subsets.at(m_Index).i_Count += 3;
            }

            uint32_t offset{};
            for (auto& s : m.Subsets)
            {
                s.first_index = offset;
                offset += s.i_Count;
                s.i_Count = 0;
            }
        }

        std::vector<VERTEX>temp;
        temp.resize(p_Count * 3LL);
        m.Indices.resize(p_Count * 3LL);

        std::vector<int> submeshIndex;

        submeshIndex.resize(p_Count * 3LL);

        FbxStringList uv_names;
        fMesh->GetUVSetNames(uv_names);
        // Control points of the mesh (Vertex Information is retrieve here)
        const FbxVector4* cPoints{ fMesh->GetControlPoints() };
        for (int ind = 0; ind < p_Count; ++ind)
        {
            // Material Index
            const int m_Index{ m_Count ? fMesh->GetElementMaterial()->GetIndexArray().GetAt(ind) : 0 };
            MESH::SUBSET& s{ m.Subsets.at(m_Index) };

            const uint32_t offset{ s.first_index + s.i_Count };
            FbxAMatrix localTransform{ fMesh->GetNode()->EvaluateLocalTransform() };
            XMFLOAT4X4 LocalTransform{ Convert::ToFloat4x4(localTransform) };
            XMMATRIX LOCALTRANSFORM{ XMLoadFloat4x4(&LocalTransform) };
            for (int p_Pos = 0; p_Pos < 3; ++p_Pos)
            {
                // Which vertex to be inserted
                const int v_Index{ ind * 3 + p_Pos };

                // For inserting into Vertices vector
                VERTEX v;
                VERTEX& vt = temp.at(v_Index);
                const int p_Vertex{ fMesh->GetPolygonVertex(ind, p_Pos) };

                // Position Retrieval 
                XMVECTOR POS{ XMVectorSet((float)cPoints[p_Vertex][0], (float)cPoints[p_Vertex][1], (float)cPoints[p_Vertex][2], 0) };
                POS = XMVector3TransformCoord(POS, LOCALTRANSFORM);

                XMStoreFloat3(&v.position, POS);

                // Inserting bone properties into vertices
                const std::vector<BONE_INFLUENCE>& bfs{ Influences.at(p_Vertex) };
                for (size_t if_Index = 0; if_Index < bfs.size(); ++if_Index)
                {
                    if (if_Index < BONE_INFL)
                    {
                        v.Bone_Properties.Weights[if_Index] = bfs.at(if_Index).b_Weight;
                        v.Bone_Properties.Indices[if_Index] = bfs.at(if_Index).b_Index;
                    }
                }
                // Normal Retrieval
                if (fMesh->GetElementNormalCount() > 0)
                {
                    FbxVector4 n_Pos;
                    fMesh->GetPolygonVertexNormal(ind, p_Pos, n_Pos);
                    v.normal.x = (float)n_Pos[0];
                    v.normal.y = (float)n_Pos[1];
                    v.normal.z = (float)n_Pos[2];
                }

                // Tangent Retrieval
                if (fMesh->GenerateTangentsData(0, false))
                {
                    FbxGeometryElementTangent* t{ fMesh->GetElementTangent() };

                    int cur_Index = t->GetIndexArray().GetAt(v_Index);

                    v.tangent.x = (float)(t->GetDirectArray().GetAt(cur_Index)[0]);
                    v.tangent.y = (float)(t->GetDirectArray().GetAt(cur_Index)[1]);
                    v.tangent.z = (float)(t->GetDirectArray().GetAt(cur_Index)[2]);
                    v.tangent.w = (float)(t->GetDirectArray().GetAt(cur_Index)[3]);
                }
                else
                    v.tangent = { 0, 1, 0, 1 };

                // UV Retrieval
                if (fMesh->GetElementUVCount() > 0)
                {
                    FbxVector2 uv;
                    bool unmapped_uv;

                    fMesh->GetPolygonVertexUV(ind, p_Pos, uv_names[0], uv, unmapped_uv);
                    v.UV.x = (float)uv[0];
                    v.UV.y = 1 - (float)uv[1];
                    if (v.UV.x > 1.0f || v.UV.y > 1.0f)int a = 0;
                }
                vt = v;
                bool exists{};
                int index{};
                for (auto& vt : m.Vertices)
                {
                    bool p_same{}, n_same{}, t_same{}, u_same{}, i_same{}, w_same{};
                    if (v.position.x == vt.position.x && v.position.y == vt.position.y && v.position.z == vt.position.z)p_same = true;
                    if (v.normal.x == vt.normal.x && v.normal.y == vt.normal.y && v.normal.z == vt.normal.z)n_same = true;
                    if (v.tangent.x == vt.tangent.x && v.tangent.y == vt.tangent.y && v.tangent.z == vt.tangent.z)t_same = true;
                    if (v.UV.x == vt.UV.x && v.UV.y == vt.UV.y)u_same = true;

                    if (p_same && n_same && t_same && u_same)
                    {
                        exists = true;
                        break;
                    }
                    ++index;
                }

                if (!exists)
                {
                    m.Vertices.push_back(v);
                    s.indices.push_back(m.Vertices.size() - 1);
                }
                else
                {
                    s.indices.push_back(index);
                }
                s.i_Count++;
            }
        }
        Meshes.emplace_back(m);
    }
}

/*-------------------------------------- - MODEL_RESOURCES RetrieveMaterials()----------------------------------------------*/

void FBXModel::RetrieveMaterials(FbxScene* scene)
{
    // Node Count
    const size_t nCount{ Scenes.Nodes.size() };
    for (int ind = 0; ind < nCount; ++ind)
    {
        const SCENE::NODE n{ Scenes.Nodes.at(ind) };
        const FbxNode* fNode{ scene->FindNodeByName(n.Name.c_str()) };
        // Material Count (Referenced from FbxNode)
        const int m_Count{ fNode->GetMaterialCount() };
        MATERIAL m;

        if (m_Count)
        {
            for (int m_Index = 0; m_Index < m_Count; ++m_Index)
            {
                const FbxSurfaceMaterial* fSurfaceMaterial{ fNode->GetMaterial(m_Index) };
                m.name = fSurfaceMaterial->GetName();
                m.uid = fSurfaceMaterial->GetUniqueID();
                FbxProperty fProperty{ fSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sDiffuse) };
                if (!fProperty.IsValid())continue;
                FbxDouble3 colours{ fProperty.Get<FbxDouble3>() };
                m.Kd.x = colours[0];
                m.Kd.y = colours[1];
                m.Kd.z = colours[2];
                m.Kd.w = 1.0f;

                const FbxFileTexture* texture{ fProperty.GetSrcObject<FbxFileTexture>() };
                m.texture_path[0] = texture ? texture->GetRelativeFileName() : "";
                if (!m_Count)m.texture_path[0] = "";

                // Normal Map Retrieval
                fProperty = fSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sNormalMap);
                bool prop = fProperty.IsValid();
                if (fProperty.IsValid())
                {
                    int count = fProperty.GetSrcObjectCount<FbxLayeredTexture>();
                    texture = fProperty.GetSrcObject<FbxFileTexture>();
                    m.texture_path[1] = texture ? texture->GetRelativeFileName() : "";
                }

                fProperty = fSurfaceMaterial->FindProperty(FbxSurfaceMaterial::sBump);
                if (fProperty.IsValid())
                {
                    int count = fProperty.GetSrcObjectCount<FbxLayeredTexture>();

                    texture = fProperty.GetSrcObject<FbxFileTexture>();
                }
                Materials.emplace(m.uid, std::move(m));
            }
        }
        if (!m_Count)Materials.emplace(m.uid, std::move(m));
    }
}

/*-------------------------------------- - MODEL_RESOURCES RetrieveInfluences()----------------------------------------------*/

void FBXModel::RetrieveInfleunces(FbxMesh* m, std::vector<std::vector<BONE_INFLUENCE>>& bi)
{
    const int cp_Count{ m->GetControlPointsCount() };      // Control Point Count
    const int s_Count{ m->GetDeformerCount(FbxDeformer::eSkin) };

    bi.resize(cp_Count);
    if (!s_Count)
    {
        for (auto& a : bi)
        {
            BONE_INFLUENCE& bf{ a.emplace_back() };
            bf.b_Index = 0;
            bf.b_Weight = 1;
        }
    }
    for (int s_Index = 0; s_Index < s_Count; ++s_Index)
    {
        FbxSkin* f_Skin{ (FbxSkin*)m->GetDeformer(s_Index, FbxDeformer::eSkin) };
        int c_Count{ f_Skin->GetClusterCount() };       // Cluster Count
        for (int c_Index = 0; c_Index < c_Count; ++c_Index)
        {
            FbxCluster* f_Cluster{ f_Skin->GetCluster(c_Index) };
            int cpi_Count{ f_Cluster->GetControlPointIndicesCount() };
            for (int cpi_Index = 0; cpi_Index < cpi_Count; ++cpi_Index)
            {
                int cp_Index{ f_Cluster->GetControlPointIndices()[cpi_Index] };
                double cp_Weight{ f_Cluster->GetControlPointWeights()[cpi_Index] };
                BONE_INFLUENCE& bf(bi.at(cp_Index).emplace_back());
                bf.b_Index = (uint32_t)c_Index;
                bf.b_Weight = (float)cp_Weight;
            }
        }
    }
}

/*-------------------------------------- - MODEL_RESOURCES RetrieveSkeleton()----------------------------------------------*/

void FBXModel::RetrieveSkeleton(FbxMesh* m, SKELETON& s)
{
    const int d_Count{ m->GetDeformerCount(FbxDeformer::eSkin) };       // Deformer Count (Retrieved from FbxMesh)

    FbxAMatrix fm = m->GetNode()->EvaluateGlobalTransform();

    for (int d_Index = 0; d_Index < d_Count; ++d_Index)
    {
        FbxSkin* f_Skin{ (FbxSkin*)m->GetDeformer(d_Index, FbxDeformer::eSkin) };
        const int c_Count{ f_Skin->GetClusterCount() };         // Cluster Count (Retrieved from FbxSkin)
        s.Bones.resize(c_Count);
        int c_Index{};
        FbxCluster* cluster = f_Skin->GetCluster(52);
        for (auto& b : s.Bones)
        {
            FbxCluster* f_Cluster{ f_Skin->GetCluster(c_Index) };

            // Parameter retrieval of Bone
            b.Name = f_Cluster->GetLink()->GetName();
            b.UID = f_Cluster->GetLink()->GetUniqueID();
            b.p_Index = s.indexof(f_Cluster->GetLink()->GetParent()->GetUniqueID());
            b.n_Index = Scenes.indexof(b.UID);

            FbxAMatrix rgi_Position;        // Reference Global Initiated Position
            FbxAMatrix cgi_Position;        // Cluster Global Initiated Position;
            f_Cluster->GetTransformLinkMatrix(cgi_Position);
            f_Cluster->GetTransformMatrix(rgi_Position);

            b.o_Transform = Convert::ToFloat4x4(cgi_Position.Inverse() * rgi_Position * fm.Inverse());
            ++c_Index;
        }
    }
    if (!d_Count)
    {
        SKELETON::BONE& b{ s.Bones.emplace_back() };
        b.Name = "NO_BONE";
    }
}

/*-------------------------------------- - MODEL_RESOURCES RetrieveAnimations()----------------------------------------------*/

void FBXModel::RetrieveAnimations(FbxScene* s, float samplingRate)
{
    FbxArray<FbxString*>a_StackNames;       // Animations stack names
    s->FillAnimStackNameArray(a_StackNames);
    const int a_StackCount{ a_StackNames.GetCount() };      // Animation Stack Count
    for (int a_StackIndex = 0; a_StackIndex < a_StackCount; ++a_StackIndex)
    {
        ANIMATION a;
        a.Name = a_StackNames.GetAt(a_StackIndex)->Buffer();

        FbxAnimStack* a_Stack{ s->FindMember<FbxAnimStack>(a.Name.c_str()) };
        s->SetCurrentAnimationStack(a_Stack);

        const FbxTime::EMode Mode{ s->GetGlobalSettings().GetTimeMode() };      // Time Mode
        FbxTime one_sec;
        one_sec.SetTime(0, 0, 1, 0, 0, Mode);
        a.SamplingRate = samplingRate > 0 ? samplingRate : (float)one_sec.GetFrameRate(Mode);   // Sampling Rate settings
        const FbxTime samplingInterval{ (FbxLongLong)(one_sec.Get() / a.SamplingRate) };          // How many time sampling happens during a second
        const FbxTakeInfo* t_Info{ s->GetTakeInfo(a.Name.c_str()) };
        const FbxTime Start{ t_Info->mLocalTimeSpan.GetStart() };
        const FbxTime Stop{ t_Info->mLocalTimeSpan.GetStop() };
        for (FbxTime t = Start; t < Stop; t += samplingInterval)
        {
            ANIMATION::KEYFRAME& kf{ a.Keyframes.emplace_back() };
            size_t n_Count{ Scenes.Nodes.size() };
            kf.Nodes.resize(n_Count);

            for (size_t n_Index = 0; n_Index < n_Count; ++n_Index)
            {
                FbxNode* n{ s->FindNodeByName(Scenes.Nodes.at(n_Index).Name.c_str()) };
                if (!n)continue;
                ANIMATION::KEYFRAME::NODE& node{ kf.Nodes.at(n_Index) };
                node.g_Transform = Convert::ToFloat4x4(n->EvaluateGlobalTransform(t));
                FbxAMatrix& l_Transform{ n->EvaluateLocalTransform(t) };
                node.Scaling = Convert::ToFloat3(l_Transform.GetS());
                node.Rotation = Convert::ToFloat4(l_Transform.GetQ());
                node.Translation = Convert::ToFloat3(l_Transform.GetT());
            }
        }
        Animations.push_back(a);
    }
    if (!a_StackCount)
    {
        size_t n_Count = Scenes.Nodes.size();
        ANIMATION a;
        a.Name = "DEFAULT";
        a.SamplingRate = 60;
        ANIMATION::KEYFRAME kf;
        kf.Nodes.resize(n_Count);
        for (size_t n_Index = 0; n_Index < n_Count; ++n_Index)
        {
            ANIMATION::KEYFRAME::NODE& node{ kf.Nodes.at(n_Index) };
            FbxNode* n{ s->FindNodeByName(Scenes.Nodes.at(n_Index).Name.c_str()) };
            node.g_Transform = Convert::ToFloat4x4(n->EvaluateGlobalTransform());
            FbxAMatrix& l_Transform{ n->EvaluateLocalTransform() };
            node.Scaling = Convert::ToFloat3(l_Transform.GetS());
            node.Rotation = Convert::ToFloat4(l_Transform.GetQ());
            node.Translation = Convert::ToFloat3(l_Transform.GetT());
        }
        a.Keyframes.push_back(kf);
        Animations.push_back(a);
    }

    for (int ind = 0; ind < a_StackCount; ++ind)delete a_StackNames[ind];
}

/*-------------------------------------- - MODEL_RESOURCES CreateBuffers()----------------------------------------------*/

void FBXModel::CreateBuffers(ID3D11Device* dv, const char* model_path)
{
    for (auto& m : Meshes)
    {
        m.vbd.ByteWidth = (UINT)(m.Vertices.size() * sizeof(VERTEX));
        m.vbd.Usage = D3D11_USAGE_DEFAULT;
        m.vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
        m.vd.pSysMem = m.Vertices.data();

        HRESULT hr = dv->CreateBuffer(&m.vbd, &m.vd, m.dxVertexBuffer.GetAddressOf());
        if (FAILED(hr))assert(!"Failed to create Vertex Buffer");

        for (int a = 0; a < m.Subsets.size(); ) {
            if (m.Subsets[a].indices.size() <= 0) { m.Subsets.erase(m.Subsets.begin() + a); }
            else {
                D3D11_BUFFER_DESC ibd{};
                ibd.Usage = D3D11_USAGE_DEFAULT;
                ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
                ibd.ByteWidth = (UINT)(sizeof(int) * m.Subsets[a].indices.size());
                D3D11_SUBRESOURCE_DATA id{};
                id.pSysMem = m.Subsets[a].indices.data();
                HRESULT hr = DirectX11::Instance()->Device()->CreateBuffer(&ibd, &id, m.Subsets[a].subsetIndexBuffer.GetAddressOf());
                if (FAILED(hr))assert(!"Failed to create Index Buffer");
                ++a;
            }
        }
    }

    bool hasTexture{};
    for (auto m = Materials.begin(); m != Materials.end(); ++m)
    {
        // Set material path
        const char* back{ "/" };
        std::filesystem::path bs{ back };
        std::filesystem::path path(model_path);
        path.replace_extension("");
        std::filesystem::path model_name{ path.filename() };
        std::string dir{ "./Data/Model/" + model_name.string() + "/Textures/" };
        //model_name += 
        std::filesystem::path directory("/Textures/");
        path += directory;
        std::string form{ path.string() };

        // Begin setting texture from path
        for (int ind = 0; ind < 4; ++ind)
        {
            if (m->second.texture_path[ind] != "")
            {
                std::filesystem::path format{ dir };
                if (!std::filesystem::exists(format))
                    std::filesystem::create_directories(format);
                std::filesystem::path filename{ m->second.texture_path[ind] };
                //filename.replace_extension("DDS");

                path = format += filename.filename();
                m->second.Textures[ind] = TextureManager::Instance()->Retrieve(path.wstring());
            }
            else
                m->second.Textures[ind] = TextureManager::Instance()->Retrieve(texture_names[ind]);
        }
    }

    D3D11_BUFFER_DESC cbd{};
    cbd.ByteWidth = sizeof(MESH_CONSTANT_BUFFER);
    cbd.Usage = D3D11_USAGE_DEFAULT;
    cbd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    HRESULT hr = dv->CreateBuffer(&cbd, nullptr, meshConstantBuffer.GetAddressOf());
    if (FAILED(hr))assert(!"Failed to create constant buffer");

    cbd.ByteWidth = sizeof(SUBSET_CONSTANT_BUFFER);
    hr = dv->CreateBuffer(&cbd, nullptr, subsetConstantBuffer.GetAddressOf());
    assert(hr == S_OK);

    cbd.ByteWidth = sizeof(OUTLINE_CONSTANT_BUFFER);
    hr = dv->CreateBuffer(&cbd, nullptr, outlineConstantBuffer.GetAddressOf());
    assert(hr == S_OK);
}

/*-------------------------------------- - MODEL_RESOURCES AddAnimation()----------------------------------------------*/

bool FBXModel::AddAnimation(std::string anim_path, std::string animationName, float SamplingRate)
{
    FbxManager* manager{ FbxManager::Create() };
    FbxScene* scene{ FbxScene::Create(manager, "") };
    FbxImporter* importer{ FbxImporter::Create(manager, "") };

    bool status{};
    status = importer->Initialize(anim_path.c_str());
    std::string temp{ importer->GetStatus().GetErrorString() };
    assert(status);

    status = importer->Import(scene);
    assert(status);

    RetrieveAnimations(scene, SamplingRate);
    Animations.back().Name = animationName;
    manager->Destroy();

    return status;
}

/*-------------------------------------- - MODEL_RESOURCES BlendAnimation()----------------------------------------------*/

void FBXModel::BlendAnimation(ANIMATION::KEYFRAME* start, ANIMATION::KEYFRAME* end, float factor, ANIMATION::KEYFRAME* output)
{
    size_t n_Count{ start->Nodes.size() };

    output->Nodes.resize(n_Count);
    for (size_t n_Index = 0; n_Index < n_Count; ++n_Index)
    {
        XMVECTOR S[2] = { XMLoadFloat3(&start->Nodes.at(n_Index).Scaling), XMLoadFloat3(&end->Nodes.at(n_Index).Scaling) };
        XMVECTOR R[2] = { XMLoadFloat4(&start->Nodes.at(n_Index).Rotation), XMLoadFloat4(&end->Nodes.at(n_Index).Rotation) };
        XMVECTOR T[2] = { XMLoadFloat3(&start->Nodes.at(n_Index).Translation), XMLoadFloat3(&end->Nodes.at(n_Index).Translation) };

        XMStoreFloat3(&output->Nodes.at(n_Index).Scaling, XMVectorLerp(S[0], S[1], factor));
        XMStoreFloat4(&output->Nodes.at(n_Index).Rotation, XMQuaternionSlerp(R[0], R[1], factor));
        XMStoreFloat3(&output->Nodes.at(n_Index).Translation, XMVectorLerp(T[0], T[1], factor));
    }
}

/*-------------------------------------- - MODEL_RESOURCES Render()----------------------------------------------*/

void FBXModel::Render(ID3D11DeviceContext* dc, XMFLOAT4X4 world, XMFLOAT4 colour, const ANIMATION::KEYFRAME* kf)
{
    OUTLINE_CONSTANT_BUFFER outlineData{};
    outlineData.outline_color = { 1.0f, 1.0f, 1.0f, 1.0f };
    outlineData.outline_size = 0.01f;
    dc->RSSetState(RasterizerManager::Instance()->Retrieve("3D")->Rasterizer().Get());

    for (auto& shader : shaders)
    {
        if (shader.first == L"Outline.fx")continue;
        for (auto& m : Meshes)
        {
            dc->RSSetState(RasterizerManager::Instance()->Retrieve("3D")->Rasterizer().Get());
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
            dc->OMSetBlendState(BlendModeManager::Instance()->Get().Get(), 0, 0xFFFFFFFF);
            shader.second->SetShaders(dc);
            //Updating Object Constant buffers (World and colour)
            MESH_CONSTANT_BUFFER data{};
            XMMATRIX f_World{ XMLoadFloat4x4(&m.BaseTransform) * (XMLoadFloat4x4(&Axises.AxisCoords) * XMLoadFloat4x4(&world)) };       // Converting  Axis Systems to Base Axis

            XMMATRIX transposed;
            XMVECTOR s, r, t;
            XMMatrixDecompose(&s, &r, &t, XMLoadFloat4x4(&m.BaseTransform));
            t = XMVector3TransformCoord(t, XMLoadFloat4x4(&Axises.AxisCoords));
            transposed = XMMatrixScalingFromVector(s) * XMMatrixRotationRollPitchYawFromVector(r) * XMMatrixTranslationFromVector(t);

            data.colour = colour;               // Incase model has no subsets

            const size_t b_Count{ m.Bind_Pose.Bones.size() };
            for (size_t b_Index = 0; b_Index < b_Count; ++b_Index)
            {
                const SKELETON::BONE& b{ m.Bind_Pose.Bones.at(b_Index) };
                const ANIMATION::KEYFRAME::NODE& bNode{ kf->Nodes.at(b.n_Index) };
                XMStoreFloat4x4(&data.world, f_World);

                XMStoreFloat4x4(&data.b_Transform[b_Index],
                    XMLoadFloat4x4(&b.o_Transform)
                    * XMLoadFloat4x4(&bNode.g_Transform)
                    * XMMatrixInverse(nullptr, XMLoadFloat4x4(&m.BaseTransform)));
            }

            for (const auto& s : m.Subsets)
            {
                UINT stride{ sizeof(VERTEX) }, offset{ 0 };
                dc->IASetIndexBuffer(s.subsetIndexBuffer.Get(), DXGI_FORMAT_R32_UINT, offset);
                dc->IASetVertexBuffers(0, 1, m.dxVertexBuffer.GetAddressOf(), &stride, &offset);

                MATERIAL& ms{ Materials.at(s.m_UID) };
                XMStoreFloat4(&data.colour, XMLoadFloat4(&colour) * XMLoadFloat4(&ms.Kd));
                dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &data, 0, 0);
                dc->UpdateSubresource(outlineConstantBuffer.Get(), 0, 0, &outlineData, 0, 0);

                std::vector<ID3D11ShaderResourceView*>ts;
                for (int a = 0; a < 4; ++a)
                {
                    if (ms.texture_path[a] != "")
                        ts.push_back(ms.Textures[a]->GetSRV().Get());
                }

                int ind{};
                for (auto& t : ms.Textures)
                {
                    dc->PSSetShaderResources(ind, 1, t->GetSRV().GetAddressOf());
                    ++ind;
                }
                ID3D11Buffer* buffers[] = { meshConstantBuffer.Get(), outlineConstantBuffer.Get() };
                dc->VSSetConstantBuffers(1, 2, buffers);
                dc->PSSetConstantBuffers(1, 2, buffers);
                dc->DrawIndexed((UINT)(s.indices.size()), 0, 0);
            }
        }
    }
}

/*-------------------------------------- - MODEL_RESOURCES AddMaterial()----------------------------------------------*/

HRESULT FBXModel::AddMaterial(std::string mat_path, std::string model_name, MATERIAL_TYPE type, MATERIAL& m)
{
    std::string format;
    std::filesystem::path path(mat_path);
    std::string file_name{ path.filename().string() };
    format = "./Data/Model/" + model_name + "/Textures/";
    std::string local_path{ format + path.filename().string() };
    std::filesystem::path Path(local_path);

    switch (type)
    {
    case MATERIAL_TYPE::DIFFUSE:
    {
        m.texture_path[0] = Path.filename().string();
        m.Textures[0].reset(new TEXTURE(Path, DirectX11::Instance()->Device()));
        break;
    }
    case MATERIAL_TYPE::NORMAL:
    {
        m.texture_path[1] = Path.filename().string();
        m.Textures[1].reset(new TEXTURE(Path, DirectX11::Instance()->Device()));
        break;
    }
    case MATERIAL_TYPE::BUMP:
    {
        m.texture_path[2] = Path.filename().string();
        m.Textures[2].reset(new TEXTURE(Path, DirectX11::Instance()->Device()));
        break;
    }
    }
    return S_OK;
}

/*-------------------------------------- - MODEL_RESOURCES ResetDefaultTransform()----------------------------------------------*/

void FBXModel::ResetDefaultTransform(MESH* m)
{
    m->BaseTransform = Convert::Identity();
}