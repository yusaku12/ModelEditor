#include "Collision.h"

using namespace COLLIDERS;

/*---------------------------------------------------PointLineClosest()---------------------------------------------------*/
/// <summary>
/// <para> Calculates a point on a line that is closest to the target point </para>
/// <para> 目標点に一番近い点を計算 </para>
/// </summary>
/// <param name="top"> : Starting point of vector</param>
/// <param name="bot"> : Ending point of vector</param>
/// <returns></returns>
XMFLOAT3 COLLIDERS::PointLineClosest(XMFLOAT3 top, XMFLOAT3 bottom, XMFLOAT3 target)
{
    // Forming a line vector
    // 直線ベクター
    // Dot product to get the point
    // 座標点を計算
    float dot = {};
    DirectX::XMStoreFloat(&dot, XMVector3Dot(XMVectorSubtract(XMLoadFloat3(&target), XMLoadFloat3(&bottom)), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom)))));

    // Limiting point inside the line
    // 座標点を線内に制限
    float d1 = {}, d2 = {};
    DirectX::XMStoreFloat(&d1, (XMVector3Dot(XMVectorSubtract(XMVectorAdd(XMLoadFloat3(&bottom), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))), dot)), XMLoadFloat3(&top)), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))))));
    DirectX::XMStoreFloat(&d2, (XMVector3Dot(XMVectorSubtract(XMVectorAdd(XMLoadFloat3(&bottom), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))), dot)), XMLoadFloat3(&bottom)), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))))));

    if (d1 > 0)XMVectorAdd(XMLoadFloat3(&bottom), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))), dot)) = XMLoadFloat3(&top);
    if (d2 < 0)XMVectorAdd(XMLoadFloat3(&bottom), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))), dot)) = XMLoadFloat3(&bottom);

    XMFLOAT3 po = {};
    XMStoreFloat3(&po, XMVectorAdd(XMLoadFloat3(&bottom), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom))), dot)));

    return po;
}

/// <summary>
/// <para> Calculates a point on a line that is closest to the target point </para>
/// <para> 目標点に一番近い点を計算 </para>
/// </summary>
/// <returns></returns>
XMFLOAT3 COLLIDERS::PointLineClosest(XMFLOAT3 origin, CAPSULE* target)
{
    // Forming a line vector
    // 直線ベクター
    // Dot product to get the point
    // 座標点を計算
    float dot = {};
    XMStoreFloat(&dot, XMVector3Dot((XMVectorSubtract(XMLoadFloat3(&origin), XMLoadFloat3(&target->Bottom()))), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom())))));

    float d1 = {}, d2 = {};
    DirectX::XMStoreFloat(&d1, XMVector3Dot(XMVectorSubtract(XMVectorAdd(XMLoadFloat3(&target->Bottom()), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom()))), dot)), XMLoadFloat3(&target->Top())), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom())))));
    DirectX::XMStoreFloat(&d2, XMVector3Dot(XMVectorSubtract(XMVectorAdd(XMLoadFloat3(&target->Bottom()), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom()))), dot)), XMLoadFloat3(&target->Bottom())), XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom())))));

    // Limiting point inside the line
    // 座標点を線内に制限
    if (d1 > 0)XMVectorAdd(XMLoadFloat3(&target->Bottom()), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom()))), dot)) = XMLoadFloat3(&target->Top());
    if (d2 < 0)XMVectorAdd(XMLoadFloat3(&target->Bottom()), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom()))), dot)) = XMLoadFloat3(&target->Bottom());

    XMFLOAT3 POINT = {};
    XMStoreFloat3(&POINT, XMVectorAdd(XMLoadFloat3(&target->Bottom()), XMVectorScale(XMVector3Normalize(XMVectorSubtract(XMLoadFloat3(&target->Top()), XMLoadFloat3(&target->Bottom()))), dot)));

    return POINT;
}

