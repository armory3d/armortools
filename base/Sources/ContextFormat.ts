/// <reference path='./project.ts'/>
/// <reference path='./enums.ts'/>

// type context_t = {
class context_t {

	texture?: asset_t = null;
	paint_object?: mesh_object_t;
	merged_object?: mesh_object_t = null; // For object mask
	merged_object_is_atlas?: bool = false; // Only objects referenced by atlas are merged

	ddirty?: i32 = 0; // depth
	pdirty?: i32 = 0; // paint
	rdirty?: i32 = 0; // render
	brush_blend_dirty?: bool = true;
	node_preview_socket?: i32 = 0;

	split_view?: bool = false;
	view_index?: i32 = -1;
	view_index_last?: i32 = -1;

	swatch?: swatch_color_t;
	picked_color?: swatch_color_t = make_swatch();
	color_picker_callback?: (sc: swatch_color_t)=>void = null;

	default_irradiance?: Float32Array = null;
	default_radiance?: image_t = null;
	default_radiance_mipmaps?: image_t[] = null;
	saved_envmap?: image_t = null;
	empty_envmap?: image_t = null;
	preview_envmap?: image_t = null;
	envmap_loaded?: bool = false;
	show_envmap?: bool = false;
	show_envmap_handle?: zui_handle_t = zui_handle_create({ selected: false });
	show_envmap_blur?: bool = false;
	show_envmap_blur_handle? = zui_handle_create({ selected: false });
	envmap_angle?: f32 = 0.0;
	light_angle?: f32 = 0.0;
	cull_backfaces?: bool = true;
	texture_filter?: bool = true;

	format_type?: texture_ldr_format_t = texture_ldr_format_t.PNG;
	format_quality?: f32 = 100.0;
	layers_destination? = export_destination_t.DISK;
	split_by?: split_type_t = split_type_t.OBJECT;
	parse_transform?: bool = true;
	parse_vcols?: bool = false;

	select_time?: f32 = 0.0;
	///if (krom_direct3d12 || krom_vulkan || krom_metal)
	pathtrace_mode?: path_trace_mode_t = path_trace_mode_t.CORE;
	///end
	///if (krom_direct3d12 || krom_vulkan) // || krom_metal)
	viewport_mode?: viewport_mode_t = viewport_mode_t.PATH_TRACE;
	///else
	viewport_mode?: viewport_mode_t = viewport_mode_t.LIT;
	///end
	///if (krom_android || krom_ios)
	render_mode?: render_mode_t = render_mode_t.FORWARD;
	///else
	render_mode?: render_mode_t = render_mode_t.DEFERRED;
	///end

	viewport_shader?: (ns: NodeShaderRaw)=>string = null;
	hscale_was_changed?: bool = false;
	export_mesh_format?: mesh_format_t = mesh_format_t.OBJ;
	export_mesh_index?: i32 = 0;
	pack_assets_on_export?: bool = true;

	paint_vec?: vec4_t = vec4_create();
	last_paint_x?: f32 = -1.0;
	last_paint_y?: f32 = -1.0;
	foreground_event?: bool = false;
	painted?: i32 = 0;
	brush_time?: f32 = 0.0;
	clone_start_x?: f32 = -1.0;
	clone_start_y?: f32 = -1.0;
	clone_delta_x?: f32 = 0.0;
	clone_delta_y?: f32 = 0.0;

	show_compass?: bool = true;
	project_type?: project_model_t = project_model_t.ROUNDED_CUBE;
	project_aspect_ratio?: i32 = 0; // 1:1, 2:1, 1:2
	project_objects?: mesh_object_t[];

	last_paint_vec_x?: f32 = -1.0;
	last_paint_vec_y?: f32 = -1.0;
	prev_paint_vec_x?: f32 = -1.0;
	prev_paint_vec_y?: f32 = -1.0;
	frame?: i32 = 0;
	paint2d_view?: bool = false;

