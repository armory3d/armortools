
let util_mesh_unwrappers: map_t<string, any> = map_create(); // JSValue * -> ((a: raw_mesh_t)=>void)

function util_mesh_merge(paint_objects: mesh_object_t[] = null) {
	if (paint_objects == null) {
		if (context_raw.tool == tool_type_t.GIZMO) {
			paint_objects = util_mesh_ext_get_unique();
		}
		else {
			paint_objects = project_paint_objects;
		}
	}
	if (paint_objects.length == 0) {
		return;
	}
	context_raw.merged_object_is_atlas = paint_objects.length < project_paint_objects.length;
	let vlen: i32 = 0;
	let ilen: i32 = 0;
	let max_scale: f32 = 0.0;
	for (let i: i32 = 0; i < paint_objects.length; ++i) {
		vlen += paint_objects[i].data.vertex_arrays[0].values.length;
		ilen += paint_objects[i].data.index_array.length;
		if (paint_objects[i].data.scale_pos > max_scale) {
			max_scale = paint_objects[i].data.scale_pos;
		}
	}
	vlen = math_floor(vlen / 4);
	let va0: i16_array_t = i16_array_create(vlen * 4);
	let va1: i16_array_t = i16_array_create(vlen * 2);
	let va2: i16_array_t = i16_array_create(vlen * 2);
	let va3: i16_array_t = paint_objects[0].data.vertex_arrays.length > 3 ? i16_array_create(vlen * 4) : null;
	let ia: u32_array_t = u32_array_create(ilen);

	let voff: i32 = 0;
	let ioff: i32 = 0;
	for (let i: i32 = 0; i < paint_objects.length; ++i) {
		let vas: vertex_array_t[] = paint_objects[i].data.vertex_arrays;
		let ias: u32_array_t = paint_objects[i].data.index_array;
		let scale: f32 = paint_objects[i].data.scale_pos;

		// Pos
		for (let j: i32 = 0; j < vas[0].values.length; ++j) {
			va0[j + voff * 4] = vas[0].values[j];
		}

		// Translate
		// for (let j: i32 = 0; j < math_floor(va0.length / 4); ++j) {
		// 	va0[j * 4     + voff * 4] += math_floor(transform_world_x(paint_objects[i].base.transform) * 32767);
		// 	va0[j * 4 + 1 + voff * 4] += math_floor(transform_world_y(paint_objects[i].base.transform) * 32767);
		// 	va0[j * 4 + 2 + voff * 4] += math_floor(transform_world_z(paint_objects[i].base.transform) * 32767);
		// }

		// Re-scale
		for (let j: i32 = voff; j < math_floor(va0.length / 4); ++j) {
			va0[j * 4    ] = math_floor((va0[j * 4    ] * scale) / max_scale);
			va0[j * 4 + 1] = math_floor((va0[j * 4 + 1] * scale) / max_scale);
			va0[j * 4 + 2] = math_floor((va0[j * 4 + 2] * scale) / max_scale);
		}

		// Nor
		for (let j: i32 = 0; j < vas[1].values.length; ++j) {
			va1[j + voff * 2] = vas[1].values[j];
		}
		// Tex
		for (let j: i32 = 0; j < vas[2].values.length; ++j) {
			va2[j + voff * 2] = vas[2].values[j];
		}
		// Col
		if (va3 != null) {
			for (let j: i32 = 0; j < vas[3].values.length; ++j) {
				va3[j + voff * 4] = vas[3].values[j];
			}
		}
		// Indices
		for (let j: i32 = 0; j < ias.length; ++j) {
			ia[j + ioff] = ias[j] + voff;
		}

		voff += math_floor(vas[0].values.length / 4);
		ioff += math_floor(ias.length);
	}

	let raw: mesh_data_t = {
		name: context_raw.paint_object.base.name,
		vertex_arrays: [
			{
				values: va0,
				attrib: "pos",
				data: "short4norm"
			},
			{
				values: va1,
				attrib: "nor",
				data: "short2norm"
			},
			{
				values: va2,
				attrib: "tex",
				data: "short2norm"
			}
		],
		index_array: ia,
		scale_pos: max_scale,
		scale_tex: 1.0
	};
	if (va3 != null) {
		let col: vertex_array_t = {
			values: va3,
			attrib: "col",
			data: "short4norm"
		};
		array_push(raw.vertex_arrays, col);
	}

	util_mesh_remove_merged();
	let md: mesh_data_t = mesh_data_create(raw);
	context_raw.merged_object = mesh_object_create(md, context_raw.paint_object.materials);
	context_raw.merged_object.base.name = context_raw.paint_object.base.name + "_merged";
	context_raw.merged_object.force_context = "paint";
	object_set_parent(context_raw.merged_object.base, context_main_object().base);

	render_path_raytrace_ready = false;
}

