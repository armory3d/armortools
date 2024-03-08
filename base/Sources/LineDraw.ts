
class LineDraw {

	static color: color_t = 0xffff0000;
	static strength: f32 = 0.005;
	static mat: mat4_t = null;
	static dim: vec4_t = null;

	static vertex_buffer: vertex_buffer_t;
	static index_buffer: index_buffer_t;
	static pipeline: pipeline_t = null;

	static vp: mat4_t;
	static vp_loc: kinc_const_loc_t;

	static vb_data: DataView;
	static ib_data: Uint32Array;

	static max_lines: i32 = 300;
	static max_vertices: i32 = LineDraw.max_lines * 4;
	static max_indices: i32 = LineDraw.max_lines * 6;
	static lines: i32 = 0;

	static wpos: vec4_t;
	static vx: vec4_t = vec4_create();
	static vy: vec4_t = vec4_create();
	static vz: vec4_t = vec4_create();

	static v1: vec4_t = vec4_create();
	static v2: vec4_t = vec4_create();
	static t: vec4_t = vec4_create();

	static mid_point: vec4_t = vec4_create();
	static mid_line: vec4_t = vec4_create();
	static corner1: vec4_t = vec4_create();
	static corner2: vec4_t = vec4_create();
	static corner3: vec4_t = vec4_create();
	static corner4: vec4_t = vec4_create();
	static camera_look: vec4_t = vec4_create();

	static render = (matrix: mat4_t) => {
		LineDraw.mat = matrix;
		LineDraw.dim = mat4_get_scale(matrix);

		if (LineDraw.pipeline == null) {
			let structure: vertex_struct_t = g4_vertex_struct_create();
			g4_vertex_struct_add(structure, "pos", vertex_data_t.F32_3X);
			g4_vertex_struct_add(structure, "col", vertex_data_t.F32_3X);
			LineDraw.pipeline = g4_pipeline_create();
			LineDraw.pipeline.input_layout = [structure];
			LineDraw.pipeline.fragment_shader = sys_get_shader("line.frag");
			LineDraw.pipeline.vertex_shader = sys_get_shader("line.vert");
			LineDraw.pipeline.depth_write = true;
			LineDraw.pipeline.depth_mode = compare_mode_t.LESS;
			LineDraw.pipeline.cull_mode = cull_mode_t.NONE;
			LineDraw.pipeline.color_attachment_count = 3;
			LineDraw.pipeline.color_attachments[0] = tex_format_t.RGBA64;
			LineDraw.pipeline.color_attachments[1] = tex_format_t.RGBA64;
			LineDraw.pipeline.color_attachments[2] = tex_format_t.RGBA64;
			LineDraw.pipeline.depth_attachment = depth_format_t.DEPTH24;
			g4_pipeline_compile(LineDraw.pipeline);
			LineDraw.vp_loc = g4_pipeline_get_const_loc(LineDraw.pipeline, "VP");
			LineDraw.vp = mat4_identity();
			LineDraw.vertex_buffer = g4_vertex_buffer_create(LineDraw.max_vertices, structure, usage_t.DYNAMIC);
			LineDraw.index_buffer = g4_index_buffer_create(LineDraw.max_indices);
		}

		LineDraw.begin();
		LineDraw.bounds(LineDraw.mat, LineDraw.dim);
		LineDraw.end();
	}

	static bounds = (mat: mat4_t, dim: vec4_t) => {
		LineDraw.wpos = mat4_get_loc(mat);
		let dx: f32 = dim.x / 2;
		let dy: f32 = dim.y / 2;
		let dz: f32 = dim.z / 2;

		let up: vec4_t = mat4_up(mat);
		let look: vec4_t = mat4_look(mat);
		let right: vec4_t = mat4_right(mat);
		vec4_normalize(up);
		vec4_normalize(look);
		vec4_normalize(right);

		vec4_set_from(LineDraw.vx, right);
		vec4_mult(LineDraw.vx, dx);
		vec4_set_from(LineDraw.vy, look);
		vec4_mult(LineDraw.vy, dy);
		vec4_set_from(LineDraw.vz, up);
		vec4_mult(LineDraw.vz, dz);

		LineDraw.lineb(-1, -1, -1,  1, -1, -1);
		LineDraw.lineb(-1,  1, -1,  1,  1, -1);
		LineDraw.lineb(-1, -1,  1,  1, -1,  1);
		LineDraw.lineb(-1,  1,  1,  1,  1,  1);

		LineDraw.lineb(-1, -1, -1, -1,  1, -1);
		LineDraw.lineb(-1, -1,  1, -1,  1,  1);
		LineDraw.lineb( 1, -1, -1,  1,  1, -1);
		LineDraw.lineb( 1, -1,  1,  1,  1,  1);

		LineDraw.lineb(-1, -1, -1, -1, -1,  1);
		LineDraw.lineb(-1,  1, -1, -1,  1,  1);
		LineDraw.lineb( 1, -1, -1,  1, -1,  1);
		LineDraw.lineb( 1,  1, -1,  1,  1,  1);
	}

