
type TProjectFormat = {
	version?: string;
	assets?: string[]; // texture_assets
	is_bgra?: Null<bool>; // Swapped red and blue channels for layer textures
	packed_assets?: TPackedAsset[];
	envmap?: string; // Asset name
	envmap_strength?: Null<f32>;
	camera_world?: Float32Array;
	camera_origin?: Float32Array;
	camera_fov?: Null<f32>;
	swatches?: TSwatchColor[];

	///if (is_paint || is_sculpt)
	brush_nodes?: zui_node_canvas_t[];
	brush_icons?: ArrayBuffer[];
	material_nodes?: zui_node_canvas_t[];
	material_groups?: zui_node_canvas_t[];
	material_icons?: ArrayBuffer[];
	font_assets?: string[];
	layer_datas?: TLayerData[];
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
}

type TAsset = {
	id: i32;
	name: string;
	file: string;
}

type TPackedAsset = {
	name: string;
	bytes: ArrayBuffer;
}

type TSwatchColor = {
	base: Color;
	opacity: f32;
	occlusion: f32;
	roughness: f32;
	metallic: f32;
	normal: Color;
	emission: f32;
	height: f32;
	subsurface: f32;
}

///if (is_paint || is_sculpt)
type TLayerData = {
	name: string;
	res: i32; // Width pixels
	bpp: i32; // Bits per pixel
	texpaint: ArrayBuffer;
	uv_scale: f32;
	uv_rot: f32;
	uv_type: i32;
	decal_mat: Float32Array;
	opacity_mask: f32;
	fill_layer: i32;
	object_mask: i32;
	blending: i32;
	parent: i32;
	visible: bool;
	///if is_paint
	texpaint_nor: ArrayBuffer;
	texpaint_pack: ArrayBuffer;
	paint_base: bool;
	paint_opac: bool;
	paint_occ: bool;
	paint_rough: bool;
	paint_met: bool;
	paint_nor: bool;
	paint_nor_blend: bool;
	paint_height: bool;
	paint_height_blend: bool;
	paint_emis: bool;
	paint_subs: bool;
	///end
}
///end
