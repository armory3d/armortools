	uniform float4x4 W;
	struct SPIRV_Cross_Input { float4 pos : TEXCOORD0; };
	struct SPIRV_Cross_Output { float4 svpos : SV_POSITION; };
	SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {
	  SPIRV_Cross_Output stage_output;
	  stage_output.svpos.xyz = mul(float4(stage_input.pos.xyz, 1.0), W).xyz / float3(1, 1, 1);
	  stage_output.svpos.w = 1.0;
	  return stage_output;
	}
