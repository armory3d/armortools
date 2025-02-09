
function make_clone_run(vert: node_shader_t, frag: node_shader_t) {
	node_shader_add_uniform(frag, "vec2 clone_delta", "_clone_delta");
	node_shader_write(frag, "vec2 tex_coord_inp = texelFetch(gbuffer2, ivec2((sp.xy + clone_delta) * gbuffer_size), 0).ba;");

	node_shader_write(frag, "vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, tex_coord_inp, 0.0).rgb;");
	let base: string = "textureLod(texpaint_undo, tex_coord_inp, 0.0).rgb";
	let rough: string = "texpaint_pack_sample.g";
	let met: string = "texpaint_pack_sample.b";
	let occ: string = "texpaint_pack_sample.r";
	let nortan: string = "textureLod(texpaint_nor_undo, tex_coord_inp, 0.0).rgb";
	let height: string = "0.0";
	let opac: string = "1.0";
	node_shader_write(frag, "vec3 basecol = " + base + ";");
	node_shader_write(frag, "float roughness = " + rough + ";");
	node_shader_write(frag, "float metallic = " + met + ";");
	node_shader_write(frag, "float occlusion = " + occ + ";");
	node_shader_write(frag, "vec3 nortan = " + nortan + ";");
	node_shader_write(frag, "float height = " + height + ";");
	node_shader_write(frag, "float mat_opacity = " + opac + ";");
	node_shader_write(frag, "float opacity = mat_opacity * brush_opacity;");
	if (context_raw.material.paint_emis) {
		node_shader_write(frag, "float emis = 0.0;");
	}
	if (context_raw.material.paint_subs) {
		node_shader_write(frag, "float subs = 0.0;");
	}
}
