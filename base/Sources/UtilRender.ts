
///if (is_paint || is_sculpt)

class UtilRender {

	static material_preview_size: i32 = 256;
	static decal_preview_size: i32 = 512;
	static layer_preview_size: i32 = 200;
	static screen_aligned_full_vb: vertex_buffer_t = null;
	static screen_aligned_full_ib: index_buffer_t = null;

	static make_material_preview = () => {
		Context.raw.material_preview = true;

		let sphere: mesh_object_t = scene_get_child(".Sphere").ext;
		sphere.base.visible = true;
		let meshes: mesh_object_t[] = scene_meshes;
		scene_meshes = [sphere];
		let painto: mesh_object_t = Context.raw.paint_object;
		Context.raw.paint_object = sphere;

		sphere.materials[0] = Project.materials[0].data;
		Context.raw.material.preview_ready = true;

		mat4_set_from(Context.raw.saved_camera, scene_camera.base.transform.local);
		let m: mat4_t = mat4_create(0.9146286343879498, -0.0032648027153306235, 0.404281837254303, 0.4659988049397712, 0.404295023959927, 0.007367569133732468, -0.9145989516155143, -1.0687517188018691, 0.000007410128652369705, 0.9999675337275382, 0.008058532943908717, 0.015935682577325486, 0, 0, 0, 1);
		transform_set_matrix(scene_camera.base.transform, m);
		let savedFov: f32 = scene_camera.data.fov;
		scene_camera.data.fov = 0.92;
		Viewport.update_camera_type(camera_type_t.PERSPECTIVE);
		let light: light_object_t = scene_lights[0];
		let _lightStrength: f32 = light.data.strength;
		let probe: world_data_t = scene_world;
		let _probeStrength: f32 = probe.strength;
		light.data.strength = 0;
		probe.strength = 7;
		let _envmapAngle: f32 = Context.raw.envmap_angle;
		Context.raw.envmap_angle = 6.0;
		let _brushScale: f32 = Context.raw.brush_scale;
		Context.raw.brush_scale = 1.5;
		let _brushNodesScale: f32 = Context.raw.brush_nodes_scale;
		Context.raw.brush_nodes_scale = 1.0;

		scene_world._.envmap = Context.raw.preview_envmap;
		// No resize
		_render_path_last_w = UtilRender.material_preview_size;
		_render_path_last_h = UtilRender.material_preview_size;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		MakeMaterial.parse_mesh_preview_material();
		let _commands: ()=>void = render_path_commands;
		render_path_commands = RenderPathPreview.commands_preview;
		render_path_render_frame();
		render_path_commands = _commands;

		Context.raw.material_preview = false;
		_render_path_last_w = app_w();
		_render_path_last_h = app_h();

		// Restore
		sphere.base.visible = false;
		scene_meshes = meshes;
		Context.raw.paint_object = painto;

		transform_set_matrix(scene_camera.base.transform, Context.raw.saved_camera);
		Viewport.update_camera_type(Context.raw.camera_type);
		scene_camera.data.fov = savedFov;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);
		light.data.strength = _lightStrength;
		probe.strength = _probeStrength;
		Context.raw.envmap_angle = _envmapAngle;
		Context.raw.brush_scale = _brushScale;
		Context.raw.brush_nodes_scale = _brushNodesScale;
		scene_world._.envmap = Context.raw.show_envmap ? Context.raw.saved_envmap : Context.raw.empty_envmap;
		MakeMaterial.parse_mesh_material();
		Context.raw.ddirty = 0;
	}

	static make_decal_preview = () => {
		let current: image_t = _g2_current;
		let g2_in_use: bool = _g2_in_use;
		if (g2_in_use) g2_end();

		if (Context.raw.decal_image == null) {
			Context.raw.decal_image = image_create_render_target(UtilRender.decal_preview_size, UtilRender.decal_preview_size);
		}
		Context.raw.decal_preview = true;

		let plane: mesh_object_t = scene_get_child(".Plane").ext;
		vec4_set(plane.base.transform.scale, 1, 1, 1);
		quat_from_euler(plane.base.transform.rot, -Math.PI / 2, 0, 0);
		transform_build_matrix(plane.base.transform);
		plane.base.visible = true;
		let meshes: mesh_object_t[] = scene_meshes;
		scene_meshes = [plane];
		let painto: mesh_object_t = Context.raw.paint_object;
		Context.raw.paint_object = plane;

		mat4_set_from(Context.raw.saved_camera, scene_camera.base.transform.local);
		let m: mat4_t = mat4_identity();
		mat4_translate(m, 0, 0, 1);
		transform_set_matrix(scene_camera.base.transform, m);
		let savedFov: f32 = scene_camera.data.fov;
		scene_camera.data.fov = 0.92;
		Viewport.update_camera_type(camera_type_t.PERSPECTIVE);
		let light: light_object_t = scene_lights[0];
		light.base.visible = false;
		scene_world._.envmap = Context.raw.preview_envmap;

		// No resize
		_render_path_last_w = UtilRender.decal_preview_size;
		_render_path_last_h = UtilRender.decal_preview_size;
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		MakeMaterial.parse_mesh_preview_material();
		let _commands: ()=>void = render_path_commands;
		render_path_commands = RenderPathPreview.commands_decal;
		render_path_render_frame();
		render_path_commands = _commands;

		Context.raw.decal_preview = false;
		_render_path_last_w = app_w();
		_render_path_last_h = app_h();

		// Restore
		plane.base.visible = false;
		scene_meshes = meshes;
		Context.raw.paint_object = painto;

		transform_set_matrix(scene_camera.base.transform, Context.raw.saved_camera);
		scene_camera.data.fov = savedFov;
		Viewport.update_camera_type(Context.raw.camera_type);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);
		light = scene_lights[0];
		light.base.visible = true;
		scene_world._.envmap = Context.raw.show_envmap ? Context.raw.saved_envmap : Context.raw.empty_envmap;

		MakeMaterial.parse_mesh_material();
		Context.raw.ddirty = 1; // Refresh depth for decal paint

		if (g2_in_use) g2_begin(current);
	}

	static make_text_preview = () => {
		let current: image_t = _g2_current;
		if (current != null) g2_end();

		let text: string = Context.raw.text_tool_text;
		let font: g2_font_t = Context.raw.font.font;
		let fontSize: i32 = 200;
		let textW: i32 = Math.floor(g2_font_width(font, fontSize, text));
		let textH: i32 = Math.floor(g2_font_height(font, fontSize));
		let texW: i32 = textW + 32;
		if (texW < 512) texW = 512;
		if (Context.raw.text_tool_image != null && Context.raw.text_tool_image.width < texW) {
			image_unload(Context.raw.text_tool_image);
			Context.raw.text_tool_image = null;
		}
		if (Context.raw.text_tool_image == null) {
			///if krom_metal
			Context.raw.text_tool_image = image_create_render_target(texW, texW, tex_format_t.RGBA32);
			///else
			Context.raw.text_tool_image = image_create_render_target(texW, texW, tex_format_t.R8);
			///end
		}
		g2_begin(Context.raw.text_tool_image);
		g2_clear(0xff000000);
		g2_set_font(font);
		g2_set_font_size(fontSize);
		g2_set_color(0xffffffff);
		g2_draw_string(text, texW / 2 - textW / 2, texW / 2 - textH / 2);
		g2_end();

		if (current != null) g2_begin(current);
	}

	static make_font_preview = () => {
		let current: image_t = _g2_current;
		if (current != null) g2_end();

		let text: string = "Abg";
		let font: g2_font_t = Context.raw.font.font;
		let fontSize: i32 = 318;
		let textW: i32 = Math.floor(g2_font_width(font, fontSize, text)) + 8;
		let textH: i32 = Math.floor(g2_font_height(font, fontSize)) + 8;
		if (Context.raw.font.image == null) {
			Context.raw.font.image = image_create_render_target(512, 512, tex_format_t.RGBA32);
		}
		g2_begin(Context.raw.font.image);
		g2_clear(0x00000000);
		g2_set_font(font);
		g2_set_font_size(fontSize);
		g2_set_color(0xffffffff);
		g2_draw_string(text, 512 / 2 - textW / 2, 512 / 2 - textH / 2);
		g2_end();
		Context.raw.font.preview_ready = true;

		if (current != null) g2_begin(current);
	}

	static make_brush_preview = () => {
		if (RenderPathPaint.liveLayerLocked) return;
		Context.raw.material_preview = true;

		let current: image_t = _g2_current;
		if (current != null) g2_end();

		// Prepare layers
		if (RenderPathPaint.liveLayer == null) {
			RenderPathPaint.liveLayer = SlotLayer.create("_live");
		}

		let l: SlotLayerRaw = RenderPathPaint.liveLayer;
		SlotLayer.clear(l);

		if (Context.raw.brush.image == null) {
			Context.raw.brush.image = image_create_render_target(UtilRender.material_preview_size, UtilRender.material_preview_size);
			Context.raw.brush.image_icon = image_create_render_target(50, 50);
		}

		let _material: SlotMaterialRaw = Context.raw.material;
		Context.raw.material = SlotMaterial.create();
		let _tool: workspace_tool_t = Context.raw.tool;
		Context.raw.tool = workspace_tool_t.BRUSH;

		let _layer: SlotLayerRaw = Context.raw.layer;
		if (SlotLayer.is_mask(Context.raw.layer)) {
			Context.raw.layer = Context.raw.layer.parent;
		}

		let _fill_layer: SlotMaterialRaw = Context.raw.layer.fill_layer;
		Context.raw.layer.fill_layer = null;

		RenderPathPaint.use_live_layer(true);
		MakeMaterial.parse_paint_material(false);

		let hid: i32 = History.undo_i - 1 < 0 ? Config.raw.undo_steps - 1 : History.undo_i - 1;
		render_path_render_targets.set("texpaint_undo" + hid,render_path_render_targets.get("empty_black"));

		// Set plane mesh
		let painto: mesh_object_t = Context.raw.paint_object;
		let visibles: bool[] = [];
		for (let p of Project.paint_objects) {
			visibles.push(p.base.visible);
			p.base.visible = false;
		}
		let mergedObjectVisible: bool = false;
		if (Context.raw.merged_object != null) {
			mergedObjectVisible = Context.raw.merged_object.base.visible;
			Context.raw.merged_object.base.visible = false;
		}

		let cam: camera_object_t = scene_camera;
		mat4_set_from(Context.raw.saved_camera, cam.base.transform.local);
		let savedFov: f32 = cam.data.fov;
		Viewport.update_camera_type(camera_type_t.PERSPECTIVE);
		let m: mat4_t = mat4_identity();
		mat4_translate(m, 0, 0, 0.5);
		transform_set_matrix(cam.base.transform, m);
		cam.data.fov = 0.92;
		camera_object_build_proj(cam);
		camera_object_build_mat(cam);
		mat4_get_inv(m, scene_camera.vp);

		let planeo: mesh_object_t = scene_get_child(".Plane").ext;
		planeo.base.visible = true;
		Context.raw.paint_object = planeo;

		let v: vec4_t = vec4_create();
		let sx: f32 = vec4_len(vec4_set(v, m.m[0], m.m[1], m.m[2]));
		quat_from_euler(planeo.base.transform.rot, -Math.PI / 2, 0, 0);
		vec4_set(planeo.base.transform.scale, sx, 1.0, sx);
		vec4_set(planeo.base.transform.loc, m.m[12], -m.m[13], 0.0);
		transform_build_matrix(planeo.base.transform);

		RenderPathPaint.liveLayerDrawn = 0;
		RenderPathBase.draw_gbuffer();

		// Paint brush preview
		let _brushRadius: f32 = Context.raw.brush_radius;
		let _brushOpacity: f32 = Context.raw.brush_opacity;
		let _brushHardness: f32 = Context.raw.brush_hardness;
		Context.raw.brush_radius = 0.33;
		Context.raw.brush_opacity = 1.0;
		Context.raw.brush_hardness = 0.8;
		let _x: f32 = Context.raw.paint_vec.x;
		let _y: f32 = Context.raw.paint_vec.y;
		let _lastX: f32 = Context.raw.last_paint_vec_x;
		let _lastY: f32 = Context.raw.last_paint_vec_y;
		let _pdirty: i32 = Context.raw.pdirty;
		Context.raw.pdirty = 2;

		let pointsX: f32[] = [0.2, 0.2,  0.35, 0.5,  0.5, 0.5,  0.65, 0.8,  0.8, 0.8];
		let pointsY: f32[] = [0.5, 0.5,  0.35 - 0.04, 0.2 - 0.08,  0.4 + 0.015, 0.6 + 0.03,  0.45 - 0.025, 0.3 - 0.05,  0.5 + 0.025, 0.7 + 0.05];
		for (let i: i32 = 1; i < pointsX.length; ++i) {
			Context.raw.last_paint_vec_x = pointsX[i - 1];
			Context.raw.last_paint_vec_y = pointsY[i - 1];
			Context.raw.paint_vec.x = pointsX[i];
			Context.raw.paint_vec.y = pointsY[i];
			RenderPathPaint.commands_paint(false);
		}

		Context.raw.brush_radius = _brushRadius;
		Context.raw.brush_opacity = _brushOpacity;
		Context.raw.brush_hardness = _brushHardness;
		Context.raw.paint_vec.x = _x;
		Context.raw.paint_vec.y = _y;
		Context.raw.last_paint_vec_x = _lastX;
		Context.raw.last_paint_vec_y = _lastY;
		Context.raw.prev_paint_vec_x = -1;
		Context.raw.prev_paint_vec_y = -1;
		Context.raw.pdirty = _pdirty;
		RenderPathPaint.use_live_layer(false);
		Context.raw.layer.fill_layer = _fill_layer;
		Context.raw.layer = _layer;
		Context.raw.material = _material;
		Context.raw.tool = _tool;
		let _init = () => {
			MakeMaterial.parse_paint_material(false);
		}
		app_notify_on_init(_init);

		// Restore paint mesh
		Context.raw.material_preview = false;
		planeo.base.visible = false;
		for (let i: i32 = 0; i < Project.paint_objects.length; ++i) {
			Project.paint_objects[i].base.visible = visibles[i];
		}
		if (Context.raw.merged_object != null) {
			Context.raw.merged_object.base.visible = mergedObjectVisible;
		}
		Context.raw.paint_object = painto;
		transform_set_matrix(scene_camera.base.transform, Context.raw.saved_camera);
		scene_camera.data.fov = savedFov;
		Viewport.update_camera_type(Context.raw.camera_type);
		camera_object_build_proj(scene_camera);
		camera_object_build_mat(scene_camera);

		// Scale layer down to to image preview
		if (Base.pipe_merge == null) Base.make_pipe();
		l = RenderPathPaint.liveLayer;
		let target: image_t = Context.raw.brush.image;
		g2_begin(target);
		g2_clear(0x00000000);
		g2_set_pipeline(Base.pipe_copy);
		g2_draw_scaled_image(l.texpaint, 0, 0, target.width, target.height);
		g2_set_pipeline(null);
		g2_end();

		// Scale image preview down to to icon
		render_path_render_targets.get("texpreview")._image = Context.raw.brush.image;
		render_path_render_targets.get("texpreview_icon")._image = Context.raw.brush.image_icon;
		render_path_set_target("texpreview_icon");
		render_path_bind_target("texpreview", "tex");
		render_path_draw_shader("shader_datas/supersample_resolve/supersample_resolve");

		Context.raw.brush.preview_ready = true;
		Context.raw.brush_blend_dirty = true;

		if (current != null) g2_begin(current);
	}

	static make_node_preview = (canvas: zui_node_canvas_t, node: zui_node_t, image: image_t, group: zui_node_canvas_t = null, parents: zui_node_t[] = null) => {
		let res: any = MakeMaterial.parse_node_preview_material(node, group, parents);
		if (res == null || res.scon == null) return;

		if (UtilRender.screen_aligned_full_vb == null) {
			UtilRender.create_screen_aligned_full_data();
		}

		let _scaleWorld: f32 = Context.raw.paint_object.base.transform.scale_world;
		Context.raw.paint_object.base.transform.scale_world = 3.0;
		transform_build_matrix(Context.raw.paint_object.base.transform);

		g4_begin(image);
		g4_set_pipeline(res.scon._.pipe_state);
		uniforms_set_context_consts(res.scon, [""]);
		uniforms_set_obj_consts(res.scon, Context.raw.paint_object.base);
		uniforms_set_material_consts(res.scon, res.mcon);
		g4_set_vertex_buffer(UtilRender.screen_aligned_full_vb);
		g4_set_index_buffer(UtilRender.screen_aligned_full_ib);
		g4_draw();
		g4_end();

		Context.raw.paint_object.base.transform.scale_world = _scaleWorld;
		transform_build_matrix(Context.raw.paint_object.base.transform);
	}

	static pick_pos_nor_tex = () => {
		Context.raw.pick_pos_nor_tex = true;
		Context.raw.pdirty = 1;
		let _tool: workspace_tool_t = Context.raw.tool;
		Context.raw.tool = workspace_tool_t.PICKER;
		MakeMaterial.parse_paint_material();
		if (Context.raw.paint2d) {
			RenderPathPaint.set_plane_mesh();
		}
		RenderPathPaint.commands_paint(false);
		///if krom_metal
		// Flush command list
		RenderPathPaint.commands_paint(false);
		///end
		if (Context.raw.paint2d) {
			RenderPathPaint.restore_plane_mesh();
		}
		Context.raw.tool = _tool;
		Context.raw.pick_pos_nor_tex = false;
		MakeMaterial.parse_paint_material();
		Context.raw.pdirty = 0;
	}

	static get_decal_mat = (): mat4_t => {
		UtilRender.pick_pos_nor_tex();
		let decalMat: mat4_t = mat4_identity();
		let loc: vec4_t = vec4_create(Context.raw.posx_picked, Context.raw.posy_picked, Context.raw.posz_picked);
		let rot: quat_t = quat_from_to(quat_create(), vec4_create(0.0, 0.0, -1.0), vec4_create(Context.raw.norx_picked, Context.raw.nory_picked, Context.raw.norz_picked));
		let scale: vec4_t = vec4_create(Context.raw.brush_radius * 0.5, Context.raw.brush_radius * 0.5, Context.raw.brush_radius * 0.5);
		mat4_compose(decalMat, loc, rot, scale);
		return decalMat;
	}

	static create_screen_aligned_full_data = () => {
		// Over-sized triangle
		let data: f32[] = [-Math.floor(32767 / 3), -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
					 	    32767,                 -Math.floor(32767 / 3), 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0,
						   -Math.floor(32767 / 3),  32767,                 0, 32767, 0, 0, 0, 0, 0, 0, 0, 0];
		let indices: i32[] = [0, 1, 2];

		// Mandatory vertex data names and sizes
		let structure: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(structure, "pos", vertex_data_t.I16_4X_NORM);
		g4_vertex_struct_add(structure, "nor", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(structure, "tex", vertex_data_t.I16_2X_NORM);
		g4_vertex_struct_add(structure, "col", vertex_data_t.I16_4X_NORM);
		UtilRender.screen_aligned_full_vb = g4_vertex_buffer_create(Math.floor(data.length / Math.floor(g4_vertex_struct_byte_size(structure) / 4)), structure, usage_t.STATIC);
		let vertices: buffer_view_t = g4_vertex_buffer_lock(UtilRender.screen_aligned_full_vb);
		for (let i: i32 = 0; i < Math.floor(vertices.byteLength / 2); ++i) vertices.setInt16(i * 2, data[i], true);
		g4_vertex_buffer_unlock(UtilRender.screen_aligned_full_vb);

		UtilRender.screen_aligned_full_ib = g4_index_buffer_create(indices.length);
		let id: u32_array_t = g4_index_buffer_lock(UtilRender.screen_aligned_full_ib);
		for (let i: i32 = 0; i < id.length; ++i) id[i] = indices[i];
		g4_index_buffer_unlock(UtilRender.screen_aligned_full_ib);
	}
}

///end