/*---------------------------------------------------AxisCasting()---------------------------------------------------*/
/// <summary>
/// <para> Performs axis casting, where each point is casted onto the axis, and is compared </para>
/// <para> 各点を軸にキャストして比較される </para>
/// </summary>
/// <param name="oriMin"> ： Minimum point of first collider</param>
/// <param name="oriMax"> ： Maximum point of first collider</param>
/// <param name="tarMin"> ： Minimum point of second collider</param>
/// <param name="tarMax"> ： Maximum point of second collider</param>
/// <param name="rotation"> : Rotation of first collider</param>
/// <param name="colCount"> : Output. Shows how many time it is a hit</param>
void COLLIDERS::AxisCasting(XMFLOAT3 oriMin, XMFLOAT3 oriMax, XMFLOAT3 tarMin, XMFLOAT3 tarMax, XMFLOAT3 rotation, int* colCount)
{
    // Extracing the axises from the rotation matrix 
    // 回転MatrixからXYZ軸を抽出
    XMFLOAT3 temp{ rotation.x, rotation.y, rotation.z };
    XMMATRIX tempM{ XMMatrixIdentity() };
    tempM = XMMatrixRotationRollPitchYaw(rotation.x, rotation.y, rotation.z);
    XMFLOAT3 r, t, f;
    XMStoreFloat3(&r, tempM.r[0]);
    XMStoreFloat3(&t, tempM.r[1]);
    XMStoreFloat3(&f, tempM.r[2]);
    XMFLOAT3 Right{ r }, Top{ t }, Front{ f };

    float ori_x1, ori_y1, ori_z1, ori_x2, ori_y2, ori_z2;
    float tar_x1, tar_y1, tar_z1, tar_x2, tar_y2, tar_z2;

    // Perform dot on each points to check if they are within range
    // 各座標を内積して範囲内チェック
    XMStoreFloat(&ori_x1, XMVector3Dot(XMLoadFloat3(&Right), XMLoadFloat3(&oriMin)));
    XMStoreFloat(&ori_x2, XMVector3Dot(XMLoadFloat3(&Right), XMLoadFloat3(&oriMax)));
    XMStoreFloat(&ori_y1, XMVector3Dot(XMLoadFloat3(&Top), XMLoadFloat3(&oriMin)));
    XMStoreFloat(&ori_y2, XMVector3Dot(XMLoadFloat3(&Top), XMLoadFloat3(&oriMax)));
    XMStoreFloat(&ori_z1, XMVector3Dot(XMLoadFloat3(&Front), XMLoadFloat3(&oriMin)));
    XMStoreFloat(&ori_z2, XMVector3Dot(XMLoadFloat3(&Front), XMLoadFloat3(&oriMax)));
    XMStoreFloat(&tar_x1, XMVector3Dot(XMLoadFloat3(&Right), XMLoadFloat3(&tarMin)));
    XMStoreFloat(&tar_x2, XMVector3Dot(XMLoadFloat3(&Right), XMLoadFloat3(&tarMax)));
    XMStoreFloat(&tar_y1, XMVector3Dot(XMLoadFloat3(&Top), XMLoadFloat3(&tarMin)));
    XMStoreFloat(&tar_y2, XMVector3Dot(XMLoadFloat3(&Top), XMLoadFloat3(&tarMax)));
    XMStoreFloat(&tar_z1, XMVector3Dot(XMLoadFloat3(&Front), XMLoadFloat3(&tarMin)));
    XMStoreFloat(&tar_z2, XMVector3Dot(XMLoadFloat3(&Front), XMLoadFloat3(&tarMax)));

    // Perform Comparison on each axis
    if (ori_x1 > tar_x2 || ori_x2 < tar_x1)
        return;
    if (ori_y1 > tar_y2 || ori_y2 < tar_y1)
        return;
    if (ori_z1 > tar_z2 || ori_z2 < tar_z1)
        return;
    *colCount += 3;
}

/*---------------------------------------------------OBBCollision()---------------------------------------------------*/
/// <summary>
/// <para> Performs collision check between 2 OBBs </para>
/// <para> 2つのOBBに当たり判定を計算 </para>
/// </summary>
/// <returns></returns>
bool COLLIDERS::OBBCollision(OBB* ori, OBB* tar)
{
    // Check if colliders are activated
    // コライダーのステータスチェック
    if (!ori->Status() || !tar->Status())
        return false;

    // Check both OBB for their minimum and maximum points
    // 両方のOBBの最小と最大点をチェック
    XMFLOAT3 min1, max1, min2, max2;
    for (auto& v : ori->Points())
    {
        if (v.x == ori->Points().at(0).x && v.y == ori->Points().at(0).y && v.z == ori->Points().at(0).z)
        {
            min1 = v;
            max1 = v;
        }

        if (min1.x > v.x)
            min1.x = v.x;
        if (min1.y > v.y)
            min1.y = v.y;
        if (min1.z > v.z)
            min1.z = v.z;
        if (max1.x < v.x)
            max1.x = v.x;
        if (max1.y < v.y)
            max1.y = v.y;
        if (max1.z < v.z)
            max1.z = v.z;
    }
    for (auto& v : tar->Points())
    {
        if (v.x == tar->Points().at(0).x && v.y == tar->Points().at(0).y && v.z == tar->Points().at(0).z)
        {
            min2 = v;
            max2 = v;
        }
        if (min2.x > v.x)
            min2.x = v.x;
        if (min2.y > v.y)
            min2.y = v.y;
        if (min2.z > v.z)
            min2.z = v.z;
        if (max2.x < v.x)
            max2.x = v.x;
        if (max2.y < v.y)
            max2.y = v.y;
        if (max2.z < v.z)
            max2.z = v.z;
    }

    // Perform axis casting to see if all points are within range
    // Axis Castを使ってすべての座標は範囲内チェック
    int count{};
    AxisCasting(min1, max1, min2, max2, ori->Rotation(), &count);
    AxisCasting(min1, max1, min2, max2, tar->Rotation(), &count);
    return count == 6;
}

