
let line_draw_color: color_t = 0xffff0000;
let line_draw_strength: f32 = 0.005;
let line_draw_mat: mat4_t = mat4_nan();
let line_draw_dim: vec4_t = vec4_nan();

let line_draw_vertex_buffer: gpu_buffer_t;
let line_draw_index_buffer: gpu_buffer_t;
let line_draw_pipeline: gpu_pipeline_t = null;
let line_draw_overlay_pipeline: gpu_pipeline_t = null;

let line_draw_vp: mat4_t;
let line_draw_vp_loc: i32;
let line_draw_color_loc: i32;

let line_draw_vb_data: buffer_t;
let line_draw_ib_data: u32_array_t;

let line_draw_max_lines: i32 = 300;
let line_draw_max_vertices: i32 = line_draw_max_lines * 4;
let line_draw_max_indices: i32 = line_draw_max_lines * 6;
let line_draw_lines: i32 = 0;

let line_draw_wpos: vec4_t;
let line_draw_vx: vec4_t = vec4_create();
let line_draw_vy: vec4_t = vec4_create();
let line_draw_vz: vec4_t = vec4_create();

let line_draw_v1: vec4_t = vec4_create();
let line_draw_v2: vec4_t = vec4_create();
let line_draw_t: vec4_t = vec4_create();

let line_draw_mid_point: vec4_t = vec4_create();
let line_draw_mid_line: vec4_t = vec4_create();
let line_draw_corner1: vec4_t = vec4_create();
let line_draw_corner2: vec4_t = vec4_create();
let line_draw_corner3: vec4_t = vec4_create();
let line_draw_corner4: vec4_t = vec4_create();
let line_draw_camera_look: vec4_t = vec4_create();

function line_draw_init() {
	if (line_draw_pipeline == null) {
		let structure: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
		line_draw_pipeline = gpu_create_pipeline();
		line_draw_pipeline.input_layout = structure;
		line_draw_pipeline.fragment_shader = sys_get_shader("line.frag");
		line_draw_pipeline.vertex_shader = sys_get_shader("line.vert");
		line_draw_pipeline.depth_write = true;
		line_draw_pipeline.depth_mode = compare_mode_t.LESS;
		line_draw_pipeline.cull_mode = cull_mode_t.NONE;
		line_draw_pipeline.color_attachment_count = 3;
		ARRAY_ACCESS(line_draw_pipeline.color_attachment, 0) = tex_format_t.RGBA64;
		ARRAY_ACCESS(line_draw_pipeline.color_attachment, 1) = tex_format_t.RGBA64;
		ARRAY_ACCESS(line_draw_pipeline.color_attachment, 2) = tex_format_t.RGBA64;
		line_draw_pipeline.depth_attachment_bits = 32;
		gpu_pipeline_compile(line_draw_pipeline);
		pipes_offset = 0;
		line_draw_vp_loc = pipes_get_constant_location("mat4");
		line_draw_color_loc = pipes_get_constant_location("vec3");
		line_draw_vp = mat4_identity();
		line_draw_vertex_buffer = gpu_create_vertex_buffer(line_draw_max_vertices, structure);
		line_draw_index_buffer = gpu_create_index_buffer(line_draw_max_indices);
	}
	if (line_draw_overlay_pipeline == null) {
		let structure: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
		line_draw_overlay_pipeline = gpu_create_pipeline();
		line_draw_overlay_pipeline.input_layout = structure;
		line_draw_overlay_pipeline.fragment_shader = sys_get_shader("line_overlay.frag");
		line_draw_overlay_pipeline.vertex_shader = sys_get_shader("line_overlay.vert");
		line_draw_overlay_pipeline.depth_write = true;
		line_draw_overlay_pipeline.depth_mode = compare_mode_t.LESS;
		line_draw_overlay_pipeline.cull_mode = cull_mode_t.NONE;
		line_draw_overlay_pipeline.color_attachment_count = 1;
		ARRAY_ACCESS(line_draw_overlay_pipeline.color_attachment, 0) = tex_format_t.RGBA64;
		gpu_pipeline_compile(line_draw_overlay_pipeline);
	}
}

function line_draw_render(matrix: mat4_t) {
	line_draw_mat = matrix;
	line_draw_dim = mat4_get_scale(matrix);

	line_draw_begin();
	line_draw_bounds(line_draw_mat, line_draw_dim);
	line_draw_end();
}

