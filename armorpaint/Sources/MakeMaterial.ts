
class MakeMaterial {

	static default_scon: shader_context_t = null;
	static default_mcon: material_context_t = null;

	static height_used = false;
	static emis_used = false;
	static subs_used = false;

	static get_mout = (): bool => {
		for (let n of UINodes.get_canvas_material().nodes) if (n.type == "OUTPUT_MATERIAL_PBR") return true;
		return false;
	}

	static parse_mesh_material = () => {
		let m: material_data_t = project_materials[0].data;

		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				array_remove(m._.shader.contexts, c);
				array_remove(m._.shader._.contexts, c);
				MakeMaterial.delete_context(c);
				break;
			}
		}

		if (MakeMesh.layer_pass_count > 1) {
			let i: i32 = 0;
			while (i < m._.shader._.contexts.length) {
				let c: shader_context_t = m._.shader._.contexts[i];
				for (let j: i32 = 1; j < MakeMesh.layer_pass_count; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m._.shader.contexts, c);
						array_remove(m._.shader._.contexts, c);
						MakeMaterial.delete_context(c);
						i--;
						break;
					}
				}
				i++;
			}

			i = 0;
			while (i < m._.contexts.length) {
				let c: material_context_t = m._.contexts[i];
				for (let j: i32 = 1; j < MakeMesh.layer_pass_count; ++j) {
					if (c.name == "mesh" + j) {
						array_remove(m.contexts, c);
						array_remove(m._.contexts, c);
						i--;
						break;
					}
				}
				i++;
			}
		}

		let con: NodeShaderContextRaw = MakeMesh.run({ name: "Material", canvas: null });
		let scon: shader_context_t = shader_context_create(con.data);
		scon._.override_context = {};
		if (con.frag.shared_samplers.length > 0) {
			let sampler: string = con.frag.shared_samplers[0];
			scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
		}
		if (!context_raw.texture_filter) {
			scon._.override_context.filter = "point";
		}
		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);

		for (let i: i32 = 1; i < MakeMesh.layer_pass_count; ++i) {
			let con: NodeShaderContextRaw = MakeMesh.run({ name: "Material", canvas: null }, i);
			let scon: shader_context_t = shader_context_create(con.data);
			scon._.override_context = {};
			if (con.frag.shared_samplers.length > 0) {
				let sampler: string = con.frag.shared_samplers[0];
				scon._.override_context.shared_sampler = sampler.substr(sampler.lastIndexOf(" ") + 1);
			}
			if (!context_raw.texture_filter) {
				scon._.override_context.filter = "point";
			}
			m._.shader.contexts.push(scon);
			m._.shader._.contexts.push(scon);

			let mcon: material_context_t;
			mcon = material_context_create({ name: "mesh" + i, bind_textures: [] });
			m.contexts.push(mcon);
			m._.contexts.push(mcon);
		}

		context_raw.ddirty = 2;

		///if arm_voxels
		MakeMaterial.make_voxel(m);
		///end

		///if (krom_direct3d12 || krom_vulkan || krom_metal)
		RenderPathRaytrace.dirty = 1;
		///end
	}

	static parse_particle_material = () => {
		let m: material_data_t = context_raw.particle_material;
		let sc: shader_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				sc = c;
				break;
			}
		}
		if (sc != null) {
			array_remove(m._.shader.contexts, sc);
			array_remove(m._.shader._.contexts, sc);
		}
		let con: NodeShaderContextRaw = MakeParticle.run({ name: "MaterialParticle", canvas: null });
		if (sc != null) MakeMaterial.delete_context(sc);
		sc = shader_context_create(con.data);
		m._.shader.contexts.push(sc);
		m._.shader._.contexts.push(sc);
	}

	static parse_mesh_preview_material = (md: material_data_t = null) => {
		if (!MakeMaterial.get_mout()) return;

		let m: material_data_t = md == null ? project_materials[0].data : md;
		let scon: shader_context_t = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "mesh") {
				scon = c;
				break;
			}
		}

		array_remove(m._.shader.contexts, scon);
		array_remove(m._.shader._.contexts, scon);

		let mcon: material_context_t = { name: "mesh", bind_textures: [] };

		let sd: material_t = { name: "Material", canvas: null };
		let con: NodeShaderContextRaw = MakeMeshPreview.run(sd, mcon);

		for (let i: i32 = 0; i < m._.contexts.length; ++i) {
			if (m._.contexts[i].name == "mesh") {
				m._.contexts[i] = material_context_create(mcon);
				break;
			}
		}

		if (scon != null) MakeMaterial.delete_context(scon);

		let compile_error: bool = false;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compile_error = true;
		scon = _scon;
		if (compile_error) return;

		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);
	}

	///if arm_voxels
	static make_voxel = (m: material_data_t) => {
		let rebuild: bool = MakeMaterial.height_used;
		if (config_raw.rp_gi != false && rebuild) {
			let scon: shader_context_t = null;
			for (let c of m._.shader._.contexts) {
				if (c.name == "voxel") {
					scon = c;
					break;
				}
			}
			if (scon != null) make_voxel_run(scon);
		}
	}
	///end

	static parse_paint_material = (bake_previews = true) => {
		if (!MakeMaterial.get_mout()) return;

		if (bake_previews) {
			let current: image_t = _g2_current;
			if (current != null) g2_end();
			MakeMaterial.bake_node_previews();
			if (current != null) g2_begin(current);
		}

		let m: material_data_t = project_materials[0].data;
		// let scon: TShaderContext = null;
		// let mcon: TMaterialContext = null;
		for (let c of m._.shader._.contexts) {
			if (c.name == "paint") {
				array_remove(m._.shader.contexts, c);
				array_remove(m._.shader._.contexts, c);
				if (c != MakeMaterial.default_scon) MakeMaterial.delete_context(c);
				break;
			}
		}
		for (let c of m._.contexts) {
			if (c.name == "paint") {
				array_remove(m.contexts, c);
				array_remove(m._.contexts, c);
				break;
			}
		}

		let sdata: material_t = { name: "Material", canvas: UINodes.get_canvas_material() };
		let tmcon: material_context_t = { name: "paint", bind_textures: [] };
		let con: NodeShaderContextRaw = MakePaint.run(sdata, tmcon);

		let compile_error: bool = false;
		let scon: shader_context_t;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compile_error = true;
		scon = _scon;
		if (compile_error) return;
		scon._.override_context = {};
		scon._.override_context.addressing = "repeat";
		let mcon: material_context_t = material_context_create(tmcon);

		m._.shader.contexts.push(scon);
		m._.shader._.contexts.push(scon);
		m.contexts.push(mcon);
		m._.contexts.push(mcon);

		if (MakeMaterial.default_scon == null) MakeMaterial.default_scon = scon;
		if (MakeMaterial.default_mcon == null) MakeMaterial.default_mcon = mcon;
	}

	static bake_node_previews = () => {
		context_raw.node_previews_used = [];
		if (context_raw.node_previews == null) context_raw.node_previews = new Map();
		MakeMaterial.traverse_nodes(UINodes.get_canvas_material().nodes, null, []);
		for (let key of context_raw.node_previews.keys()) {
			if (context_raw.node_previews_used.indexOf(key) == -1) {
				let image: image_t = context_raw.node_previews.get(key);
				base_notify_on_next_frame(function() { image_unload(image); });
				context_raw.node_previews.delete(key);
			}
		}
	}

	static traverse_nodes = (nodes: zui_node_t[], group: zui_node_canvas_t, parents: zui_node_t[]) => {
		for (let node of nodes) {
			MakeMaterial.bake_node_preview(node, group, parents);
			if (node.type == "GROUP") {
				for (let g of project_material_groups) {
					if (g.canvas.name == node.name) {
						parents.push(node);
						MakeMaterial.traverse_nodes(g.canvas.nodes, g.canvas, parents);
						parents.pop();
						break;
					}
				}
			}
		}
	}

	static bake_node_preview = (node: zui_node_t, group: zui_node_canvas_t, parents: zui_node_t[]) => {
		if (node.type == "BLUR") {
			let id: string = ParserMaterial.node_name(node, parents);
			let image: image_t = context_raw.node_previews.get(id);
			context_raw.node_previews_used.push(id);
			let resX: i32 = Math.floor(config_get_texture_res_x() / 4);
			let resY: i32 = Math.floor(config_get_texture_res_y() / 4);
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				context_raw.node_previews.set(id, image);
			}

			ParserMaterial.blur_passthrough = true;
			UtilRender.make_node_preview(UINodes.get_canvas_material(), node, image, group, parents);
			ParserMaterial.blur_passthrough = false;
		}
		else if (node.type == "DIRECT_WARP") {
			let id: string = ParserMaterial.node_name(node, parents);
			let image: image_t = context_raw.node_previews.get(id);
			context_raw.node_previews_used.push(id);
			let resX: i32 = Math.floor(config_get_texture_res_x());
			let resY: i32 = Math.floor(config_get_texture_res_y());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY);
				context_raw.node_previews.set(id, image);
			}

			ParserMaterial.warp_passthrough = true;
			UtilRender.make_node_preview(UINodes.get_canvas_material(), node, image, group, parents);
			ParserMaterial.warp_passthrough = false;
		}
		else if (node.type == "BAKE_CURVATURE") {
			let id: string = ParserMaterial.node_name(node, parents);
			let image: image_t = context_raw.node_previews.get(id);
			context_raw.node_previews_used.push(id);
			let resX: i32 = Math.floor(config_get_texture_res_x());
			let resY: i32 = Math.floor(config_get_texture_res_y());
			if (image == null || image.width != resX || image.height != resY) {
				if (image != null) image_unload(image);
				image = image_create_render_target(resX, resY, tex_format_t.R8);
				context_raw.node_previews.set(id, image);
			}

			if (RenderPathPaint.live_layer == null) {
				RenderPathPaint.live_layer = SlotLayer.create("_live");
			}

			let _space: i32 = UIHeader.worktab.position;
			let _tool: workspace_tool_t = context_raw.tool;
			let _bake_type: bake_type_t = context_raw.bake_type;
			UIHeader.worktab.position = space_type_t.SPACE3D;
			context_raw.tool = workspace_tool_t.BAKE;
			context_raw.bake_type = bake_type_t.CURVATURE;

			ParserMaterial.bake_passthrough = true;
			ParserMaterial.start_node = node;
			ParserMaterial.start_group = group;
			ParserMaterial.start_parents = parents;
			MakeMaterial.parse_paint_material(false);
			ParserMaterial.bake_passthrough = false;
			ParserMaterial.start_node = null;
			ParserMaterial.start_group = null;
			ParserMaterial.start_parents = null;
			context_raw.pdirty = 1;
			RenderPathPaint.use_live_layer(true);
			RenderPathPaint.commands_paint(false);
			RenderPathPaint.dilate(true, false);
			RenderPathPaint.use_live_layer(false);
			context_raw.pdirty = 0;

			UIHeader.worktab.position = _space;
			context_raw.tool = _tool;
			context_raw.bake_type = _bake_type;
			MakeMaterial.parse_paint_material(false);

			let rts: Map<string, render_target_t> = render_path_render_targets;
			let texpaint_live: render_target_t = rts.get("texpaint_live");

			g2_begin(image);
			g2_draw_image(texpaint_live._image, 0, 0);
			g2_end();
		}
	}

	static parse_node_preview_material = (node: zui_node_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null): { scon: shader_context_t, mcon: material_context_t } => {
		if (node.outputs.length == 0) return null;
		let sdata: material_t = { name: "Material", canvas: UINodes.get_canvas_material() };
		let mcon_raw: material_context_t = { name: "mesh", bind_textures: [] };
		let con: NodeShaderContextRaw = MakeNodePreview.run(sdata, mcon_raw, node, group, parents);
		let compile_error: bool = false;
		let scon: shader_context_t;
		let _scon: shader_context_t = shader_context_create(con.data);
		if (_scon == null) compile_error = true;
		scon = _scon;
		if (compile_error) return null;
		let mcon: material_context_t = material_context_create(mcon_raw);
		return { scon: scon, mcon: mcon };
	}

	static parse_brush = () => {
		ParserLogic.parse(context_raw.brush.canvas);
	}

	static blend_mode = (frag: NodeShaderRaw, blending: i32, cola: string, colb: string, opac: string): string => {
		if (blending == blend_type_t.MIX) {
			return `mix(${cola}, ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.DARKEN) {
			return `mix(${cola}, min(${cola}, ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.MULTIPLY) {
			return `mix(${cola}, ${cola} * ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.BURN) {
			return `mix(${cola}, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${cola}) / ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.LIGHTEN) {
			return `max(${cola}, ${colb} * ${opac})`;
		}
		else if (blending == blend_type_t.SCREEN) {
			return `(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - ${opac}, 1.0 - ${opac}, 1.0 - ${opac}) + ${opac} * (vec3(1.0, 1.0, 1.0) - ${colb})) * (vec3(1.0, 1.0, 1.0) - ${cola}))`;
		}
		else if (blending == blend_type_t.DODGE) {
			return `mix(${cola}, ${cola} / (vec3(1.0, 1.0, 1.0) - ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.ADD) {
			return `mix(${cola}, ${cola} + ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.OVERLAY) {
			return `mix(${cola}, vec3(
				${cola}.r < 0.5 ? 2.0 * ${cola}.r * ${colb}.r : 1.0 - 2.0 * (1.0 - ${cola}.r) * (1.0 - ${colb}.r),
				${cola}.g < 0.5 ? 2.0 * ${cola}.g * ${colb}.g : 1.0 - 2.0 * (1.0 - ${cola}.g) * (1.0 - ${colb}.g),
				${cola}.b < 0.5 ? 2.0 * ${cola}.b * ${colb}.b : 1.0 - 2.0 * (1.0 - ${cola}.b) * (1.0 - ${colb}.b)
			), ${opac})`;
		}
		else if (blending == blend_type_t.SOFT_LIGHT) {
			return `((1.0 - ${opac}) * ${cola} + ${opac} * ((vec3(1.0, 1.0, 1.0) - ${cola}) * ${colb} * ${cola} + ${cola} * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - ${colb}) * (vec3(1.0, 1.0, 1.0) - ${cola}))))`;
		}
		else if (blending == blend_type_t.LINEAR_LIGHT) {
			return `(${cola} + ${opac} * (vec3(2.0, 2.0, 2.0) * (${colb} - vec3(0.5, 0.5, 0.5))))`;
		}
		else if (blending == blend_type_t.DIFFERENCE) {
			return `mix(${cola}, abs(${cola} - ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.SUBTRACT) {
			return `mix(${cola}, ${cola} - ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.DIVIDE) {
			return `vec3(1.0 - ${opac}, 1.0 - ${opac}, 1.0 - ${opac}) * ${cola} + vec3(${opac}, ${opac}, ${opac}) * ${cola} / ${colb}`;
		}
		else if (blending == blend_type_t.HUE) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${colb}).r, rgb_to_hsv(${cola}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else if (blending == blend_type_t.SATURATION) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${cola}).r, rgb_to_hsv(${colb}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else if (blending == blend_type_t.COLOR) {
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${colb}).r, rgb_to_hsv(${colb}).g, rgb_to_hsv(${cola}).b)), ${opac})`;
		}
		else { // BlendValue
			NodeShader.add_function(frag, ShaderFunctions.str_hue_sat);
			return `mix(${cola}, hsv_to_rgb(vec3(rgb_to_hsv(${cola}).r, rgb_to_hsv(${cola}).g, rgb_to_hsv(${colb}).b)), ${opac})`;
		}
	}

	static blend_mode_mask = (frag: NodeShaderRaw, blending: i32, cola: string, colb: string, opac: string): string => {
		if (blending == blend_type_t.MIX) {
			return `mix(${cola}, ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.DARKEN) {
			return `mix(${cola}, min(${cola}, ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.MULTIPLY) {
			return `mix(${cola}, ${cola} * ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.BURN) {
			return `mix(${cola}, 1.0 - (1.0 - ${cola}) / ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.LIGHTEN) {
			return `max(${cola}, ${colb} * ${opac})`;
		}
		else if (blending == blend_type_t.SCREEN) {
			return `(1.0 - ((1.0 - ${opac}) + ${opac} * (1.0 - ${colb})) * (1.0 - ${cola}))`;
		}
		else if (blending == blend_type_t.DODGE) {
			return `mix(${cola}, ${cola} / (1.0 - ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.ADD) {
			return `mix(${cola}, ${cola} + ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.OVERLAY) {
			return `mix(${cola}, ${cola} < 0.5 ? 2.0 * ${cola} * ${colb} : 1.0 - 2.0 * (1.0 - ${cola}) * (1.0 - ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.SOFT_LIGHT) {
			return `((1.0 - ${opac}) * ${cola} + ${opac} * ((1.0 - ${cola}) * ${colb} * ${cola} + ${cola} * (1.0 - (1.0 - ${colb}) * (1.0 - ${cola}))))`;
		}
		else if (blending == blend_type_t.LINEAR_LIGHT) {
			return `(${cola} + ${opac} * (2.0 * (${colb} - 0.5)))`;
		}
		else if (blending == blend_type_t.DIFFERENCE) {
			return `mix(${cola}, abs(${cola} - ${colb}), ${opac})`;
		}
		else if (blending == blend_type_t.SUBTRACT) {
			return `mix(${cola}, ${cola} - ${colb}, ${opac})`;
		}
		else if (blending == blend_type_t.DIVIDE) {
			return `(1.0 - ${opac}) * ${cola} + ${opac} * ${cola} / ${colb}`;
		}
		else { // BlendHue, BlendSaturation, BlendColor, BlendValue
			return `mix(${cola}, ${colb}, ${opac})`;
		}
	}

	static get_displace_strength = (): f32 => {
		let sc: f32 = context_main_object().base.transform.scale.x;
		return config_raw.displace_strength * 0.02 * sc;
	}

	static voxelgi_half_extents = (): string => {
		let ext: f32 = context_raw.vxao_ext;
		return `const vec3 voxelgiHalfExtents = vec3(${ext}, ${ext}, ${ext});`;
	}

	static delete_context = (c: shader_context_t) => {
		base_notify_on_next_frame(() => { // Ensure pipeline is no longer in use
			shader_context_delete(c);
		});
	}
}