	static lineb = (a: i32, b: i32, c: i32, d: i32, e: i32, f: i32) => {
		vec4_set_from(LineDraw.v1, LineDraw.wpos);
		vec4_set_from(LineDraw.t, LineDraw.vx); vec4_mult(LineDraw.t, a); vec4_add(LineDraw.v1, LineDraw.t);
		vec4_set_from(LineDraw.t, LineDraw.vy); vec4_mult(LineDraw.t, b); vec4_add(LineDraw.v1, LineDraw.t);
		vec4_set_from(LineDraw.t, LineDraw.vz); vec4_mult(LineDraw.t, c); vec4_add(LineDraw.v1, LineDraw.t);

		vec4_set_from(LineDraw.v2, LineDraw.wpos);
		vec4_set_from(LineDraw.t, LineDraw.vx); vec4_mult(LineDraw.t, d); vec4_add(LineDraw.v2, LineDraw.t);
		vec4_set_from(LineDraw.t, LineDraw.vy); vec4_mult(LineDraw.t, e); vec4_add(LineDraw.v2, LineDraw.t);
		vec4_set_from(LineDraw.t, LineDraw.vz); vec4_mult(LineDraw.t, f); vec4_add(LineDraw.v2, LineDraw.t);

		LineDraw.line(LineDraw.v1.x, LineDraw.v1.y, LineDraw.v1.z, LineDraw.v2.x, LineDraw.v2.y, LineDraw.v2.z);
	}

	static line = (x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32) => {
		if (LineDraw.lines >= LineDraw.max_lines) {
			LineDraw.end();
			LineDraw.begin();
		}

		vec4_set(LineDraw.mid_point, x1 + x2, y1 + y2, z1 + z2);
		vec4_mult(LineDraw.mid_point, 0.5);

		vec4_set(LineDraw.mid_line, x1, y1, z1);
		vec4_sub(LineDraw.mid_line, LineDraw.mid_point);

		let camera: camera_object_t = scene_camera;
		LineDraw.camera_look = mat4_get_loc(camera.base.transform.world);
		vec4_sub(LineDraw.camera_look, LineDraw.mid_point);

		let line_width: vec4_t = vec4_cross(LineDraw.camera_look, LineDraw.mid_line);
		vec4_normalize(line_width);
		vec4_mult(line_width, LineDraw.strength);

		vec4_add(vec4_set(LineDraw.corner1, x1, y1, z1), line_width);
		vec4_sub(vec4_set(LineDraw.corner2, x1, y1, z1), line_width);
		vec4_sub(vec4_set(LineDraw.corner3, x2, y2, z2), line_width);
		vec4_add(vec4_set(LineDraw.corner4, x2, y2, z2), line_width);

		let i: i32 = LineDraw.lines * 24; // 4 * 6 (structure len)
		LineDraw.add_vb_data(i, [LineDraw.corner1.x, LineDraw.corner1.y, LineDraw.corner1.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.add_vb_data(i, [LineDraw.corner2.x, LineDraw.corner2.y, LineDraw.corner2.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.add_vb_data(i, [LineDraw.corner3.x, LineDraw.corner3.y, LineDraw.corner3.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.add_vb_data(i, [LineDraw.corner4.x, LineDraw.corner4.y, LineDraw.corner4.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);

		i = LineDraw.lines * 6;
		LineDraw.ib_data[i    ] = LineDraw.lines * 4;
		LineDraw.ib_data[i + 1] = LineDraw.lines * 4 + 1;
		LineDraw.ib_data[i + 2] = LineDraw.lines * 4 + 2;
		LineDraw.ib_data[i + 3] = LineDraw.lines * 4 + 2;
		LineDraw.ib_data[i + 4] = LineDraw.lines * 4 + 3;
		LineDraw.ib_data[i + 5] = LineDraw.lines * 4;

		LineDraw.lines++;
	}

	static begin = () => {
		LineDraw.lines = 0;
		LineDraw.vb_data = g4_vertex_buffer_lock(LineDraw.vertex_buffer);
		LineDraw.ib_data = g4_index_buffer_lock(LineDraw.index_buffer);
	}

	static end = () => {
		g4_vertex_buffer_unlock(LineDraw.vertex_buffer);
		g4_index_buffer_unlock(LineDraw.index_buffer);

		g4_set_vertex_buffer(LineDraw.vertex_buffer);
		g4_set_index_buffer(LineDraw.index_buffer);
		g4_set_pipeline(LineDraw.pipeline);
		let camera: camera_object_t = scene_camera;
		mat4_set_from(LineDraw.vp, camera.v);
		mat4_mult_mat(LineDraw.vp, camera.p);
		g4_set_mat(LineDraw.vp_loc, LineDraw.vp);
		g4_draw(0, LineDraw.lines * 6);
	}

	static add_vb_data = (i: i32, data: f32[]) => {
		for (let offset: i32 = 0; offset < 6; ++offset) {
			LineDraw.vb_data.setFloat32((i + offset) * 4, data[offset], true);
		}
	}
}