function util_mesh_remove_merged() {
	if (context_raw.merged_object != null) {
		mesh_data_delete(context_raw.merged_object.data);
		mesh_object_remove(context_raw.merged_object);
		context_raw.merged_object = null;
	}
}

function util_mesh_swap_axis(a: i32, b: i32) {
	let objects: mesh_object_t[] = project_paint_objects;
	for (let i: i32 = 0; i < objects.length; ++i) {
		let o: mesh_object_t = objects[i];
		// Remapping vertices, buckle up
		// 0 - x, 1 - y, 2 - z
		let vas: vertex_array_t[] = o.data.vertex_arrays;
		let pa: i16_array_t  = vas[0].values;
		let na0: i16_array_t = a == 2 ? vas[0].values : vas[1].values;
		let na1: i16_array_t = b == 2 ? vas[0].values : vas[1].values;
		let c: i32 = a == 2 ? 3 : a;
		let d: i32 = b == 2 ? 3 : b;
		let e: i32 = a == 2 ? 4 : 2;
		let f: i32 = b == 2 ? 4 : 2;
		for (let i: i32 = 0; i < math_floor(pa.length / 4); ++i) {
			let t: i32 = pa[i * 4 + a];
			pa[i * 4 + a] = pa[i * 4 + b];
			pa[i * 4 + b] = -t;
			t = na0[i * e + c];
			na0[i * e + c] = na1[i * f + d];
			na1[i * f + d] = -t;
		}
		let g: mesh_data_t = o.data;
		mesh_data_build_vertices(g._.vertex_buffer, vas);
	}

	util_mesh_remove_merged();
	util_mesh_merge();
}

function util_mesh_flip_normals() {
	let objects: mesh_object_t[] = project_paint_objects;
	for (let i: i32 = 0; i < objects.length; ++i) {
		let o: mesh_object_t = objects[i];
		let vas: vertex_array_t[] = o.data.vertex_arrays;
		let va0: i16_array_t = vas[0].values;
		let va1: i16_array_t = vas[1].values;
		for (let i: i32 = 0; i < va0.length / 4; ++i) {
			va0[i * 4 + 3] = -va0[i * 4 + 3];
			va1[i * 2] = -va1[i * 2];
			va1[i * 2 + 1] = -va1[i * 2 + 1];
		}
		let g: mesh_data_t = o.data;
		mesh_data_build_vertices(g._.vertex_buffer, vas);
	}

	render_path_raytrace_ready = false;
}

function util_mesh_calc_normals(smooth: bool = false) {
	let va: vec4_t = vec4_create();
    let vb: vec4_t = vec4_create();
    let vc: vec4_t = vec4_create();
    let cb: vec4_t = vec4_create();
    let ab: vec4_t = vec4_create();
    let objects: mesh_object_t[] = project_paint_objects;
    for (let i: i32 = 0; i < objects.length; ++i) {
        let o: mesh_object_t = objects[i];
        let g: mesh_data_t = o.data;
        let inda: u32_array_t = g.index_array;
        let va0: i16_array_t = o.data.vertex_arrays[0].values;
        let va1: i16_array_t = o.data.vertex_arrays[1].values;
        let num_verts: i32 = math_floor(va0.length / 4);
        for (let i: i32 = 0; i < math_floor(inda.length / 3); ++i) {
            let i1: i32 = inda[i * 3    ];
            let i2: i32 = inda[i * 3 + 1];
            let i3: i32 = inda[i * 3 + 2];
            va = vec4_create(va0[i1 * 4    ], va0[i1 * 4 + 1], va0[i1 * 4 + 2]);
            vb = vec4_create(va0[i2 * 4    ], va0[i2 * 4 + 1], va0[i2 * 4 + 2]);
            vc = vec4_create(va0[i3 * 4    ], va0[i3 * 4 + 1], va0[i3 * 4 + 2]);
            cb = vec4_sub(vc, vb);
            ab = vec4_sub(va, vb);
            cb = vec4_cross(cb, ab);
            cb = vec4_norm(cb);
            let nx: i32 = math_floor(cb.x * 32767);
            let ny: i32 = math_floor(cb.y * 32767);
            let nz: i32 = math_floor(cb.z * 32767);
            va1[i1 * 2    ] = nx;
            va1[i1 * 2 + 1] = ny;
            va0[i1 * 4 + 3] = nz;
            va1[i2 * 2    ] = nx;
            va1[i2 * 2 + 1] = ny;
            va0[i2 * 4 + 3] = nz;
            va1[i3 * 2    ] = nx;
            va1[i3 * 2 + 1] = ny;
            va0[i3 * 4 + 3] = nz;
        }
		if (smooth) {
			let shared: u32_array_t = u32_array_create(1024);
			let shared_len: i32 = 0;
			let found: i32[] = [];
			for (let i: i32 = 0; i < (inda.length - 1); ++i) {
				if (array_index_of(found, i) >= 0) {
					continue;
				}
				let i1: i32 = inda[i];
				shared_len = 0;
				shared[shared_len++] = i1;
				for (let j: i32 = (i + 1); j < inda.length; ++j) {
					let i2: i32 = inda[j];
					if (va0[i1 * 4    ] == va0[i2 * 4    ] &&
						va0[i1 * 4 + 1] == va0[i2 * 4 + 1] &&
						va0[i1 * 4 + 2] == va0[i2 * 4 + 2]) {
						// if (n1.dot(n2) > 0)
						shared[shared_len++] = i2;
						array_push(found, j);
						if (shared_len >= 1024) {
							break;
						}
					}
				}
				if (shared_len > 1) {
					va = vec4_create(0, 0, 0);
					for (let j: i32 = 0; j < shared_len; ++j) {
						let i1: i32 = shared[j];
						va = vec4_fadd(va,
							va1[i1 * 2    ],
							va1[i1 * 2 + 1],
							va0[i1 * 4 + 3]);
					}
					va = vec4_mult(va, 1 / shared_len);
					va = vec4_norm(va);
					let vax: i32 = math_floor(va.x * 32767);
					let vay: i32 = math_floor(va.y * 32767);
					let vaz: i32 = math_floor(va.z * 32767);
					for (let j: i32 = 0; j < shared_len; ++j) {
						let i1: i32 = shared[j];
						va1[i1 * 2    ] = vax;
						va1[i1 * 2 + 1] = vay;
						va0[i1 * 4 + 3] = vaz;
					}
				}
			}
		}

		mesh_data_build_vertices(g._.vertex_buffer, o.data.vertex_arrays);
	}

	util_mesh_merge();
	render_path_raytrace_ready = false;
}

