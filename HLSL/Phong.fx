float3 CalculateLambertDiffuse(float3 normal, float3 lightDir, float3 lightColour, float3 kd)
{
    float d = max(0, dot(-lightDir, normal));
    return d * lightColour * kd;
}
float3 CalculatePhongSpecular(float3 normal, float3 lightVector, float3 lightColour, float3 eyeVector, float shineFactor, float3 ks)
{
    float3 r = reflect(lightVector, normal);
    float d = max(0, pow(max(0, dot(r, eyeVector)), shineFactor));
    return d * lightColour * ks;
}
float3 CalculateHalfLambertDiffuse(float3 normal, float3 lightDir, float3 lightColour, float3 kd)
{
    float d = dot(lightDir, normal) * 0.5f + 0.5f;
    d = clamp(d, 0, 1);
    return d * lightColour * kd;
}
float3 CalculateRimLighting(float3 normal, float3 eyeVector, float3 lightVector, float3 lightColour, float rimPower = 3.0f)
{
    float rim = 1.0f - saturate(dot(eyeVector, normal));
    rim = pow(rim, rimPower);
    return lightColour * rim * saturate(dot(eyeVector, -lightVector));
}
float3 CalculateToonDiffuse(Texture2D t2d, SamplerState tss, float3 normal, float3 lightVector, float3 lightColour, float3 kd)
{
    float u = clamp(dot(normal, lightVector), 0, 1);
    float c = t2d.Sample(tss, float2(u, 0.1f));
    return lightColour * c * kd;
}

struct DLIGHT_DATA
{
    float4 direction;
    float4 colour;
};
struct PLIGHT_DATA
{
    float4 position;
    float4 colour;
    float range;
    float intensity;
    float2 temp;
};
struct SLIGHT_DATA
{
    float4 position;
    float4 direction;
    float4 colour;
    float range;
    float innerCorner;
    float outerCorner;
    float temp;
};

static const int PLIGHT_MAX = 8;
static const int SLIGHT_MAX = 8;

struct VS_IN
{
    float4 position : POSITION;
    float3 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 uv : UV;
    float4 weights : WEIGHTS;
    uint4 bones : BONES;
};
struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 world_position : POSITION;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 binormal : BINORMAL;
    float4 colour : COLOUR;
    float2 uv : UV;
};

cbuffer CBUFFER_S : register(b0)
{
    row_major float4x4 view_proj;
    float4 viewPosition;
    float4 ambientLightColour;
    DLIGHT_DATA directional;
    PLIGHT_DATA pointlights[PLIGHT_MAX];
    SLIGHT_DATA spotlights[SLIGHT_MAX];
    int pLightCount;
    int sLightCount;
    float2 temp;
}
cbuffer CBUFFER_M : register(b1)
{
    row_major float4x4 world;
    row_major float4x4 b_Transform[256];
    float4 colour;
}
cbuffer CBUFFER_OUTLINE : register(b2)
{
    float4 outline_colour;
    float outline_size;
    float3 placeholder0;
}
cbuffer CBUFFER_UVSCROLL : register(b3)
{
    float2 scrollVal;
    float2 placeholder1;
}

VS_OUT VS_MAIN(VS_IN vin)
{
    VS_OUT vout;
    float4 n_Pos = { 0,0, 0, 1.0f };
    float4 n_Normal = { 0, 0, 0, 0 };
    float4 n_Tangent = { 0, 0, 0, 0 };
    for (int ind = 0; ind < 4; ++ind)
    {
        n_Pos += vin.weights[ind] * mul(float4(vin.position.xyz, 1.0f), b_Transform[vin.bones[ind]]);
        n_Normal += vin.weights[ind] * mul(float4(vin.normal, 0.0f), b_Transform[vin.bones[ind]]);
        n_Tangent += vin.weights[ind] * mul(float4(vin.tangent.xyz, 0.0f), b_Transform[vin.bones[ind]]);

    }
    vout.world_position = mul(float4(n_Pos.xyz, 1.0f), world);
    vout.position = mul(vout.world_position, view_proj);
    vout.tangent = mul(float4(n_Tangent.xyz, 0.0f), world).xyz;
    vout.normal = mul(float4(n_Normal.xyz, 0.0f), world).xyz;
    vout.tangent = normalize(vout.tangent).xyz;
    vout.normal = normalize(vout.normal).xyz;
    vout.uv = vin.uv + scrollVal;
    vout.colour = colour;
    vout.binormal = cross(vout.tangent, vout.normal);
    return vout;
}

