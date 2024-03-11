
function geom_make_plane(size_x: f32 = 1.0, size_y: f32 = 1.0, verts_x: i32 = 2, verts_y: i32 = 2, uv_scale: f32 = 1.0): raw_mesh_t {

	let mesh: raw_mesh_t = {};
	mesh.scale_pos = 1.0;
	mesh.scale_tex = 1.0;
	mesh.name = "";
	mesh.has_next = false;

	// Pack positions to (-1, 1) range
	let half_x: f32 = size_x / 2;
	let half_y: f32 = size_y / 2;
	mesh.scale_pos = math_max(half_x, half_y);
	let inv: f32 = (1 / mesh.scale_pos) * 32767;

	mesh.posa = new Int16Array(verts_x * verts_y * 4);
	mesh.nora = new Int16Array(verts_x * verts_y * 2);
	mesh.texa = new Int16Array(verts_x * verts_y * 2);
	mesh.inda = new Uint32Array((verts_x - 1) * (verts_y - 1) * 6);
	let step_x: f32 = size_x / (verts_x - 1);
	let step_y: f32 = size_y / (verts_y - 1);
	for (let i: i32 = 0; i < verts_x * verts_y; ++i) {
		let x: f32 = (i % verts_x) * step_x - half_x;
		let y: f32 = math_floor(i / verts_x) * step_y - half_y;
		mesh.posa[i * 4    ] = math_floor(x * inv);
		mesh.posa[i * 4 + 1] = math_floor(y * inv);
		mesh.posa[i * 4 + 2] = 0;
		mesh.nora[i * 2    ] = 0;
		mesh.nora[i * 2 + 1] = 0;
		mesh.posa[i * 4 + 3] = 32767;
		x = (i % verts_x) / (verts_x - 1);
		y = 1.0 - math_floor(i / verts_x) / (verts_y - 1);
		mesh.texa[i * 2    ] = (math_floor(x * 32767 * uv_scale) - 1) % 32767;
		mesh.texa[i * 2 + 1] = (math_floor(y * 32767 * uv_scale) - 1) % 32767;
	}
	for (let i: i32 = 0; i < (verts_x - 1) * (verts_y - 1); ++i) {
		let x: f32 = i % (verts_x - 1);
		let y: f32 = math_floor(i / (verts_y - 1));
		mesh.inda[i * 6    ] = y * verts_x + x;
		mesh.inda[i * 6 + 1] = y * verts_x + x + 1;
		mesh.inda[i * 6 + 2] = (y + 1) * verts_x + x;
		mesh.inda[i * 6 + 3] = y * verts_x + x + 1;
		mesh.inda[i * 6 + 4] = (y + 1) * verts_x + x + 1;
		mesh.inda[i * 6 + 5] = (y + 1) * verts_x + x;
	}

	return mesh;
}

function geom_make_uv_sphere(radius: f32 = 1.0, widthSegments: i32 = 32, heightSegments: i32 = 16, stretch_uv: bool = true, uvScale: f32 = 1.0): raw_mesh_t {

	let mesh: raw_mesh_t = {};
	mesh.scale_pos = 1.0;
	mesh.scale_tex = 1.0;
	mesh.name = "";
	mesh.has_next = false;

	// Pack positions to (-1, 1) range
	mesh.scale_pos = radius;
	mesh.scale_tex = uvScale;
	let inv: f32 = (1 / mesh.scale_pos) * 32767;
	let pi2: f32 = math_pi() * 2;

	let width_verts: i32 = widthSegments + 1;
	let height_verts: i32 = heightSegments + 1;
	mesh.posa = new Int16Array(width_verts * height_verts * 4);
	mesh.nora = new Int16Array(width_verts * height_verts * 2);
	mesh.texa = new Int16Array(width_verts * height_verts * 2);
	mesh.inda = new Uint32Array(widthSegments * heightSegments * 6 - widthSegments * 6);

	let nor: vec4_t = vec4_create();
	let pos: i32 = 0;
	for (let y: i32 = 0; y < height_verts; ++y) {
		let v: f32 = y / heightSegments;
		let v_flip: f32 = 1.0 - v;
		if (!stretch_uv) v_flip /= 2;
		let u_off: f32 = y == 0 ? 0.5 / widthSegments : y == heightSegments ? -0.5 / widthSegments : 0.0;
		for (let x: i32 = 0; x < width_verts; ++x) {
			let u: f32 = x / widthSegments;
			let u_pi2: f32 = u * pi2;
			let v_pi: f32  = v * math_pi();
			let v_pi_sin: f32 = math_sin(v_pi);
			let vx: f32 = -radius * math_cos(u_pi2) * v_pi_sin;
			let vy: f32 =  radius * math_sin(u_pi2) * v_pi_sin;
			let vz: f32 = -radius * math_cos(v_pi);
			let i4: i32 = pos * 4;
			let i2: i32 = pos * 2;
			mesh.posa[i4    ] = math_floor(vx * inv);
			mesh.posa[i4 + 1] = math_floor(vy * inv);
			mesh.posa[i4 + 2] = math_floor(vz * inv);
			vec4_normalize(vec4_set(nor, vx, vy, vz));
			mesh.posa[i4 + 3] = math_floor(nor.z * 32767);
			mesh.nora[i2    ] = math_floor(nor.x * 32767);
			mesh.nora[i2 + 1] = math_floor(nor.y * 32767);
			mesh.texa[i2    ] = (math_floor((u + u_off) * 32767) - 1) % 32767;
			mesh.texa[i2 + 1] = (math_floor(v_flip      * 32767) - 1) % 32767;
			pos++;
		}
	}

	pos = 0;
	let height_segments1: i32 = heightSegments - 1;
	for (let y: i32 = 0; y < heightSegments; ++y) {
		for (let x: i32 = 0; x < widthSegments; ++x) {
			let x1: i32 = x + 1;
			let y1: i32 = y + 1;
			let a: f32 = y  * width_verts + x1;
			let b: f32 = y  * width_verts + x;
			let c: f32 = y1 * width_verts + x;
			let d: f32 = y1 * width_verts + x1;
			if (y > 0) {
				mesh.inda[pos++] = a;
				mesh.inda[pos++] = b;
				mesh.inda[pos++] = d;
			}
			if (y < height_segments1) {
				mesh.inda[pos++] = b;
				mesh.inda[pos++] = c;
				mesh.inda[pos++] = d;
			}
		}
	}

	return mesh;
}

type raw_mesh_t = {
	posa?: Int16Array;
	nora?: Int16Array;
	texa?: Int16Array;
	inda?: Uint32Array;
	scale_pos?: f32;
	scale_tex?: f32;
	name?: string;
	has_next?: bool;
};