/*---------------------------------------------------RayCast()---------------------------------------------------*/
/// <summary>
/// <para> Perform Raycasting </para>
/// <para> レイーキャストを計算 </para>
/// </summary>
/// <param name="s"> : Starting point of ray</param>
/// <param name="e"> : Direction of ray</param>
/// <param name="m"> : Target model</param>
/// <param name="hr"> : Output. RayCastData is stored here. Create a new and put it here</param>
/// <returns></returns>
bool COLLIDERS::RayCast(XMFLOAT3& s, XMFLOAT3& e, Model* m, RAYCASTDATA& hr, int mesh_index)
{
    XMVECTOR w_Start{ XMLoadFloat3(&s) };                           // Ray World Start Position
    XMVECTOR w_End{ XMLoadFloat3(&e) };                             // Ray World End Position
    XMVECTOR w_RayVector{ w_End - w_Start };                        // World Ray Vector 
    XMVECTOR w_RayLength = XMVector3Length(w_RayVector);            // World Ray Length
    XMStoreFloat(&hr.distance, w_RayLength);

    // Retrieve current keyframe 
    // 現在のキーフレームを抽出
    bool hit{};
    FBXModel::ANIMATION::KEYFRAME& kf = m->Resource()->Animations.at(m->CurrentTake()).Keyframes.at(m->CurrentFrame());
    int cur_index{};
    for (auto& ms : m->Resource()->Meshes)
    {
        bool onTarget{};
        if (mesh_index != -1)
        {
            if (mesh_index != cur_index)
            {
                ++cur_index;
                continue;
            }
            else
            onTarget = true;
        }
        if (!onTarget && mesh_index != -1)continue;

        // Retrieve the current mesh node and transform matrix to Local Transformation
        // げんざいMESHNODEを抽出し、ローカル変換行列に変換
        FBXModel::ANIMATION::KEYFRAME::NODE& n = m->Resource()->Animations.at(m->CurrentTake()).Keyframes.at(m->CurrentFrame()).Nodes.at(ms.n_Index);

        XMMATRIX w_Transform{ XMLoadFloat4x4(&n.g_Transform) };
        w_Transform *= m->TransformMatrix();
        XMMATRIX inv_w_Transform{ XMMatrixInverse(nullptr, w_Transform) };

        XMVECTOR S{ XMVector3TransformCoord(w_Start, inv_w_Transform) };
        XMVECTOR E{ XMVector3TransformCoord(w_End, inv_w_Transform) };

        XMVECTOR V{ E - S };
        XMVECTOR Dir{ XMVector3Normalize(V) };
        XMVECTOR L{ XMVector3Length(V) };
        float min_Length{};
        XMStoreFloat(&min_Length, L);

        std::vector<FBXModel::VERTEX>& v{ ms.Vertices };
        const std::vector<int>i{ ms.Indices };

        int m_Index{ -1 };
        XMVECTOR h_Pos, h_Norm;
        for (auto& sub : ms.Subsets)
        {
            for (int in = 0; in < sub.indices.size(); in += 3)
            {
                FBXModel::VERTEX& a{ v.at(sub.indices[in]) };
                FBXModel::VERTEX& b{ v.at(sub.indices[in + 1]) };
                FBXModel::VERTEX& c{ v.at(sub.indices[in + 2]) };

                //  Step 1: Triangle Vertex Retrieval
                // 三角の生成
                XMVECTOR A{ XMLoadFloat3(&a.position) };
                XMVECTOR B{ XMLoadFloat3(&b.position) };
                XMVECTOR C{ XMLoadFloat3(&c.position) };

                // Skip if not near
                XMFLOAT3 point{};
                XMStoreFloat3(&point, A);

                //  Step 2: Vector of edges
                // 三角のベクタ
                XMVECTOR AB{ B - A };
                XMVECTOR BC{ C - B };
                XMVECTOR CA{ A - C };

                //  Step 3: Normal retrieval
                // 法線抽出
                XMVECTOR Normal{ XMVector3Cross(AB, BC) };

                //  Step 4: In front of or behind
                // ターゲットは三角以外か以内
                XMVECTOR Dot{ XMVector3Dot(Normal, Dir) };
                float f_Dot;
                XMStoreFloat(&f_Dot, Dot);
                if (f_Dot >= 0)
                    continue;
                // Step 5: Point of intersection
                // 交差点
                XMVECTOR distance{ A - S };
                XMVECTOR T = XMVector3Dot(Normal, distance) / Dot;
                float length = XMVectorGetX(T);
                if (length > min_Length || length < 0)
                    continue;
                XMVECTOR ContactPoint{ S + Dir * T };

                // Perform dot check on each point on the triangle 
                // 各点に内積チェック
                XMVECTOR PA{ A - ContactPoint };
                XMVECTOR PB{ B - ContactPoint };
                XMVECTOR PC{ C - ContactPoint };

                XMVECTOR C_PAB{ XMVector3Cross(PA, AB) };
                XMVECTOR C_PAC{ XMVector3Cross(PB, BC) };
                XMVECTOR C_PBC{ XMVector3Cross(PC, CA) };

                XMVECTOR PAB_DOT{ XMVector3Dot(Normal, C_PAB) };
                XMVECTOR PAC_DOT{ XMVector3Dot(Normal, C_PAC) };
                XMVECTOR PBC_DOT{ XMVector3Dot(Normal, C_PBC) };

                float f_pabdot{ XMVectorGetX(PAB_DOT) }, f_pacdot{ XMVectorGetX(PAC_DOT) }, f_pbcdot{ XMVectorGetX(PBC_DOT) };
                if (f_pabdot < 0 || f_pacdot < 0 || f_pbcdot < 0)
                    continue;
                h_Pos = ContactPoint;
                h_Norm = Normal;
                m_Index = (int)sub.m_UID;
            }
        }
        if (m_Index >= 0)
        {
            // RAYCASTDATA storing
            // RAYCASTDATA 保存

            XMVECTOR w_Position{ XMVector3TransformCoord(h_Pos, w_Transform) };
            XMVECTOR w_CrossVector{ w_Position - w_Start };
            XMVECTOR w_CrossLength{ XMVector3Length(w_CrossVector) };
            float dist;
            XMStoreFloat(&dist, w_CrossLength);
            if (hr.distance > dist)
            {
                XMVECTOR w_Normal = XMVector3TransformCoord(h_Norm, w_Transform);
                hr.distance = dist;
                hr.m_Index = m_Index;
                XMStoreFloat3(&hr.position, w_Position);
                XMStoreFloat3(&hr.normal, w_Normal);

                XMVECTOR NORMAL = XMVector3Normalize(XMLoadFloat3(&hr.normal));
                hit = true;
            }
        }
    }
    return hit;
}

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------COLLIDER_BASE Class--------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------COLLIDER_BASE FitToBone()--------------------------------------------------------------*/
/// <summary>
/// <para> virtual Function. Called to fit the collider to the bone of the model</para>
/// <para> 仮想関数。モデルにしてされたボーンにコライダーをセット</para>
/// </summary>
/// <param name="name"> : Name of bone</param>
/// <param name="m"> : Pointer of model</param>
void COLLIDER_BASE::FitToBone(std::string bone_name, Model* m)
{
    int64_t index{ -1 };
    XMMATRIX bone, global;
    for (auto& m : m->Resource()->Meshes)
    {
        for (auto& b : m.Bind_Pose.Bones)
        {
            if (b.Name == bone_name)
            {
                index = b.n_Index;
                break;
            }
        }
    }
    if (index != -1)
    {
        int kf{ m->NextFrame() };
        FBXModel::ANIMATION& an{ m->Resource()->Animations[m->CurrentTake()] };

        int size{ (int)an.Keyframes.size() - 1 };
        int take{ m->CurrentFrame() };
        kf = (std::min)((std::max)(kf, 0), (int)an.Keyframes.size() - 1);
        XMFLOAT4X4 temp = m->Resource()->Animations[m->CurrentTake()].Keyframes[kf].Nodes.at(index).g_Transform;
        bone = XMLoadFloat4x4(&temp);
    }
    global = m->TransformMatrix();
    XMFLOAT4X4 temp{ m->Resource()->Axises.AxisCoords };

    bone *= XMLoadFloat4x4(&temp);
    bone *= MatrixOffset() * global;
    bone_World = bone;
    Execute(bone);
}

