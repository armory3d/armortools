
type project_format_t = {
	version?: string;
	assets?: string[]; // texture_assets
	is_bgra?: Null<bool>; // Swapped red and blue channels for layer textures
	packed_assets?: packed_asset_t[];
	envmap?: string; // Asset name
	envmap_strength?: Null<f32>;
	camera_world?: Float32Array;
	camera_origin?: Float32Array;
	camera_fov?: Null<f32>;
	swatches?: swatch_color_t[];

	///if (is_paint || is_sculpt)
	brush_nodes?: zui_node_canvas_t[];
	brush_icons?: ArrayBuffer[];
	material_nodes?: zui_node_canvas_t[];
	material_groups?: zui_node_canvas_t[];
	material_icons?: ArrayBuffer[];
	font_assets?: string[];
	layer_datas?: layer_data_t[];
	mesh_datas?: mesh_data_t[];
	mesh_assets?: string[];
	mesh_icons?: ArrayBuffer[];
	///end

	///if is_paint
	atlas_objects?: i32[];
	atlas_names?: string[];
	///end

	///if is_lab
	material?: zui_node_canvas_t;
	material_groups?: zui_node_canvas_t[];
	mesh_data?: mesh_data_t;
	mesh_icon?: ArrayBuffer;
	///end
};

type asset_t = {
	id?: i32;
	name?: string;
	file?: string;
};

type packed_asset_t = {
	name?: string;
	bytes?: ArrayBuffer;
};

type swatch_color_t = {
	base?: color_t;
	opacity?: f32;
	occlusion?: f32;
	roughness?: f32;
	metallic?: f32;
	normal?: color_t;
	emission?: f32;
	height?: f32;
	subsurface?: f32;
};

///if (is_paint || is_sculpt)
type layer_data_t = {
	name?: string;
	res?: i32; // Width pixels
	bpp?: i32; // Bits per pixel
	texpaint?: ArrayBuffer;
	uv_scale?: f32;
	uv_rot?: f32;
	uv_type?: i32;
	decal_mat?: Float32Array;
	opacity_mask?: f32;
	fill_layer?: i32;
	object_mask?: i32;
	blending?: i32;
	parent?: i32;
	visible?: bool;
	///if is_paint
	texpaint_nor?: ArrayBuffer;
	texpaint_pack?: ArrayBuffer;
	paint_base?: bool;
	paint_opac?: bool;
	paint_occ?: bool;
	paint_rough?: bool;
	paint_met?: bool;
	paint_nor?: bool;
	paint_nor_blend?: bool;
	paint_height?: bool;
	paint_height_blend?: bool;
	paint_emis?: bool;
	paint_subs?: bool;
	///end
};
///end
