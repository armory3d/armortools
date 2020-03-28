Texture2D<float4> tex;
SamplerState _tex_sampler;
uniform int channel;
float4 main(float4 color : TEXCOORD0, float2 texCoord : TEXCOORD1) : SV_Target0 {
	if (channel == 1) {
		return tex.SampleLevel(_tex_sampler, texCoord, 0).rrra * color;
	}
	else if (channel == 2) {
		return tex.SampleLevel(_tex_sampler, texCoord, 0).ggga * color;
	}
	else if (channel == 3) {
		return tex.SampleLevel(_tex_sampler, texCoord, 0).bbba * color;
	}
	else if (channel == 4) {
		return tex.SampleLevel(_tex_sampler, texCoord, 0).aaaa * color;
	}
	else if (channel == 5) {
		return tex.SampleLevel(_tex_sampler, texCoord, 0).rgba * color;
	}
	else {
		float4 sample = tex.SampleLevel(_tex_sampler, texCoord, 0).rgba;
		sample.rgb *= sample.a;
		return sample.rgba * color;
	}
}
