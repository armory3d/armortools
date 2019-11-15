Texture2D<float4> tex0;
SamplerState _tex0_sampler;
Texture2D<float4> texa;
SamplerState _texa_sampler;
struct SPIRV_Cross_Output { float4 color0 : SV_Target0; };
SPIRV_Cross_Output main(float2 texCoord : TEXCOORD0) {
	float4 col0 = tex0.SampleLevel(_tex0_sampler, texCoord, 0);
	float mask = texa.SampleLevel(_texa_sampler, texCoord, 0).r;
	SPIRV_Cross_Output stage_output;
	stage_output.color0 = float4(col0.rgb, col0.a * mask);
	return stage_output;
}