/*-----------------------------------------------------COLLIDER_BASE MatrixOffset()--------------------------------------------------------------*/

XMMATRIX COLLIDER_BASE::MatrixOffset()
{
    return XMMatrixScaling(1, 1, 1) * XMMatrixRotationQuaternion(XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rotation))) * XMMatrixTranslationFromVector(XMLoadFloat3(&offset));
}

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------SPHERE Class--------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------SPHERE Constructor--------------------------------------------------------------*/

#pragma region SPHERE
SPHERE::SPHERE(XMFLOAT3 pos, float rad)
{
    center = pos;
    radius = rad;
}

/*-----------------------------------------------------SPHERE Execute()--------------------------------------------------------------*/

void SPHERE::Execute(XMMATRIX mat)
{
    XMStoreFloat3(&center, (XMVector3TransformCoord({}, mat)));
}

/*-----------------------------------------------------SPHERE Render()--------------------------------------------------------------*/

void SPHERE::Render()
{
}

/*-----------------------------------------------------SPHERE Collide()--------------------------------------------------------------*/

bool SPHERE::Collide(COLLIDER_BASE* other)
{
    SPHERE* target{ dynamic_cast<SPHERE*>(other) };

    if (target)
    {
        float min_dist{ radius + target->radius };

        XMFLOAT3 Sub = {};
        XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&center), XMLoadFloat3(&target->Center())));
        float distance = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);

        if (distance < min_dist)
            return true;
    }
    return false;
}
bool SPHERE::Collide(XMFLOAT3 p)
{
    XMFLOAT3 Sub = {};
    XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&Center()), XMLoadFloat3(&p)));
    float distance = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);
    return distance < radius;
}

