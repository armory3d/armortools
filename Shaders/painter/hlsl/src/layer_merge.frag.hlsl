Texture2D<float4> tex0;
SamplerState _tex0_sampler;
Texture2D<float4> tex1;
SamplerState _tex1_sampler;
Texture2D<float4> texmask;
SamplerState _texmask_sampler;
Texture2D<float4> texa;
SamplerState _texa_sampler;
uniform float opac;
uniform int blending;
float3 hsv_to_rgb(const float3 c) {
	const float4 K = float4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	float3 p = abs(frac(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * lerp(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}
float3 rgb_to_hsv(const float3 c) {
	const float4 K = float4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	float4 p = lerp(float4(c.bg, K.wz), float4(c.gb, K.xy), step(c.b, c.g));
	float4 q = lerp(float4(p.xyw, c.r), float4(c.r, p.yzx), step(p.x, c.r));
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return float3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}
struct SPIRV_Cross_Output { float4 color0 : SV_Target0; };
SPIRV_Cross_Output main(float2 texCoord : TEXCOORD0) {
	float4 col0 = tex0.SampleLevel(_tex0_sampler, texCoord, 0);
	float4 cola = texa.SampleLevel(_texa_sampler, texCoord, 0);
	float str = col0.a * opac;
	str *= texmask.SampleLevel(_texmask_sampler, texCoord, 0).r;
	SPIRV_Cross_Output stage_output;
	if (blending == -1) { // Merging _nor and _pack
		float4 col1 = tex1.SampleLevel(_tex1_sampler, texCoord, 0);
		stage_output.color0 = float4(lerp(cola, col1, str));
	}
	else if (blending == 0) { // Mix
		stage_output.color0 = float4(lerp(cola.rgb, col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 1) { // Darken
		stage_output.color0 = float4(lerp(cola.rgb, min(cola.rgb, col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 2) { // Multiply
		stage_output.color0 = float4(lerp(cola.rgb, cola.rgb * col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 3) { // Burn
		stage_output.color0 = float4(lerp(cola.rgb, float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - cola.rgb) / col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 4) { // Lighten
		stage_output.color0 = float4(max(cola.rgb, col0.rgb * str), max(col0.a, cola.a));
	}
	else if (blending == 5) { // Screen
		stage_output.color0 = float4((float3(1.0, 1.0, 1.0) - (float3(1.0 - str, 1.0 - str, 1.0 - str) + str * (float3(1.0, 1.0, 1.0) - col0.rgb)) * (float3(1.0, 1.0, 1.0) - cola.rgb)), max(col0.a, cola.a));
	}
	else if (blending == 6) { // Dodge
		stage_output.color0 = float4(lerp(cola.rgb, cola.rgb / (float3(1.0, 1.0, 1.0) - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 7) { // Add
		stage_output.color0 = float4(lerp(cola.rgb, cola.rgb + col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 8) { // Overlay
		stage_output.color0 = float4(lerp(cola.rgb, (cola.rgb < float3(0.5, 0.5, 0.5) ? float3(2.0, 2.0, 2.0) * cola.rgb * col0.rgb : float3(1.0, 1.0, 1.0) - float3(2.0, 2.0, 2.0) * (float3(1.0, 1.0, 1.0) - col0.rgb) * (float3(1.0, 1.0, 1.0) - cola.rgb)), str), max(col0.a, cola.a));
	}
	else if (blending == 9) { // Soft Light
		stage_output.color0 = float4(((1.0 - str) * cola.rgb + str * ((float3(1.0, 1.0, 1.0) - cola.rgb) * col0.rgb * cola.rgb + cola.rgb * (float3(1.0, 1.0, 1.0) - (float3(1.0, 1.0, 1.0) - col0.rgb) * (float3(1.0, 1.0, 1.0) - cola.rgb)))), max(col0.a, cola.a));
	}
	else if (blending == 10) { // Linear Light
		stage_output.color0 = float4((cola.rgb + str * (float3(2.0, 2.0, 2.0) * (col0.rgb - float3(0.5, 0.5, 0.5)))), max(col0.a, cola.a));
	}
	else if (blending == 11) { // Difference
		stage_output.color0 = float4(lerp(cola.rgb, abs(cola.rgb - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 12) { // Subtract
		stage_output.color0 = float4(lerp(cola.rgb, cola.rgb - col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 13) { // Divide
		stage_output.color0 = float4(float3(1.0 - str, 1.0 - str, 1.0 - str) * cola.rgb + float3(str, str, str) * cola.rgb / col0.rgb, max(col0.a, cola.a));
	}
	else if (blending == 14) { // Hue
		stage_output.color0 = float4(lerp(cola.rgb, hsv_to_rgb(float3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 15) { // Saturation
		stage_output.color0 = float4(lerp(cola.rgb, hsv_to_rgb(float3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 16) { // Color
		stage_output.color0 = float4(lerp(cola.rgb, hsv_to_rgb(float3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else { // Value
		stage_output.color0 = float4(lerp(cola.rgb, hsv_to_rgb(float3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(col0.rgb).b)), str), max(col0.a, cola.a));
	}
	return stage_output;
}
