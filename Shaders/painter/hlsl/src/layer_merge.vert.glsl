struct SPIRV_Cross_Output { float2 texCoord : TEXCOORD0; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float2 pos : TEXCOORD0) {
	SPIRV_Cross_Output stage_output;
	stage_output.gl_Position = float4(pos.xy, 0.0, 1.0);
	stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
	const float2 madd = float2(0.5, 0.5);
	stage_output.texCoord = pos.xy * madd + madd;
	stage_output.texCoord.y = 1.0 - stage_output.texCoord.y;
	return stage_output;
}