/*-----------------------------------------------------SPHERE Center()--------------------------------------------------------------*/

XMFLOAT3 SPHERE::Center()
{
    return center;
}

/*-----------------------------------------------------SPHERE Radius()--------------------------------------------------------------*/

float SPHERE::Radius()
{
    return radius;
}

/*-----------------------------------------------------SPHERE SetCenter()--------------------------------------------------------------*/

void SPHERE::SetCenter(XMFLOAT3 v)
{
    center = v;
}

/*-----------------------------------------------------SPHERE SetRadius()--------------------------------------------------------------*/

void SPHERE::SetRadius(float rad)
{
    radius = rad;
}

/*-----------------------------------------------------SPHERE SetData()--------------------------------------------------------------*/

void SPHERE::SetData(ComponentData* data)
{
    //SphereCollider_Data* d{ CastData<SphereCollider_Data>(data) };
    //SetCenter(d->center);
    //SetRadius(d->radius);
}

#pragma endregion
#pragma region OBB
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------OBB Class--------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------OBB Constructor--------------------------------------------------------------*/

OBB::OBB()
{
    *this = OBB({}, {});
}
OBB::OBB(XMFLOAT3 vMin, XMFLOAT3 vMax)
{
    oriMin = vMin;
    oriMax = vMax;

    XMFLOAT3& p = points.emplace_back();
    p = vMin;
    XMFLOAT3& p2 = points.emplace_back();
    p2 = { vMax.x, vMin.y, vMin.z };
    XMFLOAT3& p3 = points.emplace_back();
    p3 = { vMin.x, vMax.y, vMin.z };
    XMFLOAT3& p4 = points.emplace_back();
    p4 = { vMax.x, vMax.y, vMin.z };
    XMFLOAT3& p5 = points.emplace_back();
    p5 = { vMin.x, vMin.y, vMax.z };
    XMFLOAT3& p6 = points.emplace_back();
    p6 = { vMax.x, vMin.y, vMax.z };
    XMFLOAT3& p7 = points.emplace_back();
    p7 = { vMin.x, vMax.y, vMax.z };
    XMFLOAT3& p8 = points.emplace_back();
    p8 = vMax;
    Initialize();
}

/*-----------------------------------------------------OBB Initialize()--------------------------------------------------------------*/\

HRESULT OBB::Initialize()
{
    return S_OK;
}

/*-----------------------------------------------------OBB UpdatePosition()--------------------------------------------------------------*/\

void OBB::UpdatePosition(XMMATRIX mat)
{
    std::vector<XMFLOAT3>& ps(points);

    ps[0] = oriMin;
    ps[1] = XMFLOAT3{ oriMax.x, oriMin.y, oriMin.z };
    ps[2] = XMFLOAT3{ oriMin.x, oriMax.y, oriMin.z };
    ps[3] = XMFLOAT3{ oriMax.x, oriMax.y, oriMin.z };
    ps[4] = XMFLOAT3{ oriMin.x, oriMin.y, oriMax.z };
    ps[5] = XMFLOAT3{ oriMax.x, oriMin.y, oriMax.z };
    ps[6] = XMFLOAT3{ oriMin.x, oriMax.y, oriMax.z };
    ps[7] = oriMax;

    // Changing points to global transfrom
    XMMATRIX world{ XMMatrixScaling(1, 1, 1) * XMMatrixTranslationFromVector(XMLoadFloat3(&offset)) };
    for (auto& p : points)
    {
        XMVECTOR pss = XMVector3TransformCoord(XMLoadFloat3(&p), world);
        pss = XMVector3TransformCoord(pss, mat);
        XMStoreFloat3(&p, pss);
    }
}

/*-----------------------------------------------------OBB Update()--------------------------------------------------------------*/\

