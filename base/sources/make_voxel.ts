
///if arm_voxels
function make_voxel_run(data: shader_context_t) {
	let structure: vertex_struct_t = g4_vertex_struct_create();
	g4_vertex_struct_add(structure, "pos", vertex_data_t.I16_4X_NORM);
	g4_vertex_struct_add(structure, "nor", vertex_data_t.I16_2X_NORM);
	g4_vertex_struct_add(structure, "tex", vertex_data_t.I16_2X_NORM);

	let pipe_state: pipeline_t = data._.pipe_state;
	pipe_state.input_layout = [structure];

	data.vertex_elements = [
		{
			name: "pos",
			data: "short4norm"
		},
		{
			name: "nor",
			data: "short2norm"
		},
		{
			name: "tex",
			data: "short2norm"
		}
	];

	// ///if arm_skin
	// let is_mesh: bool = raw.object.constructor == mesh_object_t;
	// let skin: bool = is_mesh && cast(raw.object, mesh_object_t).data.geom.bones != null;
	// if (skin) {
	// 	g4_vertex_structure_add(structure, "bone", vertex_data_t.I16_4X_Normalized);
	// 	g4_vertex_structure_add(structure, "weight", vertex_data_t.I16_4X_Normalized);
	// 	array_push(data.raw.vertex_elements, { name: "bone", data: "short4norm" });
	// 	array_push(data.raw.vertex_elements, { name: "weight", data: "short4norm" });
	// }
	// ///end

	pipe_state.vertex_shader = g4_shader_from_source(make_voxel_source(), shader_type_t.VERTEX);

	g4_pipeline_compile(pipe_state);

	data.constants = [
		{
			name: "W",
			type: "mat4",
			link: "_world_matrix"
		},
		{
			name: "N",
			type: "mat3",
			link: "_normal_matrix"
		}
	];
	data._.constants = [
		g4_pipeline_get_const_loc(pipe_state, "W"),
		g4_pipeline_get_const_loc(pipe_state, "N")
	];
	data.texture_units = [
		{
			name: "texpaint_pack"
		},
		{
			name: "voxels",
			image_uniform: true
		}
	];
	data._.tex_units = [
		g4_pipeline_get_tex_unit(pipe_state, "texpaint_pack"),
		g4_pipeline_get_tex_unit(pipe_state, "voxels")
	];
}

function make_voxel_source(): string {
	let ds: f32 = make_material_get_displace_strength();
	///if arm_direct3d11
	return "#define vec3 float3 \n\
	uniform float4x4 W; \
	uniform float3x3 N; \
	Texture2D<float4> texpaint_pack; \
	SamplerState _texpaint_pack_sampler; \
	struct SPIRV_Cross_Input { float4 pos : TEXCOORD1; float2 nor : TEXCOORD0; float2 tex : TEXCOORD2; }; \
	struct SPIRV_Cross_Output { float4 svpos : SV_POSITION; }; \
	SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) { \
		SPIRV_Cross_Output stage_output; \
		" + make_material_voxelgi_half_extents() + " \
		stage_output.svpos.xyz = mul(float4(stage_input.pos.xyz, 1.0), W).xyz / voxelgi_half_extents.xxx; \
		float3 wnormal = normalize(mul(float3(stage_input.nor.xy, stage_input.pos.w), N)); \
		float height = texpaint_pack.SampleLevel(_texpaint_pack_sampler, stage_input.tex, 0.0).a; \
		stage_output.svpos.xyz += wnormal * height.xxx * float3(" + ds + ", " + ds + ", " + ds + "); \
		stage_output.svpos.w = 1.0; \
		return stage_output; \
	}";
	///else
	return "#version 450 \n\
	in vec4 pos; \
	in vec2 nor; \
	in vec2 tex; \
	out vec3 voxposition_geom; \
	uniform mat4 W; \
	uniform mat3 N; \
	uniform sampler2D texpaint_pack; \
	void main() { \
		" + make_material_voxelgi_half_extents() + " \
		voxposition_geom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgi_half_extents; \
		vec3 wnormal = normalize(N * vec3(nor.xy, pos.w)); \
		float height = textureLod(texpaint_pack, tex, 0.0).a; \
		voxposition_geom += wnormal * vec3(height) * vec3(" + ds + "); \
	}";
	///end
}
///end