	lock_started_x?: f32 = -1.0;
	lock_started_y?: f32 = -1.0;
	brush_locked?: bool = false;
	brush_can_lock?: bool = false;
	brush_can_unlock?: bool = false;
	camera_type?: camera_type_t = camera_type_t.PERSPECTIVE;
	cam_handle?: zui_handle_t = zui_handle_create();
	fov_handle?: zui_handle_t = null;
	undo_handle?: zui_handle_t = null;
	hssao?: zui_handle_t = null;
	hssr?: zui_handle_t = null;
	hbloom?: zui_handle_t = null;
	hsupersample?: zui_handle_t = null;
	hvxao?: zui_handle_t = null;
	///if is_forge
	vxao_ext?: f32 = 2.0;
	///else
	vxao_ext?: f32 = 1.0;
	///end
	vxao_offset?: f32 = 1.5;
	vxao_aperture?: f32 = 1.2;
	texture_export_path?: string = "";
	last_status_position?: i32 = 0;
	camera_controls?: camera_controls_t = camera_controls_t.ORBIT;
	pen_painting_only?: bool = false; // Reject painting with finger when using pen

	///if (is_paint || is_sculpt)
	material?: SlotMaterialRaw;
	layer?: SlotLayerRaw;
	brush?: SlotBrushRaw;
	font?: SlotFontRaw;
	tool?: workspace_tool_t = workspace_tool_t.BRUSH;

	layer_preview_dirty?: bool = true;
	layers_preview_dirty?: bool = false;
	node_preview_dirty?: bool = false;
	node_preview?: image_t = null;
	node_previews?: Map<string, image_t> = null;
	node_previews_used?: string[] = null;
	node_preview_name?: string = "";
	mask_preview_rgba32?: image_t = null;
	mask_preview_last?: SlotLayerRaw = null;

	colorid_picked?: bool = false;
	material_preview?: bool = false; // Drawing material previews
	saved_camera?: mat4_t = mat4_identity();

	color_picker_previous_tool? = workspace_tool_t.BRUSH;
	materialid_picked?: i32 = 0;
	uvx_picked?: f32 = 0.0;
	uvy_picked?: f32 = 0.0;
	picker_select_material?: bool = true;
	picker_mask_handle?: zui_handle_t = zui_handle_create();
	pick_pos_nor_tex?: bool = false;
	posx_picked?: f32 = 0.0;
	posy_picked?: f32 = 0.0;
	posz_picked?: f32 = 0.0;
	norx_picked?: f32 = 0.0;
	nory_picked?: f32 = 0.0;
	norz_picked?: f32 = 0.0;

	draw_wireframe?: bool = false;
	wireframe_handle?: zui_handle_t = zui_handle_create({ selected: false });
	draw_texels?: bool = false;
	texels_handle?: zui_handle_t = zui_handle_create({ selected: false });

	colorid_handle?: zui_handle_t = zui_handle_create();
	layers_export?: export_mode_t = export_mode_t.VISIBLE;

	decal_image?: image_t = null;
	decal_preview?: bool = false;
	decal_x?: f32 = 0.0;
	decal_y?: f32 = 0.0;

	cache_draws?: bool = false;
	write_icon_on_export?: bool = false;

	text_tool_image?: image_t = null;
	text_tool_text?: string;
	particle_material?: material_data_t = null;
	///if arm_physics
	particle_physics?: bool = false;
	particle_hit_x?: f32 = 0.0;
	particle_hit_y?: f32 = 0.0;
	particle_hit_z?: f32 = 0.0;
	last_particle_hit_x?: f32 = 0.0;
	last_particle_hit_y?: f32 = 0.0;
	last_particle_hit_z?: f32 = 0.0;
	particle_timer?: tween_anim_t = null;
	paint_body?: PhysicsBodyRaw = null;
	///end

	layer_filter?: i32 = 0;
	run_brush?: (i: i32)=>void = null;
	parse_brush_inputs?: ()=>void = null;

	gizmo?: object_t = null;
	gizmo_translate_x?: object_t = null;
	gizmo_translate_y?: object_t = null;
	gizmo_translate_z?: object_t = null;
	gizmo_scale_x?: object_t = null;
	gizmo_scale_y?: object_t = null;
	gizmo_scale_z?: object_t = null;
	gizmo_rotate_x?: object_t = null;
	gizmo_rotate_y?: object_t = null;
	gizmo_rotate_z?: object_t = null;
	gizmo_started?: bool = false;
	gizmo_offset?: f32 = 0.0;
	gizmo_drag?: f32 = 0.0;
	gizmo_drag_last?: f32 = 0.0;
	translate_x?: bool = false;
	translate_y?: bool = false;
	translate_z?: bool = false;
	scale_x?: bool = false;
	scale_y?: bool = false;
	scale_z?: bool = false;
	rotate_x?: bool = false;
	rotate_y?: bool = false;
	rotate_z?: bool = false;

