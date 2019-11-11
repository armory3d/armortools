uniform float4x4 projectionMatrix;
struct SPIRV_Cross_Output { float4 color : TEXCOORD0; float2 texCoord : TEXCOORD1; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float4 col : TEXCOORD0, float3 pos : TEXCOORD1, float2 tex : TEXCOORD2) {
	SPIRV_Cross_Output stage_output;
	stage_output.gl_Position = mul(float4(pos, 1.0f), projectionMatrix);
	stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
	stage_output.texCoord = tex;
	stage_output.color = col;
	return stage_output;
}
