Texture2D<float4> tex;
SamplerState _tex_sampler;
float4 main(float2 texCoord : TEXCOORD0) : SV_Target0 {
	float4 col = tex.Sample(_tex_sampler, texCoord);
	return float4(col.rgb / col.a, col.a);
}