function line_draw_bounds(mat: mat4_t, dim: vec4_t) {
	line_draw_wpos = mat4_get_loc(mat);
	let dx: f32 = dim.x / 2;
	let dy: f32 = dim.y / 2;
	let dz: f32 = dim.z / 2;

	let up: vec4_t = mat4_up(mat);
	let look: vec4_t = mat4_look(mat);
	let right: vec4_t = mat4_right(mat);
	up = vec4_norm(up);
	look = vec4_norm(look);
	right = vec4_norm(right);

	line_draw_vx = vec4_clone(right);
	line_draw_vx = vec4_mult(line_draw_vx, dx);
	line_draw_vy = vec4_clone(look);
	line_draw_vy = vec4_mult(line_draw_vy, dy);
	line_draw_vz = vec4_clone(up);
	line_draw_vz = vec4_mult(line_draw_vz, dz);

	line_draw_lineb(-1, -1, -1,  1, -1, -1);
	line_draw_lineb(-1,  1, -1,  1,  1, -1);
	line_draw_lineb(-1, -1,  1,  1, -1,  1);
	line_draw_lineb(-1,  1,  1,  1,  1,  1);

	line_draw_lineb(-1, -1, -1, -1,  1, -1);
	line_draw_lineb(-1, -1,  1, -1,  1,  1);
	line_draw_lineb( 1, -1, -1,  1,  1, -1);
	line_draw_lineb( 1, -1,  1,  1,  1,  1);

	line_draw_lineb(-1, -1, -1, -1, -1,  1);
	line_draw_lineb(-1,  1, -1, -1,  1,  1);
	line_draw_lineb( 1, -1, -1,  1, -1,  1);
	line_draw_lineb( 1,  1, -1,  1,  1,  1);
}

function line_draw_lineb(a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) {
	line_draw_v1 = vec4_clone(line_draw_wpos);

	line_draw_t = vec4_clone(line_draw_vx);
	line_draw_t = vec4_mult(line_draw_t, a);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_t = vec4_clone(line_draw_vy);
	line_draw_t = vec4_mult(line_draw_t, b);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_t = vec4_clone(line_draw_vz);
	line_draw_t = vec4_mult(line_draw_t, c);
	line_draw_v1 = vec4_add(line_draw_v1, line_draw_t);

	line_draw_v2 = vec4_clone(line_draw_wpos);

	line_draw_t = vec4_clone(line_draw_vx);
	line_draw_t = vec4_mult(line_draw_t, d);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_t = vec4_clone(line_draw_vy);
	line_draw_t = vec4_mult(line_draw_t, e);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_t = vec4_clone(line_draw_vz);
	line_draw_t = vec4_mult(line_draw_t, f);
	line_draw_v2 = vec4_add(line_draw_v2, line_draw_t);

	line_draw_line(line_draw_v1.x, line_draw_v1.y, line_draw_v1.z,
				   line_draw_v2.x, line_draw_v2.y, line_draw_v2.z);
}

function line_draw_line(x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32) {
	if (line_draw_lines >= line_draw_max_lines) {
		line_draw_end();
		line_draw_begin();
	}

	line_draw_mid_point = vec4_create(x1 + x2, y1 + y2, z1 + z2);
	line_draw_mid_point = vec4_mult(line_draw_mid_point, 0.5);

	line_draw_mid_line = vec4_create(x1, y1, z1);
	line_draw_mid_line = vec4_sub(line_draw_mid_line, line_draw_mid_point);

	let camera: camera_object_t = scene_camera;
	line_draw_camera_look = mat4_get_loc(camera.base.transform.world);
	line_draw_camera_look = vec4_sub(line_draw_camera_look, line_draw_mid_point);

	let line_width: vec4_t = vec4_cross(line_draw_camera_look, line_draw_mid_line);
	line_width = vec4_norm(line_width);
	line_width = vec4_mult(line_width, line_draw_strength);

	line_draw_corner1 = vec4_create(x1, y1, z1);
	line_draw_corner1 = vec4_add(line_draw_corner1, line_width);

	line_draw_corner2 = vec4_create(x1, y1, z1);
	line_draw_corner2 = vec4_sub(line_draw_corner2, line_width);

	line_draw_corner3 = vec4_create(x2, y2, z2);
	line_draw_corner3 = vec4_sub(line_draw_corner3, line_width);

	line_draw_corner4 = vec4_create(x2, y2, z2);
	line_draw_corner4 = vec4_add(line_draw_corner4, line_width);

	let i: i32 = line_draw_lines * 12; // 4 * 3 (structure len)

	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner1.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner1.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner1.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner2.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner2.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner2.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner3.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner3.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner3.z);

	i += 3;
	buffer_set_f32(line_draw_vb_data, (i + 0) * 4, line_draw_corner4.x);
	buffer_set_f32(line_draw_vb_data, (i + 1) * 4, line_draw_corner4.y);
	buffer_set_f32(line_draw_vb_data, (i + 2) * 4, line_draw_corner4.z);

	i = line_draw_lines * 6;
	line_draw_ib_data[i    ] = line_draw_lines * 4;
	line_draw_ib_data[i + 1] = line_draw_lines * 4 + 1;
	line_draw_ib_data[i + 2] = line_draw_lines * 4 + 2;
	line_draw_ib_data[i + 3] = line_draw_lines * 4 + 2;
	line_draw_ib_data[i + 4] = line_draw_lines * 4 + 3;
	line_draw_ib_data[i + 5] = line_draw_lines * 4;

	line_draw_lines++;
}

