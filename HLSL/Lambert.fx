struct VS_IN
{
	float4 position : POSITION;
	float4 normal : NORMAL;
	float2 uv : UV;
	float4 tangent : TANGENT;
	float4 b_Weights : B_WEIGHTS;
	uint4 b_Indices : B_INDICES;
};

struct VS_OUT
{
	float4 position : SV_POSITION;
	float4 world_position : POSITION;
	float4 world_normal : NORMAL;
	float4 world_tangent : TANGENT;
	float4 colour : COLOR;
	float2 uv : UV;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b0)
{
	row_major float4x4 view_proj;
	float4 light_dir;
	float4 camera_pos;
};


cbuffer OBJECT_CONSTANT_BUFFER : register(b1)
{
	row_major float4x4 world;
	row_major float4x4 b_Transform[256];
	float4 colour;
};

VS_OUT VS_MAIN(VS_IN vin)
{
	VS_OUT vout;
	float sigma = vin.tangent.w;
	vin.tangent.w = 0;

	float4 n_Pos = { 0,0, 0, 1.0f };
	float4 n_Normal = { 0, 0, 0, 0 };
	float4 n_Tangent = { 0, 0, 0, 0 };
	for (int ind = 0; ind < 4; ++ind)
	{
		n_Pos += vin.b_Weights[ind] * mul(vin.position, b_Transform[vin.b_Indices[ind]]);
		n_Normal += vin.b_Weights[ind] * mul(vin.normal, b_Transform[vin.b_Indices[ind]]);
		n_Tangent += vin.b_Weights[ind] * mul(vin.tangent, b_Transform[vin.b_Indices[ind]]);

	}
	vout.world_position = mul(float4(n_Pos.xyz, 1.0f), world);
	vout.world_normal = mul(float4(n_Normal.xyz, 0.0f), world);
	vout.world_tangent = mul(float4(n_Tangent.xyz, 0.0f), world);
	vout.world_tangent.w = sigma;
	vout.position = mul(vout.world_position, view_proj);
	vout.uv = vin.uv;
	vout.colour = colour;
	return vout;
}

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states  : register(s0);
Texture2D texture_maps[4] : register(t0);
float4 PS_MAIN(VS_OUT pin) : SV_TARGET
{
	float4 coloura = texture_maps[0].Sample(sampler_states, pin.uv);
	float alpha = coloura.a;
	float3 N = normalize(pin.world_normal.xyz);
	float3 T = normalize(pin.world_tangent.xyz);
	float sigma = pin.world_tangent.w;

	T = normalize(T - dot(N, T));
	float3 B = normalize(cross(N, T) * sigma);

	float4 normal = texture_maps[1].Sample(sampler_states, pin.uv);
	normal = normal * 2.0f - 1.0f;
	N = normalize((normal.x * T) + (normal.y * B) + (normal.z * N));

	float3 L = normalize(-light_dir.xyz);
	float3 diffuse = coloura.rgb * max(0, dot(N, L));
	float3 V = normalize(camera_pos.xyz - pin.world_position.xyz);
	float3 spec = pow(max(0, dot(N, normalize(V + L))), 128);
	return coloura * pin.colour;
	return float4(diffuse + spec, alpha) * pin.colour;
	return coloura * (float4(diffuse, 1) * pin.colour);
}