	brush_nodes_radius?: f32 = 1.0;
	brush_nodes_opacity?: f32 = 1.0;
	brush_mask_image?: image_t = null;
	brush_mask_image_is_alpha?: bool = false;
	brush_stencil_image?: image_t = null;
	brush_stencil_image_is_alpha?: bool = false;
	brush_stencil_x?: f32 = 0.02;
	brush_stencil_y?: f32 = 0.02;
	brush_stencil_scale?: f32 = 0.9;
	brush_stencil_scaling?: bool = false;
	brush_stencil_angle?: f32 = 0.0;
	brush_stencil_rotating?: bool = false;
	brush_nodes_scale?: f32 = 1.0;
	brush_nodes_angle?: f32 = 0.0;
	brush_nodes_hardness?: f32 = 1.0;
	brush_directional?: bool = false;

	brush_radius?: f32 = 0.5;
	brush_radius_handle?: zui_handle_t = zui_handle_create({ value: 0.5 });
	brush_scale_x?: f32 = 1.0;
	brush_decal_mask_radius?: f32 = 0.5;
	brush_decal_mask_radius_handle?: zui_handle_t = zui_handle_create({ value: 0.5 });
	brush_scale_x_handle?: zui_handle_t = zui_handle_create({ value: 1.0 });
	brush_blending?: blend_type_t = blend_type_t.MIX;
	brush_opacity?: f32 = 1.0;
	brush_opacity_handle?: zui_handle_t = zui_handle_create({ value: 1.0 });
	brush_scale?: f32 = 1.0;
	brush_angle?: f32 = 0.0;
	brush_angle_handle?: zui_handle_t = zui_handle_create({ value: 0.0 });
	///if is_paint
	brush_hardness?: f32 = 0.8;
	///end
	///if is_sculpt
	brush_hardness?: f32 = 0.05;
	///end
	brush_lazy_radius?: f32 = 0.0;
	brush_lazy_step?: f32 = 0.0;
	brush_lazy_x?: f32 = 0.0;
	brush_lazy_y?: f32 = 0.0;
	brush_paint?: uv_type_t = uv_type_t.UVMAP;
	brush_angle_reject_dot?: f32 = 0.5;
	bake_type?: bake_type_t = bake_type_t.AO;
	bake_axis?: bake_axis_t = bake_axis_t.XYZ;
	bake_up_axis?: bake_up_axis_t = bake_up_axis_t.Z;
	bake_samples?: i32 = 128;
	bake_ao_strength?: f32 = 1.0;
	bake_ao_radius?: f32 = 1.0;
	bake_ao_offset?: f32 = 1.0;
	bake_curv_strength?: f32 = 1.0;
	bake_curv_radius?: f32 = 1.0;
	bake_curv_offset?: f32 = 0.0;
	bake_curv_smooth?: i32 = 1;
	bake_high_poly?: i32 = 0;

	xray?: bool = false;
	sym_x?: bool = false;
	sym_y?: bool = false;
	sym_z?: bool = false;
	fill_type_handle?: zui_handle_t = zui_handle_create();

	paint2d?: bool = false;

	last_htab0_pos?: i32 = 0;
	maximized_sidebar_width?: i32 = 0;
	drag_dest?: i32 = 0;
	///end

	///if is_lab
	material?: any; ////
	layer?: any; ////
	tool?: workspace_tool_t = workspace_tool_t.ERASER;

	color_picker_previous_tool?: workspace_tool_t = workspace_tool_t.ERASER;

	brush_radius?: f32 = 0.25;
	brush_radius_handle?: zui_handle_t = zui_handle_create({ value: 0.25 });
	brush_scale?: f32 = 1.0;

	coords?: vec4_t = vec4_create();
	start_x?: f32 = 0.0;
	start_y?: f32 = 0.0;

	// Brush ruler
	lock_begin?: bool = false;
	lock_x?: bool = false;
	lock_y?: bool = false;
	lock_start_x?: f32 = 0.0;
	lock_start_y?: f32 = 0.0;
	registered?: bool = false;
	///end

	///if is_forge
	selected_object?: object_t = null;
	///end
}