function line_draw_begin() {
	line_draw_init();
	line_draw_lines = 0;
	line_draw_vb_data = gpu_lock_vertex_buffer(line_draw_vertex_buffer);
	line_draw_ib_data = gpu_lock_index_buffer(line_draw_index_buffer);
	for (let i: i32 = 0; i < line_draw_max_indices; ++i) {
		line_draw_ib_data[i] = 0;
	}
}

function line_draw_end(overlay: bool = false) {
	gpu_vertex_buffer_unlock(line_draw_vertex_buffer);
	gpu_index_buffer_unlock(line_draw_index_buffer);

	gpu_set_vertex_buffer(line_draw_vertex_buffer);
	gpu_set_index_buffer(line_draw_index_buffer);
	gpu_set_pipeline(overlay ? line_draw_overlay_pipeline : line_draw_pipeline);
	let camera: camera_object_t = scene_camera;
	line_draw_vp = mat4_clone(camera.v);
	line_draw_vp = mat4_mult_mat(line_draw_vp, camera.p);
	gpu_set_matrix4(line_draw_vp_loc, line_draw_vp);
	gpu_set_float3(line_draw_color_loc,
		color_get_rb(line_draw_color) / 255,
		color_get_gb(line_draw_color) / 255,
		color_get_bb(line_draw_color) / 255
	);
	gpu_draw();
}

let _shape_draw_sphere_vb: gpu_buffer_t = null;
let _shape_draw_sphere_ib: gpu_buffer_t = null;

function shape_draw_sphere(mat: mat4_t) {
	line_draw_init();

	if (_shape_draw_sphere_vb == null) {
		let sphere: mesh_object_t = scene_get_child(".Sphere").ext;
    	let md: mesh_data_t = sphere.data;

		let posa: i16_array_t = md.vertex_arrays[0].values;
		let structure: gpu_vertex_structure_t = gpu_vertex_struct_create();
		gpu_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
		_shape_draw_sphere_vb = gpu_create_vertex_buffer(posa.length, structure);
		let data: buffer_t = gpu_lock_vertex_buffer(_shape_draw_sphere_vb);
		for (let i: i32 = 0; i < posa.length / 4; ++i) {
			buffer_set_f32(data, (i * 3 + 0) * 4, posa[i * 4 + 0] / 32767);
			buffer_set_f32(data, (i * 3 + 1) * 4, posa[i * 4 + 1] / 32767);
			buffer_set_f32(data, (i * 3 + 2) * 4, posa[i * 4 + 2] / 32767);
		}
		gpu_vertex_buffer_unlock(_shape_draw_sphere_vb);
		_shape_draw_sphere_ib = md._.index_buffers[0];
	}

	gpu_set_vertex_buffer(_shape_draw_sphere_vb);
	gpu_set_index_buffer(_shape_draw_sphere_ib);
	gpu_set_pipeline(line_draw_overlay_pipeline);
	let camera: camera_object_t = scene_camera;
	line_draw_vp = mat4_clone(mat);
	let f: f32 = line_draw_strength * 50;
	line_draw_vp = mat4_scale(line_draw_vp, vec4_create(f, f, f));
	line_draw_vp = mat4_mult_mat(line_draw_vp, camera.v);
	line_draw_vp = mat4_mult_mat(line_draw_vp, camera.p);
	gpu_set_matrix4(line_draw_vp_loc, line_draw_vp);
	gpu_set_float3(line_draw_color_loc,
		color_get_rb(line_draw_color) / 255,
		color_get_gb(line_draw_color) / 255,
		color_get_bb(line_draw_color) / 255
	);
	gpu_draw();
}