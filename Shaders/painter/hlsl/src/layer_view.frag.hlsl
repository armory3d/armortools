Texture2D<float4> tex;
SamplerState _tex_sampler;
float4 main(float4 color : TEXCOORD0, float2 texCoord : TEXCOORD1) : SV_Target0 {
	return tex.SampleLevel(_tex_sampler, texCoord, 0) * color;
}