function util_mesh_to_origin() {
	let dx: f32 = 0.0;
	let dy: f32 = 0.0;
	let dz: f32 = 0.0;
	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let o: mesh_object_t = project_paint_objects[i];
		let l: i32 = 4;
		let sc: f32 = o.data.scale_pos / 32767;
		let va: i16_array_t = o.data.vertex_arrays[0].values;
		let minx: f32 = va[0];
		let maxx: f32 = va[0];
		let miny: f32 = va[1];
		let maxy: f32 = va[1];
		let minz: f32 = va[2];
		let maxz: f32 = va[2];
		for (let i: i32 = 1; i < math_floor(va.length / l); ++i) {
			if (va[i * l] < minx) {
				minx = va[i * l];
			}
			else if (va[i * l] > maxx) {
				maxx = va[i * l];
			}
			if (va[i * l + 1] < miny) {
				miny = va[i * l + 1];
			}
			else if (va[i * l + 1] > maxy) {
				maxy = va[i * l + 1];
			}
			if (va[i * l + 2] < minz) {
				minz = va[i * l + 2];
			}
			else if (va[i * l + 2] > maxz) {
				maxz = va[i * l + 2];
			}
		}
		dx += (minx + maxx) / 2 * sc;
		dy += (miny + maxy) / 2 * sc;
		dz += (minz + maxz) / 2 * sc;
	}
	dx /= project_paint_objects.length;
	dy /= project_paint_objects.length;
	dz /= project_paint_objects.length;

	for (let i: i32 = 0; i < project_paint_objects.length; ++i) {
		let o: mesh_object_t = project_paint_objects[i];
		let g: mesh_data_t = o.data;
		let sc: f32 = o.data.scale_pos / 32767;
		let va: i16_array_t = o.data.vertex_arrays[0].values;
		let max_scale: f32 = 0.0;
		for (let i: i32 = 0; i < math_floor(va.length / 4); ++i) {
			if (math_abs(va[i * 4    ] * sc - dx) > max_scale) {
				max_scale = math_abs(va[i * 4    ] * sc - dx);
			}
			if (math_abs(va[i * 4 + 1] * sc - dy) > max_scale) {
				max_scale = math_abs(va[i * 4 + 1] * sc - dy);
			}
			if (math_abs(va[i * 4 + 2] * sc - dz) > max_scale) {
				max_scale = math_abs(va[i * 4 + 2] * sc - dz);
			}
		}
		o.base.transform.scale_world = o.data.scale_pos = o.data.scale_pos = max_scale;
		transform_build_matrix(o.base.transform);

		for (let i: i32 = 0; i < math_floor(va.length / 4); ++i) {
			va[i * 4    ] = math_floor((va[i * 4    ] * sc - dx) / max_scale * 32767);
			va[i * 4 + 1] = math_floor((va[i * 4 + 1] * sc - dy) / max_scale * 32767);
			va[i * 4 + 2] = math_floor((va[i * 4 + 2] * sc - dz) / max_scale * 32767);
		}

		mesh_data_build_vertices(g._.vertex_buffer, o.data.vertex_arrays);
	}

	util_mesh_merge();
}

