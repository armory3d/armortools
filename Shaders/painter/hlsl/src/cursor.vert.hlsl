uniform float4x4 VP;
uniform float4x4 invVP;
uniform float2 mouse;
uniform float2 step;
uniform float radius;
Texture2D<float4> texa; // direct3d12 unit align
SamplerState _texa_sampler; // direct3d12 unit align
Texture2D<float4> gbufferD;
SamplerState _gbufferD_sampler;
Texture2D<float4> gbuffer0;
SamplerState _gbuffer0_sampler;
float2 octahedronWrap(float2 v) { return (1.0 - abs(v.yx)) * (float2(v.x >= 0.0 ? 1.0 : -1.0, v.y >= 0.0 ? 1.0 : -1.0)); }
float3x3 rotAxis(float3 axis, float a) {
	float c = cos(a);
	float3 as = axis * sin(a).xxx;
	float3x3 p = float3x3(axis.xxx * axis, axis.yyy * axis, axis.zzz * axis);
	float3x3 q = float3x3(c, -as.z, as.y, as.z, c, -as.x, -as.y, as.x, c);
	return p * (1.0 - c) + q;
}
float3 getNormal(float2 uv) {
	float2 g0 = gbuffer0.SampleLevel(_gbuffer0_sampler, uv, 0.0).rg;
	float3 n;
	n.z = 1.0 - abs(g0.x) - abs(g0.y);
	n.xy = n.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);
	return n;
}
struct SPIRV_Cross_Output { float2 texCoord : TEXCOORD0; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float2 nor : TEXCOORD0, float4 pos : TEXCOORD1, float2 tex : TEXCOORD2) {
	SPIRV_Cross_Output stage_output;
	stage_output.texCoord = tex;
	float2 mouseinv = float2(mouse.x, 1.0 - mouse.y);
	float depth = gbufferD.SampleLevel(_gbufferD_sampler, mouseinv, 0).r;
	float keep = texa.SampleLevel(_texa_sampler, mouseinv, 0).r; // direct3d12 unit align
	depth += keep * 0.0000001; // direct3d12 unit align
	float4 wpos = float4(mouse * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	wpos = mul(wpos, invVP);
	wpos.xyz /= wpos.w;
	float3 n = normalize(
		getNormal(mouseinv + float2(step.x, step.y)) +
		getNormal(mouseinv + float2(-step.x, step.y)) +
		getNormal(mouseinv + float2(-step.x, -step.y)) +
		getNormal(mouseinv + float2(step.x, -step.y)) +
		getNormal(mouseinv)
	);
	float ax = acos(dot(float3(1,0,0), float3(n.x,0,0)));
	float az = acos(dot(float3(0,0,1), float3(0,0,n.z)));
	float sy = -sign(n.y);
	wpos.xyz += mul(mul(pos.xyz * radius.xxx, rotAxis(float3(0,0,1), ax + 3.14/2)),
					rotAxis(float3(1,0,0), -az * sy + 3.14/2));
	stage_output.gl_Position = mul(float4(wpos.xyz, 1.0), VP);
	stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
	return stage_output;
}