void OBB::Update(XMFLOAT3 pos, XMFLOAT3 rot)
{
    XMMATRIX T{ XMMatrixTranslationFromVector(XMLoadFloat3(&pos)) };
    XMVECTOR Q{ XMQuaternionRotationRollPitchYawFromVector(XMLoadFloat3(&rot)) };
    XMMATRIX R{ XMMatrixRotationQuaternion(Q) };
    XMMATRIX W{ R * T };
    UpdatePosition(W);
}

/*-----------------------------------------------------OBB Execute()--------------------------------------------------------------*/\

void OBB::Execute(XMMATRIX mat)
{
    UpdatePosition(mat);
}

/*-----------------------------------------------------OBB Render()--------------------------------------------------------------*/\

void OBB::Render()
{
}

/*-----------------------------------------------------OBB Collide()--------------------------------------------------------------*/\

bool OBB::Collide(COLLIDER_BASE* other)
{
    return OBBCollision(this, static_cast<OBB*>(other));
}
bool OBB::Collide(XMFLOAT3 p)
{
    XMFLOAT3 Sub1 = {};
    XMStoreFloat3(&Sub1, XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&points[8])));
    float Length1 = sqrtf(Sub1.x * Sub1.x + Sub1.y * Sub1.y + Sub1.z * Sub1.z);

    XMFLOAT3 Sub2 = {};
    XMStoreFloat3(&Sub2, XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&points[8])));
    float Length2 = sqrtf(Sub2.x * Sub2.x + Sub2.y * Sub2.y + Sub2.z * Sub2.z);

    return Length2 < Length1;
}

/*-----------------------------------------------------OBB Points()--------------------------------------------------------------*/\

std::vector<XMFLOAT3>OBB::Points()
{
    return points;
}

/*-----------------------------------------------------OBB Rotation()--------------------------------------------------------------*/\

XMFLOAT3 OBB::Rotation()
{
    return rotation;
}

/*-----------------------------------------------------OBB Center()--------------------------------------------------------------*/\

XMFLOAT3 OBB::Center()
{
    float temp = {};
    XMStoreFloat(&temp, XMVectorSubtract(XMLoadFloat3(&points[7]), XMLoadFloat3(&points[0])));
    temp *= .5;

    XMFLOAT3 TEMP = {};
    XMStoreFloat3(&TEMP, XMVectorScale(XMLoadFloat3(&*points.begin()), temp));

    return TEMP;
}

/*-----------------------------------------------------OBB Size()--------------------------------------------------------------*/\

float OBB::Size()
{
    XMFLOAT3 center{ Center() };

    XMFLOAT3 Sub = {};
    XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&points[0]), XMLoadFloat3(&center)));
    float Length = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);

    return Length;
}

/*-----------------------------------------------------OBB Status()--------------------------------------------------------------*/\

bool OBB::Status()
{
    return isActive;
}

/*-----------------------------------------------------OBB SetMin()--------------------------------------------------------------*/\

void OBB::SetMin(XMFLOAT3 min)
{
    oriMin = min;
}

/*-----------------------------------------------------OBB SetMax()--------------------------------------------------------------*/\

void OBB::SetMax(XMFLOAT3 max)
{
    oriMax = max;
}

/*-----------------------------------------------------OBB SetData()--------------------------------------------------------------*/\

void OBB::SetData(ComponentData* d)
{
    //OBBCollider_Data* od{ static_cast<OBBCollider_Data*>(d) };
    //SetMin(od->min);
    //SetMax(od->max);
}

#pragma endregion
#pragma region CYLINDER_UNUSED

CYLINDER::CYLINDER()
{
}
CYLINDER::CYLINDER(XMFLOAT3 tp, XMFLOAT3 bot, float rad) : top(tp), bottom(bot), radius(rad)
{
}
HRESULT CYLINDER::Initialize()
{
    XMFLOAT3 Sub = {};
    XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom)));
    float Length = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);

    height = Length;
    return S_OK;
}
void CYLINDER::Execute(XMMATRIX mat)
{
    XMStoreFloat3(&top, (XMVector3TransformCoord(XMLoadFloat3(&top), mat)));
    XMStoreFloat3(&bottom, (XMVector3TransformCoord(XMLoadFloat3(&bottom), mat)));
}
void CYLINDER::Render()
{
}
bool CYLINDER::Collide(COLLIDER_BASE* other)
{
    CYLINDER* target = (CYLINDER*)other;

    XMFLOAT3 ori_closest_point{ top }, tar_closest_point{ target->Top() };

    tar_closest_point = PointLineClosest(target->top, target->bottom, ori_closest_point);
    ori_closest_point = PointLineClosest(top, bottom, tar_closest_point);

    float V1 = {}, V2 = {};
    XMStoreFloat(&V1, XMVectorSubtract(XMLoadFloat3(&top), XMLoadFloat3(&bottom)));
    XMStoreFloat(&V2, XMVectorSubtract(XMLoadFloat3(&bottom), XMLoadFloat3(&top)));
      
    if (V1 < 0 || V2 < 0)
        return false;

    XMFLOAT3 Sub = {};
    XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&tar_closest_point), XMLoadFloat3(&ori_closest_point)));

    float Length = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);

    return Length < radius + target->Radius();
}
void CYLINDER::SetData(XMFLOAT3 tp, XMFLOAT3 bot, float rad)
{
    top = tp;
    bottom = bot;
    radius = rad;
}