function util_mesh_apply_displacement(texpaint_pack: gpu_texture_t, strength: f32 = 0.1, uv_scale: f32 = 1.0) {
	let height: buffer_t = gpu_get_texture_pixels(texpaint_pack);
    let res: i32 = texpaint_pack.width;
    let o: mesh_object_t = project_paint_objects[0];
    let g: mesh_data_t = o.data;
    let va0: i16_array_t = g.vertex_arrays[0].values;
    let va1: i16_array_t = g.vertex_arrays[1].values;
    let va2: i16_array_t = g.vertex_arrays[2].values;
    let num_verts: i32 = math_floor(va0.length / 4);
    for (let i: i32 = 0; i < num_verts; ++i) {
        let x: i32 = math_floor(va2[i * 2] / 32767 * res);
        let y: i32 = math_floor(va2[i * 2 + 1] / 32767 * res);
        let ix: i32 = math_floor(x * uv_scale);
        let iy: i32 = math_floor(y * uv_scale);
        let xx: i32 = ix % res;
        let yy: i32 = iy % res;
        let h: f32 = (1.0 - buffer_get_u8(height, (yy * res + xx) * 4 + 3) / 255) * strength;
        va0[i * 4    ] -= math_floor(va1[i * 2    ] * h);
        va0[i * 4 + 1] -= math_floor(va1[i * 2 + 1] * h);
        va0[i * 4 + 2] -= math_floor(va0[i * 4 + 3] * h);
    }
	mesh_data_build_vertices(g._.vertex_buffer, o.data.vertex_arrays);
}

function util_mesh_equirect_unwrap(mesh: raw_mesh_t) {
	let verts: i32 = math_floor(mesh.posa.length / 4);
	mesh.texa = i16_array_create(verts * 2);
	let n: vec4_t = vec4_create();
	for (let i: i32 = 0; i < verts; ++i) {
		n = vec4_create(mesh.posa[i * 4] / 32767, mesh.posa[i * 4 + 1] / 32767, mesh.posa[i * 4 + 2] / 32767);
		n = vec4_norm(n);
		// Sphere projection
		// mesh.texa[i * 2    ] = math_atan2(n.x, n.y) / (math_pi() * 2) + 0.5;
		// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
		// Equirect
		mesh.texa[i * 2    ] = math_floor(((math_atan2(-n.z, n.x) + math_pi()) / (math_pi() * 2)) * 32767);
		mesh.texa[i * 2 + 1] = math_floor((math_acos(n.y) / math_pi()) * 32767);
	}
}

function util_mesh_pnpoly(v0x: f32, v0y: f32, v1x: f32, v1y: f32, v2x: f32, v2y: f32, px: f32, py: f32): bool {
	// https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
	let c: bool = false;
	if (((v0y > py) != (v2y > py)) && (px < (v2x - v0x) * (py - v0y) / (v2y - v0y) + v0x)) {
		c = !c;
	}
	if (((v1y > py) != (v0y > py)) && (px < (v0x - v1x) * (py - v1y) / (v0y - v1y) + v1x)) {
		c = !c;
	}
	if (((v2y > py) != (v1y > py)) && (px < (v1x - v2x) * (py - v2y) / (v1y - v2y) + v2x)) {
		c = !c;
	}
	return c;
}

function util_mesh_calc_normal(p0: vec4_t, p1: vec4_t, p2: vec4_t): vec4_t {
	let cb: vec4_t = vec4_sub(p2, p1);
	let ab: vec4_t = vec4_sub(p0, p1);
	cb = vec4_cross(cb, ab);
	cb = vec4_norm(cb);
	return cb;
}

function util_mesh_decimate() {
	let o: mesh_object_t = project_paint_objects[0];
	let vas: vertex_array_t[] = o.data.vertex_arrays;
	let posa: i16_array_t = vas[0].values;
	let nora: i16_array_t = vas[1].values;
	let texa: i16_array_t = vas[2].values;
	let inda: u32_array_t = o.data.index_array;

	let mesh: raw_mesh_t = {
		posa: posa,
		nora: nora,
		texa: texa,
		cola: null,
		inda: inda,
		vertex_count: posa.length / 4,
		index_count: inda.length,
		scale_pos: o.data.scale_pos,
		scale_tex: 1.0,
		name: "Decimated",
	};
	// decimate_mesh(mesh);
	import_mesh_add_mesh(mesh);
}
