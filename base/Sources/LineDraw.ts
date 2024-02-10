
class LineDraw {

	static color: Color = 0xffff0000;
	static strength = 0.005;
	static mat: mat4_t = null;
	static dim: vec4_t = null;

	static vertexBuffer: vertex_buffer_t;
	static indexBuffer: index_buffer_t;
	static pipeline: pipeline_t = null;

	static vp: mat4_t;
	static vpID: kinc_const_loc_t;

	static vbData: DataView;
	static ibData: Uint32Array;

	static maxLines = 300;
	static maxVertices = LineDraw.maxLines * 4;
	static maxIndices = LineDraw.maxLines * 6;
	static lines = 0;

	static render = (matrix: mat4_t) => {
		LineDraw.mat = matrix;
		LineDraw.dim = mat4_get_scale(matrix);

		if (LineDraw.pipeline == null) {
			let structure = g4_vertex_struct_create();
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
			LineDraw.vpID = g4_pipeline_get_const_loc(LineDraw.pipeline, "VP");
			LineDraw.vp = mat4_identity();
			LineDraw.vertexBuffer = g4_vertex_buffer_create(LineDraw.maxVertices, structure, usage_t.DYNAMIC);
			LineDraw.indexBuffer = g4_index_buffer_create(LineDraw.maxIndices);
		}

		LineDraw.begin();
		LineDraw.bounds(LineDraw.mat, LineDraw.dim);
		LineDraw.end();
	}

	static wpos: vec4_t;
	static vx = vec4_create();
	static vy = vec4_create();
	static vz = vec4_create();

	static bounds = (mat: mat4_t, dim: vec4_t) => {
		LineDraw.wpos = mat4_get_loc(mat);
		let dx = dim.x / 2;
		let dy = dim.y / 2;
		let dz = dim.z / 2;

		let up = mat4_up(mat);
		let look = mat4_look(mat);
		let right = mat4_right(mat);
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

	static v1 = vec4_create();
	static v2 = vec4_create();
	static t = vec4_create();

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

	static midPoint = vec4_create();
	static midLine = vec4_create();
	static corner1 = vec4_create();
	static corner2 = vec4_create();
	static corner3 = vec4_create();
	static corner4 = vec4_create();
	static cameraLook = vec4_create();

	static line = (x1: f32, y1: f32, z1: f32, x2: f32, y2: f32, z2: f32) => {
		if (LineDraw.lines >= LineDraw.maxLines) {
			LineDraw.end();
			LineDraw.begin();
		}

		vec4_set(LineDraw.midPoint, x1 + x2, y1 + y2, z1 + z2);
		vec4_mult(LineDraw.midPoint, 0.5);

		vec4_set(LineDraw.midLine, x1, y1, z1);
		vec4_sub(LineDraw.midLine, LineDraw.midPoint);

		let camera = scene_camera;
		LineDraw.cameraLook = mat4_get_loc(camera.base.transform.world);
		vec4_sub(LineDraw.cameraLook, LineDraw.midPoint);

		let lineWidth = vec4_cross(LineDraw.cameraLook, LineDraw.midLine);
		vec4_normalize(lineWidth, );
		vec4_mult(lineWidth, LineDraw.strength);

		vec4_add(vec4_set(LineDraw.corner1, x1, y1, z1), lineWidth);
		vec4_sub(vec4_set(LineDraw.corner2, x1, y1, z1), lineWidth);
		vec4_sub(vec4_set(LineDraw.corner3, x2, y2, z2), lineWidth);
		vec4_add(vec4_set(LineDraw.corner4, x2, y2, z2), lineWidth);

		let i = LineDraw.lines * 24; // 4 * 6 (structure len)
		LineDraw.addVbData(i, [LineDraw.corner1.x, LineDraw.corner1.y, LineDraw.corner1.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner2.x, LineDraw.corner2.y, LineDraw.corner2.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner3.x, LineDraw.corner3.y, LineDraw.corner3.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);
		i += 6;
		LineDraw.addVbData(i, [LineDraw.corner4.x, LineDraw.corner4.y, LineDraw.corner4.z, color_get_rb(LineDraw.color) / 255, color_get_gb(LineDraw.color) / 255, color_get_ab(LineDraw.color) / 255]);

		i = LineDraw.lines * 6;
		LineDraw.ibData[i    ] = LineDraw.lines * 4;
		LineDraw.ibData[i + 1] = LineDraw.lines * 4 + 1;
		LineDraw.ibData[i + 2] = LineDraw.lines * 4 + 2;
		LineDraw.ibData[i + 3] = LineDraw.lines * 4 + 2;
		LineDraw.ibData[i + 4] = LineDraw.lines * 4 + 3;
		LineDraw.ibData[i + 5] = LineDraw.lines * 4;

		LineDraw.lines++;
	}

	static begin = () => {
		LineDraw.lines = 0;
		LineDraw.vbData = g4_vertex_buffer_lock(LineDraw.vertexBuffer);
		LineDraw.ibData = g4_index_buffer_lock(LineDraw.indexBuffer);
	}

	static end = () => {
		g4_vertex_buffer_unlock(LineDraw.vertexBuffer);
		g4_index_buffer_unlock(LineDraw.indexBuffer);

		g4_set_vertex_buffer(LineDraw.vertexBuffer);
		g4_set_index_buffer(LineDraw.indexBuffer);
		g4_set_pipeline(LineDraw.pipeline);
		let camera = scene_camera;
		mat4_set_from(LineDraw.vp, camera.v);
		mat4_mult_mat(LineDraw.vp, camera.p);
		g4_set_mat(LineDraw.vpID, LineDraw.vp);
		g4_draw(0, LineDraw.lines * 6);
	}

	static addVbData = (i: i32, data: f32[]) => {
		for (let offset = 0; offset < 6; ++offset) {
			LineDraw.vbData.setFloat32((i + offset) * 4, data[offset], true);
		}
	}
}