XMFLOAT3 CYLINDER::Top()
{
    return top;
}
XMFLOAT3 CYLINDER::Bottom()
{
    return bottom;
}
float CYLINDER::Height()
{
    return height;
}
float CYLINDER::Radius()
{
    return radius;
}

#pragma endregion
#pragma region CAPSULE

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------CAPSULE Class--------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------CAPSULE Constructor--------------------------------------------------------------*/

CAPSULE::CAPSULE() : center(), radius()
{
    Initialize();
}
CAPSULE::CAPSULE(float rad) : center(), radius(rad)
{
    Initialize();
}
CAPSULE::CAPSULE(XMFLOAT3 cent, float rad, float ht) : center(cent), radius(rad), height(ht)
{
    Initialize();
}

/*-----------------------------------------------------CAPSULE Initialize()--------------------------------------------------------------*/

HRESULT CAPSULE::Initialize()
{
    return S_OK;
}

/*-----------------------------------------------------CAPSULE Execute()--------------------------------------------------------------*/

void CAPSULE::Execute(XMMATRIX mat)
{
    world = MatrixOffset() * mat;
}

/*-----------------------------------------------------CAPSULE Render()--------------------------------------------------------------*/

void CAPSULE::Render()
{
}

/*-----------------------------------------------------CAPSULE Collide()--------------------------------------------------------------*/

bool CAPSULE::Collide(COLLIDER_BASE* other)
{
    CAPSULE* target{ static_cast<CAPSULE*>(other) };

    // Check for point closest to each other 
    //一番近い点を検索
    XMFLOAT3 p0, p1;
    XMFLOAT3 top{ Top() }, bottom{ Bottom() };
    p0 = top;
    p1 = PointLineClosest(target->Top(), target->Bottom(), p0);
    p0 = PointLineClosest(top, bottom, p1);

    float minDist{ radius + target->radius };

    XMFLOAT3 Sub = {};
    XMStoreFloat3(&Sub, XMVectorSubtract(XMLoadFloat3(&p0), XMLoadFloat3(&p1)));
    float Length = sqrtf(Sub.x * Sub.x + Sub.y * Sub.y + Sub.z * Sub.z);

    if (Length < minDist)
        return true;
    return false;
}
bool CAPSULE::Collide(XMFLOAT3 p)
{
    XMFLOAT3 point = PointLineClosest(Top(), Bottom(), p);

    XMFLOAT3 W = {};
    XMStoreFloat3(&W, XMVectorSubtract(XMLoadFloat3(&point), XMLoadFloat3(&p)));
    float distance = sqrtf(W.x * W.x + W.y * W.y + W.z * W.z);

    return distance < radius;
}

/*-----------------------------------------------------CAPSULE Top()--------------------------------------------------------------*/

XMFLOAT3 CAPSULE::Top()
{
    XMFLOAT3 center_point{};
    XMStoreFloat3(&center_point, (XMVector3TransformCoord(XMLoadFloat3(&center), world)));
    XMFLOAT3 top = { center.x, center.y + height / 2, center.z };
    XMStoreFloat3(&top, (XMVector3TransformCoord(XMLoadFloat3(&top), world)));
    return top;
}

/*-----------------------------------------------------CAPSULE Bottom()--------------------------------------------------------------*/

XMFLOAT3 CAPSULE::Bottom()
{
    XMFLOAT3 center_point{};
    XMStoreFloat3(&center_point, (XMVector3TransformCoord(XMLoadFloat3(&center), world)));

    XMFLOAT3 bottom = { center.x, center.y - height / 2, center.z };
    XMStoreFloat3(&bottom, (XMVector3TransformCoord(XMLoadFloat3(&bottom), world)));
    return bottom;
}

/*-----------------------------------------------------CAPSULE Center()--------------------------------------------------------------*/

XMFLOAT3 CAPSULE::Center()
{
    return center;
}

/*-----------------------------------------------------CAPSULE Radius()--------------------------------------------------------------*/

float CAPSULE::Radius()
{
    return radius;
}

/*-----------------------------------------------------CAPSULE Size()--------------------------------------------------------------*/

float CAPSULE::Size()
{
    return radius;
}

