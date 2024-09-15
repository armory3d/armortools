
let line_draw_color: color_t = 0xffff0000;
let line_draw_strength: f32 = 0.005;
let line_draw_mat: mat4_t = mat4_nan();
let line_draw_dim: vec4_t = vec4_nan();

let line_draw_vertex_buffer: vertex_buffer_t;
let line_draw_index_buffer: index_buffer_t;
let line_draw_pipeline: pipeline_t = null;

let line_draw_vp: mat4_t;
let line_draw_vp_loc: kinc_const_loc_t;

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

function line_draw_render(matrix: mat4_t) {
	line_draw_mat = matrix;
	line_draw_dim = mat4_get_scale(matrix);

	if (line_draw_pipeline == null) {
		let structure: vertex_struct_t = g4_vertex_struct_create();
		g4_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
		g4_vertex_struct_add(structure, "col", vertex_data_t.F32_3X);
		line_draw_pipeline = g4_pipeline_create();
		line_draw_pipeline.input_layout = [structure];
		line_draw_pipeline.fragment_shader = sys_get_shader("line.frag");
		line_draw_pipeline.vertex_shader = sys_get_shader("line.vert");
		line_draw_pipeline.depth_write = true;
		line_draw_pipeline.depth_mode = compare_mode_t.LESS;
		line_draw_pipeline.cull_mode = cull_mode_t.NONE;
		line_draw_pipeline.color_attachment_count = 3;
		line_draw_pipeline.color_attachments[0] = tex_format_t.RGBA64;
		line_draw_pipeline.color_attachments[1] = tex_format_t.RGBA64;
		line_draw_pipeline.color_attachments[2] = tex_format_t.RGBA64;
		line_draw_pipeline.depth_attachment = depth_format_t.DEPTH24;
		g4_pipeline_compile(line_draw_pipeline);
		line_draw_vp_loc = g4_pipeline_get_const_loc(line_draw_pipeline, "VP");
		line_draw_vp = mat4_identity();
		line_draw_vertex_buffer = g4_vertex_buffer_create(line_draw_max_vertices, structure, usage_t.DYNAMIC);
		line_draw_index_buffer = g4_index_buffer_create(line_draw_max_indices);
	}

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

	line_draw_line(line_draw_v1.x, line_draw_v1.y, line_draw_v1.z, line_draw_v2.x, line_draw_v2.y, line_draw_v2.z);
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

	let i: i32 = line_draw_lines * 24; // 4 * 6 (structure len)
	let data: f32[] = [line_draw_corner1.x, line_draw_corner1.y, line_draw_corner1.z, color_get_rb(line_draw_color) / 255, color_get_gb(line_draw_color) / 255, color_get_ab(line_draw_color) / 255];
	line_draw_add_vb_data(i, data);

	i += 6;
	data = [line_draw_corner2.x, line_draw_corner2.y, line_draw_corner2.z, color_get_rb(line_draw_color) / 255, color_get_gb(line_draw_color) / 255, color_get_ab(line_draw_color) / 255];
	line_draw_add_vb_data(i, data);

	i += 6;
	data = [line_draw_corner3.x, line_draw_corner3.y, line_draw_corner3.z, color_get_rb(line_draw_color) / 255, color_get_gb(line_draw_color) / 255, color_get_ab(line_draw_color) / 255];
	line_draw_add_vb_data(i, data);

	i += 6;
	data = [line_draw_corner4.x, line_draw_corner4.y, line_draw_corner4.z, color_get_rb(line_draw_color) / 255, color_get_gb(line_draw_color) / 255, color_get_ab(line_draw_color) / 255];
	line_draw_add_vb_data(i, data);

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
	line_draw_lines = 0;
	line_draw_vb_data = g4_vertex_buffer_lock(line_draw_vertex_buffer);
	line_draw_ib_data = g4_index_buffer_lock(line_draw_index_buffer);
}

function line_draw_end() {
	g4_vertex_buffer_unlock(line_draw_vertex_buffer);
	g4_index_buffer_unlock(line_draw_index_buffer);

	g4_set_vertex_buffer(line_draw_vertex_buffer);
	g4_set_index_buffer(line_draw_index_buffer);
	g4_set_pipeline(line_draw_pipeline);
	let camera: camera_object_t = scene_camera;
	line_draw_vp = mat4_clone(camera.v);
	line_draw_vp = mat4_mult_mat(line_draw_vp, camera.p);
	g4_set_mat(line_draw_vp_loc, line_draw_vp);
	g4_draw(0, line_draw_lines * 6);
}

function line_draw_add_vb_data(i: i32, data: f32[]) {
	for (let offset: i32 = 0; offset < 6; ++offset) {
		buffer_set_f32(line_draw_vb_data, (i + offset) * 4, data[offset]);
	}
}
