	struct SPIRV_Cross_Input { float4 svpos : SV_POSITION; };
	struct SPIRV_Cross_Output { float3 wpos : TEXCOORD0; float4 svpos : SV_POSITION; };
	[maxvertexcount(3)]
	void main(triangle SPIRV_Cross_Input stage_input[3], inout TriangleStream<SPIRV_Cross_Output> output) {
	  float3 p1 = stage_input[1].svpos.xyz - stage_input[0].svpos.xyz;
	  float3 p2 = stage_input[2].svpos.xyz - stage_input[0].svpos.xyz;
	  float3 p = abs(cross(p1, p2));
	  for (int i = 0; i < 3; ++i) {
	    SPIRV_Cross_Output stage_output;
	    stage_output.wpos = stage_input[i].svpos.xyz;
	    if (p.z > p.x && p.z > p.y) {
	      stage_output.svpos = float4(stage_input[i].svpos.x, stage_input[i].svpos.y, 0.0, 1.0);
	    }
	    else if (p.x > p.y && p.x > p.z) {
	      stage_output.svpos = float4(stage_input[i].svpos.y, stage_input[i].svpos.z, 0.0, 1.0);
	    }
	    else {
	      stage_output.svpos = float4(stage_input[i].svpos.x, stage_input[i].svpos.z, 0.0, 1.0);
	    }
	    output.Append(stage_output);
	  }
	}