/*-----------------------------------------------------CAPSULE OffsetCenter()--------------------------------------------------------------*/

void CAPSULE::OffsetCenter(XMMATRIX world)
{
}

/*-----------------------------------------------------CAPSULE Center()--------------------------------------------------------------*/

void CAPSULE::SetCenter(XMFLOAT3 c)
{
    center = c;
}

/*-----------------------------------------------------CAPSULE SetRadius()--------------------------------------------------------------*/

void CAPSULE::SetRadius(float r)
{
    radius = r;
}

/*-----------------------------------------------------CAPSULE SetHeight()--------------------------------------------------------------*/

void CAPSULE::SetHeight(float h)
{
    height = h;
}

/*-----------------------------------------------------CAPSULE SetData()--------------------------------------------------------------*/

void CAPSULE::SetData(ComponentData* d)
{
    //CapsuleCollider_Data* cd = (static_cast<CapsuleCollider_Data*>(d));
    //SetRadius(cd->radius);
}

#pragma endregion
#pragma region RAYCAST_MANAGER

/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------RAYCAST_MANAGER Class--------------------------------------------------------------------*/
/*--------------------------------------------------------------------------------------------------------------------------------------------*/
/*-----------------------------------------------------RAYCAST_MANAGER Insert()--------------------------------------------------------------*/
/// <summary>
/// <para> Inserts the model into the map and allow it to perform collision check </para>
/// <para> モデルをマップに登録し、MESH COLLIDERの対象内になる</para>
/// </summary>
/// <param name="name"> : Name of model</param>
/// <param name="m"> : Model pointer</param>
void RAYCAST_MANAGER::Insert(Mesh_Component* m)
{
    meshes.push_back(m);
}

/*-----------------------------------------------------RAYCAST_MANAGER Finalize()--------------------------------------------------------------*/
/// <summary>
/// Called at the end of the program or when switching scenes
/// </summary>
void RAYCAST_MANAGER::Finalize()
{
    meshes.clear();
}

/*-----------------------------------------------------RAYCAST_MANAGER Collide()--------------------------------------------------------------*/
/// <summary>
/// <para> Perform ray casting collision check </para>
/// <para> レイーキャストを利用して当たり判定を計算 </para>
/// </summary>
/// <param name="name"> : Name of current model. Collision check will not be performed onto this model</param>
/// <param name="startOfRay"> : Starting point of object</param>
/// <param name="direction_vector"> : Direction of movement</param>
/// <param name="rcd"> : Output. RayCastData is stored here. Create a new and put it here</param>
/// <returns></returns>
bool RAYCAST_MANAGER::Collide(XMFLOAT3 startOfRay, XMFLOAT3 endOfRay, Mesh_Component* cur_mesh, RAYCASTDATA& rcd)
{
    //bool output{};
    //for (auto& m : meshes)
    //{
    //    Vector3 target{ m->Parent()->GetComponent<Transform3D_Component>()->Translation() };
    //    //if ((target - startOfRay).Length() > 0.3f)
    //    //    continue;
    //    if (m == cur_mesh)
    //        continue;
    //    output = RayCast(startOfRay, endOfRay, m->Model().get(), rcd);
    //    if (output)
    //        break;
    //}
    //return output;
    return false;
}

/*-----------------------------------------------------RAYCAST_MANAGER Collide()--------------------------------------------------------------*/

bool RAYCAST_MANAGER::Collide(XMFLOAT3 startofRay, XMFLOAT3 endOfRay, Mesh_Component* target_mesh, int target_mesh_index, RAYCASTDATA& rcd)
{
    //bool output{};
    //for (auto& m : meshes)
    //{
    //    if (m != target_mesh)
    //        continue;
    //    output = RayCast(startofRay, endOfRay, m->Model().get(), rcd);
    //    if (output)
    //        break;
    //}
    //return output;
    return false;
}

/*-----------------------------------------------------RAYCAST_MANAGER Collide()--------------------------------------------------------------*/

void RAYCAST_MANAGER::GetListOfCollided(Mesh_Component* cur_Mesh, XMFLOAT3 startOfRay, XMFLOAT3 directionVector, std::vector<RAYCASTDATA>& rcd)
{
    //for (auto& m : meshes)
    //{
    //    RAYCASTDATA& cur_rcd{ rcd.emplace_back() };
    //    if (RayCast(startOfRay, directionVector, m->Model().get(), cur_rcd))
    //    {
    //        cur_rcd.model_name = m->Parent()->Data()->Name();
    //    }
    //}
}

/*-----------------------------------------------------RAYCAST_MANAGER ModelMap()--------------------------------------------------------------*/

std::vector<Mesh_Component*>RAYCAST_MANAGER::Meshes()
{
    return meshes;
}
#pragma endregion