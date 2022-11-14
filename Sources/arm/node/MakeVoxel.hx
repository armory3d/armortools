package arm.node;

class MakeVoxel {

	#if rp_voxels
	public static function run(data: iron.data.ShaderData.ShaderContext) {
		var structure = new kha.graphics4.VertexStructure();
		structure.add("pos", kha.graphics4.VertexData.Short4Norm);
		structure.add("nor", kha.graphics4.VertexData.Short2Norm);
		structure.add("tex", kha.graphics4.VertexData.Short2Norm);

		var pipeState = data.pipeState;
		pipeState.inputLayout = [structure];
		data.raw.vertex_elements = [{name: "pos", data: "short4norm"}, {name: "nor", data: "short2norm"}, {name: "tex", data: "short2norm"}];

		// #if arm_skin
		// var isMesh = Std.isOfType(Context.object, MeshObject);
		// var skin = isMesh && cast(Context.object, MeshObject).data.geom.bones != null;
		// if (skin) {
		// 	structure.add("bone", kha.graphics4.VertexData.Short4Norm);
		// 	structure.add("weight", kha.graphics4.VertexData.Short4Norm);
		// 	data.raw.vertex_elements.push({ name: "bone", data: 'short4norm' });
		// 	data.raw.vertex_elements.push({ name: "weight", data: 'short4norm' });
		// }
		// #end

		var ds = MakeMaterial.getDisplaceStrength();
		pipeState.vertexShader = kha.graphics4.VertexShader.fromSource(
		#if kha_direct3d11
		"#define vec3 float3
		uniform float4x4 W;
		uniform float3x3 N;
		Texture2D<float4> texpaint_pack;
		SamplerState _texpaint_pack_sampler;
		struct SPIRV_Cross_Input { float4 pos : TEXCOORD1; float2 nor : TEXCOORD0; float2 tex : TEXCOORD2; };
		struct SPIRV_Cross_Output { float4 svpos : SV_POSITION; };
		SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {
			SPIRV_Cross_Output stage_output;
			" + MakeMaterial.voxelgiHalfExtents() + "
			stage_output.svpos.xyz = mul(float4(stage_input.pos.xyz, 1.0), W).xyz / voxelgiHalfExtents.xxx;
			float3 wnormal = normalize(mul(float3(stage_input.nor.xy, stage_input.pos.w), N));
			float height = texpaint_pack.SampleLevel(_texpaint_pack_sampler, stage_input.tex, 0.0).a;
			stage_output.svpos.xyz += wnormal * height.xxx * float3(" + ds + "," + ds + "," + ds + ");
			stage_output.svpos.w = 1.0;
			return stage_output;
		}"
		#else
		"#version 450
		in vec4 pos;
		in vec2 nor;
		in vec2 tex;
		out vec3 voxpositionGeom;
		uniform mat4 W;
		uniform mat3 N;
		uniform sampler2D texpaint_pack;
		void main() {
			" + MakeMaterial.voxelgiHalfExtents() + "
			voxpositionGeom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgiHalfExtents;
			vec3 wnormal = normalize(N * vec3(nor.xy, pos.w));
			float height = textureLod(texpaint_pack, tex, 0.0).a;
			voxpositionGeom += wnormal * vec3(height) * vec3(" + ds + ");
		}"
		#end
		);

		pipeState.compile();
		data.raw.constants = [{ name: "W", type: "mat4", link: "_worldMatrix" }, { name: "N", type: "mat3", link: "_normalMatrix" }];
		data.constants = [pipeState.getConstantLocation("W"), pipeState.getConstantLocation("N")];
		data.raw.texture_units = [{ name: "texpaint_pack" }, { name: "voxels", is_image: true }];
		data.textureUnits = [pipeState.getTextureUnit("texpaint_pack"), pipeState.getTextureUnit("voxels")];
	}
	#end
}
