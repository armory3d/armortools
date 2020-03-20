uniform float4x4 VP;
uniform float4x4 invVP;
uniform float2 mouse;
uniform float2 texStep;
uniform float radius;
uniform float3 cameraRight;
Texture2D<float4> texa; // direct3d12 unit align
SamplerState _texa_sampler; // direct3d12 unit align
Texture2D<float4> gbufferD;
SamplerState _gbufferD_sampler;

float3 getPos(float2 uv) {
	float2 uvinv = float2(uv.x, 1.0 - uv.y);
	float depth = gbufferD.SampleLevel(_gbufferD_sampler, uvinv, 0).r;
	float4 wpos = float4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	wpos = mul(wpos, invVP);
	return wpos.xyz / wpos.w;
}

float3 getNormal(float3 p0, float2 uv) {
	float3 p1 = getPos(uv + float2(texStep.x, 0));
	float3 p2 = getPos(uv + float2(0, texStep.y));
	return normalize(cross(p2 - p0, p1 - p0));
}

void createBasis(float3 normal, out float3 tangent, out float3 binormal) {
	// Project vector onto plane
	tangent = normalize(cameraRight - normal * dot(cameraRight, normal));
	binormal = cross(tangent, normal);
}

struct SPIRV_Cross_Output { float2 texCoord : TEXCOORD0; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float2 nor : TEXCOORD0, float4 pos : TEXCOORD1, float2 tex : TEXCOORD2, uint gl_VertexID : SV_VertexID) {
	SPIRV_Cross_Output stage_output;
	stage_output.texCoord = tex;
	float3 wpos = getPos(mouse);
	float2 uv1 = mouse + texStep * 4;
	float2 uv2 = mouse - texStep * 4;
	float3 wpos1 = getPos(uv1);
	float3 wpos2 = getPos(uv2);
	float keep = texa.SampleLevel(_texa_sampler, mouse, 0).r; // direct3d12 unit align
	wpos.x += keep * 0.0000001; // direct3d12 unit align;
	float3 n = normalize(
		getNormal(wpos, mouse) +
		getNormal(wpos1, uv1) +
		getNormal(wpos2, uv2)
	);

	float3 n_tan;
	float3 n_bin;
	createBasis(n, n_tan, n_bin);
	if      (gl_VertexID == 0) wpos += normalize(-n_tan - n_bin) * 0.7 * radius;
	else if (gl_VertexID == 1) wpos += normalize( n_tan - n_bin) * 0.7 * radius;
	else if (gl_VertexID == 2) wpos += normalize( n_tan + n_bin) * 0.7 * radius;
	else if (gl_VertexID == 3) wpos += normalize(-n_tan + n_bin) * 0.7 * radius;

	stage_output.gl_Position = mul(float4(wpos, 1.0), VP);
	stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
	return stage_output;
}