Texture2D diffuseM : register(t0);
Texture2D normalM : register(t1);
SamplerState Sampler : register(s0);

float4 PS_MAIN(VS_OUT pin) : SV_TARGET
{
    float4 diffuseColour = diffuseM.Sample(Sampler, pin.uv) * pin.colour;
    float3 normal = normalM.Sample(Sampler, pin.uv);
    normal.xyz = (normal.xyz - 0.5) * 2;
    float3x3 CM = { normalize(pin.tangent), normalize(pin.binormal), normalize(pin.normal) };
    float3 N = mul(normal, CM);
    float3 L = normalize(-directional.direction.xyz);
    float3 E = normalize(viewPosition.xyz - pin.world_position);
    N = normalize(N);

    float3 kd = { 1, 1, 1 };
    float3 ks = { 1, 1, 1 };
    float3 ka = { 1, 1, 1 };
    float shineFactor = 128;

    float3 DirectionalSpecular = CalculatePhongSpecular(N, L, directional.colour.rgb, E, shineFactor, ks);
    float3 DirectionalDiffuse = CalculateLambertDiffuse(N, L, directional.colour.rgb, kd);
    float3 ambient = ka * ambientLightColour.rgb;

    float3 PointDiffuse = float3(0, 0, 0);
    float3 PointSpecular = float3(0, 0, 0);
    for (int a = 0; a < pLightCount; ++a)
    {
        float3 lightVector = pin.world_position.xyz - pointlights[a].position.xyz;
        lightVector *= -1;
        float lightLength = length(lightVector);
        if (lightLength > pointlights[a].range)
            continue;
        float weaken = max(0.0f, 1.0f - (lightLength / pointlights[a].range));
        weaken = clamp(weaken, 0.0f, 1.0f);
        weaken *= weaken;
        float3 pl_Norm = normalize(lightVector);
        PointDiffuse += CalculateLambertDiffuse(N, pl_Norm, pointlights[a].colour.rgb, kd.rgb) * weaken * pointlights[a].intensity;
        PointSpecular += CalculatePhongSpecular(N, pl_Norm, pointlights[a].colour.rgb, E, shineFactor, ks.rgb) * weaken * pointlights[a].intensity;
    }

    float3 SpotDiffuse = { 0, 0, 0 };
    float3 SpotSpecular = { 0, 0, 0 };
    for (int b = 0; b < sLightCount; ++b)
    {
        float3 lightVector = pin.world_position.xyz - spotlights[b].position;
        float lightLength = length(lightVector);
        if (lightLength > spotlights[b].range)
            continue;
        float weaken = max(0.0f, 1.0f - (lightLength / spotlights[b].range));
        weaken = clamp(weaken, 0.0f, 1.0f);

        float3 normalizedLV = normalize(lightVector);
        float angle = dot(normalizedLV, spotlights[b].direction);
        float area = spotlights[b].innerCorner - spotlights[b].outerCorner;
        float weakenAngle = clamp(1.0f - ((spotlights[b].innerCorner - angle) / area), 0.0f, 1.0f);
        weaken *= weakenAngle;

        SpotDiffuse += CalculateLambertDiffuse(N, -normalizedLV, spotlights[b].colour.rgb, kd.rgb) * weaken;
        SpotSpecular += CalculatePhongSpecular(N, -normalizedLV, spotlights[b].colour.rgb, E, shineFactor, ks.rgb) * weaken;
    }

    float4 colour = float4(ambient, diffuseColour.a);
    colour.rgb += diffuseColour.rgb * (DirectionalDiffuse + PointDiffuse + SpotDiffuse);
    colour.rgb += DirectionalSpecular + PointSpecular + SpotSpecular;
    return colour;
}