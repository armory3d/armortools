
#include "global.h"

void util_mesh_merge(mesh_object_t_array_t *paint_objects) {
	if (paint_objects == NULL) {
		if (context_raw->tool == TOOL_TYPE_GIZMO) {
			paint_objects = util_mesh_get_unique();
		}
		else {
			paint_objects = project_paint_objects;
		}
	}
	if (paint_objects->length == 0) {
		return;
	}
	context_raw->merged_object_is_atlas = paint_objects->length < project_paint_objects->length;
	i32 vlen                            = 0;
	i32 ilen                            = 0;
	f32 max_scale                       = 0.0;
	for (i32 i = 0; i < paint_objects->length; ++i) {
		vlen += paint_objects->buffer[i]->data->vertex_arrays->buffer[0]->values->length;
		ilen += paint_objects->buffer[i]->data->index_array->length;
		if (paint_objects->buffer[i]->data->scale_pos > max_scale) {
			max_scale = paint_objects->buffer[i]->data->scale_pos;
		}
	}
	vlen                = math_floor(vlen / 4.0);
	i16_array_t *va0    = i16_array_create(vlen * 4);
	i16_array_t *va1    = i16_array_create(vlen * 2);
	i16_array_t *va2    = i16_array_create(vlen * 2);
	i16_array_t *vatex1 = mesh_data_get_vertex_array(paint_objects->buffer[0]->data, "tex1") != NULL ? i16_array_create(vlen * 2) : NULL;
	i16_array_t *vacol  = mesh_data_get_vertex_array(paint_objects->buffer[0]->data, "col") != NULL ? i16_array_create(vlen * 4) : NULL;
	i32          tex1i  = 3;
	i32          coli   = vatex1 != NULL ? 4 : 3;
	u32_array_t *ia     = u32_array_create(ilen);

	i32 voff = 0;
	i32 ioff = 0;
	for (i32 i = 0; i < paint_objects->length; ++i) {
		vertex_array_t_array_t *vas   = paint_objects->buffer[i]->data->vertex_arrays;
		u32_array_t            *ias   = paint_objects->buffer[i]->data->index_array;
		f32                     scale = paint_objects->buffer[i]->data->scale_pos;

		// Pos
		for (i32 j = 0; j < vas->buffer[0]->values->length; ++j) {
			va0->buffer[j + voff * 4] = vas->buffer[0]->values->buffer[j];
		}

		// Translate
		// for (let j: i32 = 0; j < math_floor(va0.length / 4); ++j) {
		// 	va0[j * 4     + voff * 4] += math_floor(transform_world_x(paint_objects[i].base.transform) * 32767);
		// 	va0[j * 4 + 1 + voff * 4] += math_floor(transform_world_y(paint_objects[i].base.transform) * 32767);
		// 	va0[j * 4 + 2 + voff * 4] += math_floor(transform_world_z(paint_objects[i].base.transform) * 32767);
		// }

		// Re-scale
		for (i32 j = voff; j < math_floor(va0->length / 4.0); ++j) {
			va0->buffer[j * 4]     = math_floor((va0->buffer[j * 4] * scale) / (float)max_scale);
			va0->buffer[j * 4 + 1] = math_floor((va0->buffer[j * 4 + 1] * scale) / (float)max_scale);
			va0->buffer[j * 4 + 2] = math_floor((va0->buffer[j * 4 + 2] * scale) / (float)max_scale);
		}

		// Nor
		for (i32 j = 0; j < vas->buffer[1]->values->length; ++j) {
			va1->buffer[j + voff * 2] = vas->buffer[1]->values->buffer[j];
		}
		// Tex
		for (i32 j = 0; j < vas->buffer[2]->values->length; ++j) {
			va2->buffer[j + voff * 2] = vas->buffer[2]->values->buffer[j];
		}
		// Tex1
		if (vatex1 != NULL) {
			for (i32 j = 0; j < vas->buffer[tex1i]->values->length; ++j) {
				vatex1->buffer[j + voff * 2] = vas->buffer[tex1i]->values->buffer[j];
			}
		}
		// Col
		if (vacol != NULL) {
			for (i32 j = 0; j < vas->buffer[coli]->values->length; ++j) {
				vacol->buffer[j + voff * 4] = vas->buffer[coli]->values->buffer[j];
			}
		}
		// Indices
		for (i32 j = 0; j < ias->length; ++j) {
			ia->buffer[j + ioff] = ias->buffer[j] + voff;
		}

		voff += math_floor(vas->buffer[0]->values->length / 4.0);
		ioff += math_floor(ias->length);
	}
	mesh_data_t *raw = GC_ALLOC_INIT(mesh_data_t, {.name          = context_raw->paint_object->base->name,
	                                               .vertex_arrays = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = va0, .attrib = "pos", .data = "short4norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = va1, .attrib = "nor", .data = "short2norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = va2, .attrib = "tex", .data = "short2norm"}),
	                                                   },
	                                                   3),
	                                               .index_array = ia,
	                                               .scale_pos   = max_scale,
	                                               .scale_tex   = 1.0});
	if (vatex1 != NULL) {
		vertex_array_t *va = GC_ALLOC_INIT(vertex_array_t, {.values = vatex1, .attrib = "tex1", .data = "short2norm"});
		any_array_push(raw->vertex_arrays, va);
	}
	if (vacol != NULL) {
		vertex_array_t *va = GC_ALLOC_INIT(vertex_array_t, {.values = vacol, .attrib = "col", .data = "short4norm"});
		any_array_push(raw->vertex_arrays, va);
	}
	util_mesh_remove_merged();
	mesh_data_t *md                           = mesh_data_create(raw);
	context_raw->merged_object                = mesh_object_create(md, context_raw->paint_object->material);
	context_raw->merged_object->base->name    = string("%s_merged", context_raw->paint_object->base->name);
	context_raw->merged_object->force_context = "paint";
	object_set_parent(context_raw->merged_object->base, context_main_object()->base);
	render_path_raytrace_ready = false;
}

void util_mesh_remove_merged() {
	if (context_raw->merged_object != NULL) {
		mesh_data_delete(context_raw->merged_object->data);
		mesh_object_remove(context_raw->merged_object);
		context_raw->merged_object = NULL;
	}
}

void util_mesh_swap_axis(i32 a, i32 b) {
	mesh_object_t_array_t *objects = project_paint_objects;
	for (i32 i = 0; i < objects->length; ++i) {
		mesh_object_t *o = objects->buffer[i];
		// Remapping vertices, buckle up
		// 0 - x, 1 - y, 2 - z
		vertex_array_t_array_t *vas = o->data->vertex_arrays;
		i16_array_t            *pa  = vas->buffer[0]->values;
		i16_array_t            *na0 = a == 2 ? vas->buffer[0]->values : vas->buffer[1]->values;
		i16_array_t            *na1 = b == 2 ? vas->buffer[0]->values : vas->buffer[1]->values;
		i32                     c   = a == 2 ? 3 : a;
		i32                     d   = b == 2 ? 3 : b;
		i32                     e   = a == 2 ? 4 : 2;
		i32                     f   = b == 2 ? 4 : 2;
		for (i32 i = 0; i < math_floor(pa->length / 4.0); ++i) {
			i32 t                  = pa->buffer[i * 4 + a];
			pa->buffer[i * 4 + a]  = pa->buffer[i * 4 + b];
			pa->buffer[i * 4 + b]  = -t;
			t                      = na0->buffer[i * e + c];
			na0->buffer[i * e + c] = na1->buffer[i * f + d];
			na1->buffer[i * f + d] = -t;
		}
		mesh_data_t *g = o->data;
		mesh_data_build_vertices(g->_->vertex_buffer, vas);
	}
	util_mesh_remove_merged();
	util_mesh_merge(NULL);
}

void util_mesh_flip_normals() {
	mesh_object_t_array_t *objects = project_paint_objects;
	for (i32 i = 0; i < objects->length; ++i) {
		mesh_object_t          *o   = objects->buffer[i];
		vertex_array_t_array_t *vas = o->data->vertex_arrays;
		i16_array_t            *va0 = vas->buffer[0]->values;
		i16_array_t            *va1 = vas->buffer[1]->values;
		for (i32 i = 0; i < va0->length / 4.0; ++i) {
			va0->buffer[i * 4 + 3] = -va0->buffer[i * 4 + 3];
			va1->buffer[i * 2]     = -va1->buffer[i * 2];
			va1->buffer[i * 2 + 1] = -va1->buffer[i * 2 + 1];
		}
		mesh_data_t *g = o->data;
		mesh_data_build_vertices(g->_->vertex_buffer, vas);
	}
	render_path_raytrace_ready = false;
}

i32 util_mesh_calc_normals_sort(i32 *pa, i32 *pb) {
	i32 a    = *(pa);
	i32 b    = *(pb);
	i32 diff = util_mesh_va0->buffer[a * 4] - util_mesh_va0->buffer[b * 4];
	if (diff != 0)
		return diff;
	diff = util_mesh_va0->buffer[a * 4 + 1] - util_mesh_va0->buffer[b * 4 + 1];
	if (diff != 0)
		return diff;
	return util_mesh_va0->buffer[a * 4 + 2] - util_mesh_va0->buffer[b * 4 + 2];
}

void util_mesh_calc_normals(bool smooth) {
	vec4_t                 va      = vec4_create(0.0, 0.0, 0.0, 1.0);
	vec4_t                 vb      = vec4_create(0.0, 0.0, 0.0, 1.0);
	vec4_t                 vc      = vec4_create(0.0, 0.0, 0.0, 1.0);
	vec4_t                 cb      = vec4_create(0.0, 0.0, 0.0, 1.0);
	vec4_t                 ab      = vec4_create(0.0, 0.0, 0.0, 1.0);
	mesh_object_t_array_t *objects = project_paint_objects;
	for (i32 i = 0; i < objects->length; ++i) {
		mesh_object_t *o           = objects->buffer[i];
		mesh_data_t   *g           = o->data;
		u32_array_t   *inda        = g->index_array;
		i16_array_t   *va0         = o->data->vertex_arrays->buffer[0]->values;
		i16_array_t   *va1         = o->data->vertex_arrays->buffer[1]->values;
		i32            num_verts   = math_floor(va0->length / 4.0);
		f32_array_t   *smooth_vals = NULL;
		i32_array_t   *vert_map    = NULL;
		if (smooth) {
			smooth_vals          = f32_array_create(num_verts * 3);
			vert_map             = i32_array_create(num_verts);
			i32_array_t *indices = i32_array_create_from_raw((i32[]){}, 0);
			for (i32 j = 0; j < num_verts; ++j) {
				i32_array_push(indices, j);
			}
			gc_unroot(util_mesh_va0);
			util_mesh_va0 = va0;
			gc_root(util_mesh_va0);
			i32_array_sort(indices, &util_mesh_calc_normals_sort);
			if (indices->length > 0) {
				i32 unique_id                        = indices->buffer[0];
				vert_map->buffer[indices->buffer[0]] = unique_id;
				for (i32 j = 1; j < indices->length; ++j) {
					i32 curr = indices->buffer[j];
					i32 prev = indices->buffer[j - 1];
					if (va0->buffer[curr * 4] == va0->buffer[prev * 4] && va0->buffer[curr * 4 + 1] == va0->buffer[prev * 4 + 1] &&
					    va0->buffer[curr * 4 + 2] == va0->buffer[prev * 4 + 2]) {
						vert_map->buffer[curr] = unique_id;
					}
					else {
						unique_id              = curr;
						vert_map->buffer[curr] = unique_id;
					}
				}
			}
		}

		for (i32 i = 0; i < math_floor(inda->length / 3.0); ++i) {
			i32 i1 = inda->buffer[i * 3];
			i32 i2 = inda->buffer[i * 3 + 1];
			i32 i3 = inda->buffer[i * 3 + 2];
			va     = vec4_create(va0->buffer[i1 * 4], va0->buffer[i1 * 4 + 1], va0->buffer[i1 * 4 + 2], 1.0);
			vb     = vec4_create(va0->buffer[i2 * 4], va0->buffer[i2 * 4 + 1], va0->buffer[i2 * 4 + 2], 1.0);
			vc     = vec4_create(va0->buffer[i3 * 4], va0->buffer[i3 * 4 + 1], va0->buffer[i3 * 4 + 2], 1.0);
			cb     = vec4_sub(vc, vb);
			ab     = vec4_sub(va, vb);
			cb     = vec4_cross(cb, ab);
			if (smooth) {
				i32 u1 = vert_map->buffer[i1];
				i32 u2 = vert_map->buffer[i2];
				i32 u3 = vert_map->buffer[i3];
				smooth_vals->buffer[u1 * 3] += cb.x;
				smooth_vals->buffer[u1 * 3 + 1] += cb.y;
				smooth_vals->buffer[u1 * 3 + 2] += cb.z;
				if (u2 != u1) {
					smooth_vals->buffer[u2 * 3] += cb.x;
					smooth_vals->buffer[u2 * 3 + 1] += cb.y;
					smooth_vals->buffer[u2 * 3 + 2] += cb.z;
				}
				if (u3 != u1 && u3 != u2) {
					smooth_vals->buffer[u3 * 3] += cb.x;
					smooth_vals->buffer[u3 * 3 + 1] += cb.y;
					smooth_vals->buffer[u3 * 3 + 2] += cb.z;
				}
			}
			else {
				cb                      = vec4_norm(cb);
				i32 nx                  = math_floor(cb.x * 32767);
				i32 ny                  = math_floor(cb.y * 32767);
				i32 nz                  = math_floor(cb.z * 32767);
				va1->buffer[i1 * 2]     = nx;
				va1->buffer[i1 * 2 + 1] = ny;
				va0->buffer[i1 * 4 + 3] = nz;
				va1->buffer[i2 * 2]     = nx;
				va1->buffer[i2 * 2 + 1] = ny;
				va0->buffer[i2 * 4 + 3] = nz;
				va1->buffer[i3 * 2]     = nx;
				va1->buffer[i3 * 2 + 1] = ny;
				va0->buffer[i3 * 4 + 3] = nz;
			}
		}

		if (smooth) {
			for (i32 j = 0; j < num_verts; ++j) {
				i32 u  = vert_map->buffer[j];
				f32 nx = smooth_vals->buffer[u * 3];
				f32 ny = smooth_vals->buffer[u * 3 + 1];
				f32 nz = smooth_vals->buffer[u * 3 + 2];
				f32 l  = math_sqrt(nx * nx + ny * ny + nz * nz);
				if (l > 0.0001) {
					nx /= l;
					ny /= l;
					nz /= l;
				}
				va1->buffer[j * 2]     = math_floor(nx * 32767);
				va1->buffer[j * 2 + 1] = math_floor(ny * 32767);
				va0->buffer[j * 4 + 3] = math_floor(nz * 32767);
			}
		}

		mesh_data_build_vertices(g->_->vertex_buffer, o->data->vertex_arrays);
	}

	util_mesh_merge(NULL);
	render_path_raytrace_ready = false;
}

void util_mesh_to_origin() {
	f32 dx = 0.0;
	f32 dy = 0.0;
	f32 dz = 0.0;
	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *o    = project_paint_objects->buffer[i];
		i32            l    = 4;
		f32            sc   = o->data->scale_pos / 32767.0;
		i16_array_t   *va   = o->data->vertex_arrays->buffer[0]->values;
		f32            minx = va->buffer[0];
		f32            maxx = va->buffer[0];
		f32            miny = va->buffer[1];
		f32            maxy = va->buffer[1];
		f32            minz = va->buffer[2];
		f32            maxz = va->buffer[2];
		for (i32 i = 1; i < math_floor(va->length / (float)l); ++i) {
			if (va->buffer[i * l] < minx) {
				minx = va->buffer[i * l];
			}
			else if (va->buffer[i * l] > maxx) {
				maxx = va->buffer[i * l];
			}
			if (va->buffer[i * l + 1] < miny) {
				miny = va->buffer[i * l + 1];
			}
			else if (va->buffer[i * l + 1] > maxy) {
				maxy = va->buffer[i * l + 1];
			}
			if (va->buffer[i * l + 2] < minz) {
				minz = va->buffer[i * l + 2];
			}
			else if (va->buffer[i * l + 2] > maxz) {
				maxz = va->buffer[i * l + 2];
			}
		}
		dx += (minx + maxx) / 2.0 * sc;
		dy += (miny + maxy) / 2.0 * sc;
		dz += (minz + maxz) / 2.0 * sc;
	}
	dx /= project_paint_objects->length;
	dy /= project_paint_objects->length;
	dz /= project_paint_objects->length;

	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		mesh_object_t *o         = project_paint_objects->buffer[i];
		mesh_data_t   *g         = o->data;
		f32            sc        = o->data->scale_pos / 32767.0;
		i16_array_t   *va        = o->data->vertex_arrays->buffer[0]->values;
		f32            max_scale = 0.0;
		for (i32 i = 0; i < math_floor(va->length / 4.0); ++i) {
			if (math_abs(va->buffer[i * 4] * sc - dx) > max_scale) {
				max_scale = math_abs(va->buffer[i * 4] * sc - dx);
			}
			if (math_abs(va->buffer[i * 4 + 1] * sc - dy) > max_scale) {
				max_scale = math_abs(va->buffer[i * 4 + 1] * sc - dy);
			}
			if (math_abs(va->buffer[i * 4 + 2] * sc - dz) > max_scale) {
				max_scale = math_abs(va->buffer[i * 4 + 2] * sc - dz);
			}
		}
		o->base->transform->scale_world = o->data->scale_pos = o->data->scale_pos = max_scale;
		transform_build_matrix(o->base->transform);

		for (i32 i = 0; i < math_floor(va->length / 4.0); ++i) {
			va->buffer[i * 4]     = math_floor((va->buffer[i * 4] * sc - dx) / (float)max_scale * 32767);
			va->buffer[i * 4 + 1] = math_floor((va->buffer[i * 4 + 1] * sc - dy) / (float)max_scale * 32767);
			va->buffer[i * 4 + 2] = math_floor((va->buffer[i * 4 + 2] * sc - dz) / (float)max_scale * 32767);
		}

		mesh_data_build_vertices(g->_->vertex_buffer, o->data->vertex_arrays);
	}

	util_mesh_merge(NULL);
}

void util_mesh_apply_displacement(gpu_texture_t *texpaint_pack, f32 strength, f32 uv_scale) {
	buffer_t      *height    = gpu_get_texture_pixels(texpaint_pack);
	i32            res       = texpaint_pack->width;
	mesh_object_t *o         = project_paint_objects->buffer[0];
	mesh_data_t   *g         = o->data;
	i16_array_t   *va0       = g->vertex_arrays->buffer[0]->values;
	i16_array_t   *va1       = g->vertex_arrays->buffer[1]->values;
	i16_array_t   *va2       = g->vertex_arrays->buffer[2]->values;
	i32            num_verts = math_floor(va0->length / 4.0);
	for (i32 i = 0; i < num_verts; ++i) {
		i32 x  = math_floor(va2->buffer[i * 2] / 32767.0 * res);
		i32 y  = math_floor(va2->buffer[i * 2 + 1] / 32767.0 * res);
		i32 ix = math_floor(x * uv_scale);
		i32 iy = math_floor(y * uv_scale);
		i32 xx = ix % res;
		i32 yy = iy % res;
		f32 h  = (1.0 - buffer_get_u8(height, (yy * res + xx) * 4 + 3) / 255.0) * strength;
		va0->buffer[i * 4] -= math_floor(va1->buffer[i * 2] * h);
		va0->buffer[i * 4 + 1] -= math_floor(va1->buffer[i * 2 + 1] * h);
		va0->buffer[i * 4 + 2] -= math_floor(va0->buffer[i * 4 + 3] * h);
	}
	mesh_data_build_vertices(g->_->vertex_buffer, o->data->vertex_arrays);
}

void util_mesh_equirect_unwrap(raw_mesh_t *mesh) {
	i32 verts  = math_floor(mesh->posa->length / 4.0);
	mesh->texa = i16_array_create(verts * 2);
	vec4_t n   = vec4_create(0.0, 0.0, 0.0, 1.0);
	for (i32 i = 0; i < verts; ++i) {
		n = vec4_create(mesh->posa->buffer[i * 4] / 32767.0, mesh->posa->buffer[i * 4 + 1] / 32767.0, mesh->posa->buffer[i * 4 + 2] / 32767.0,
		                1.0);
		n = vec4_norm(n);
		// Sphere projection
		// mesh.texa[i * 2    ] = math_atan2(n.x, n.y) / (math_pi() * 2) + 0.5;
		// mesh.texa[i * 2 + 1] = n.z * 0.5 + 0.5;
		// Equirect
		mesh->texa->buffer[i * 2]     = math_floor(((math_atan2(-n.z, n.x) + math_pi()) / (float)(math_pi() * 2)) * 32767);
		mesh->texa->buffer[i * 2 + 1] = math_floor((math_acos(n.y) / (float)math_pi()) * 32767);
	}
}

bool util_mesh_pnpoly(f32 v0x, f32 v0y, f32 v1x, f32 v1y, f32 v2x, f32 v2y, f32 px, f32 py) {
	// https://wrf.ecse.rpi.edu//Research/Short_Notes/pnpoly.html
	bool c = false;
	if (((v0y > py) != (v2y > py)) && (px < (v2x - v0x) * (py - v0y) / (float)(v2y - v0y) + v0x)) {
		c = !c;
	}
	if (((v1y > py) != (v0y > py)) && (px < (v0x - v1x) * (py - v1y) / (float)(v0y - v1y) + v1x)) {
		c = !c;
	}
	if (((v2y > py) != (v1y > py)) && (px < (v1x - v2x) * (py - v2y) / (float)(v1y - v2y) + v2x)) {
		c = !c;
	}
	return c;
}

vec4_t util_mesh_calc_normal(vec4_t p0, vec4_t p1, vec4_t p2) {
	vec4_t cb = vec4_sub(p2, p1);
	vec4_t ab = vec4_sub(p0, p1);
	cb        = vec4_cross(cb, ab);
	cb        = vec4_norm(cb);
	return cb;
}

i32 util_mesh_decimate_sort(i32 *pa, i32 *pb) {
	i32 a    = *(pa);
	i32 b    = *(pb);
	i32 diff = util_mesh_quantized->buffer[a * 3] - util_mesh_quantized->buffer[b * 3];
	if (diff != 0)
		return diff;
	diff = util_mesh_quantized->buffer[a * 3 + 1] - util_mesh_quantized->buffer[b * 3 + 1];
	if (diff != 0)
		return diff;
	return util_mesh_quantized->buffer[a * 3 + 2] - util_mesh_quantized->buffer[b * 3 + 2];
}

void util_mesh_decimate(f32 strength) {
	mesh_object_t_array_t *objects   = project_paint_objects;
	mesh_object_t         *o         = objects->buffer[0];
	mesh_data_t           *g         = o->data;
	i16_array_t           *va0       = g->vertex_arrays->buffer[0]->values;
	i16_array_t           *va1       = g->vertex_arrays->buffer[1]->values;
	i16_array_t           *va2       = g->vertex_arrays->buffer[2]->values;
	u32_array_t           *inda      = g->index_array;
	i32                    num_verts = math_floor(va0->length / 4.0);
	i32                    min_x     = 32767;
	i32                    max_x     = -32767;
	i32                    min_y     = 32767;
	i32                    max_y     = -32767;
	i32                    min_z     = 32767;
	i32                    max_z     = -32767;
	for (i32 i = 0; i < num_verts; ++i) {
		i32 x = va0->buffer[i * 4];
		i32 y = va0->buffer[i * 4 + 1];
		i32 z = va0->buffer[i * 4 + 2];
		if (x < min_x)
			min_x = x;
		if (x > max_x)
			max_x = x;
		if (y < min_y)
			min_y = y;
		if (y > max_y)
			max_y = y;
		if (z < min_z)
			min_z = z;
		if (z > max_z)
			max_z = z;
	}
	i32 box_size = math_max(max_x - min_x, math_max(max_y - min_y, max_z - min_z));

	f32 cells = 200.0 * (1.0 - strength);
	if (cells < 2.0)
		cells = 2.0;
	i32 cell_size = math_floor(box_size / (float)cells);
	if (cell_size < 1)
		cell_size = 1;

	gc_unroot(util_mesh_quantized);
	util_mesh_quantized = i32_array_create(num_verts * 3);
	gc_root(util_mesh_quantized);
	i32_array_t *indices = i32_array_create_from_raw((i32[]){}, 0);

	for (i32 i = 0; i < num_verts; ++i) {
		util_mesh_quantized->buffer[i * 3]     = math_floor((va0->buffer[i * 4] - min_x) / (float)cell_size);
		util_mesh_quantized->buffer[i * 3 + 1] = math_floor((va0->buffer[i * 4 + 1] - min_y) / (float)cell_size);
		util_mesh_quantized->buffer[i * 3 + 2] = math_floor((va0->buffer[i * 4 + 2] - min_z) / (float)cell_size);
		i32_array_push(indices, i);
	}

	i32_array_sort(indices, &util_mesh_decimate_sort);
	i32_array_t *remap           = i32_array_create(num_verts);
	i32          new_verts_count = 0;
	i32_array_t *unique_indices  = i32_array_create_from_raw((i32[]){}, 0);

	if (indices->length > 0) {
		i32 start_of_cell                 = 0;
		remap->buffer[indices->buffer[0]] = 0;
		for (i32 i = 1; i <= indices->length; ++i) {
			bool is_new_cell = false;
			if (i < indices->length) {
				i32 curr = indices->buffer[i];
				i32 prev = indices->buffer[i - 1];
				if (util_mesh_quantized->buffer[curr * 3] != util_mesh_quantized->buffer[prev * 3] ||
				    util_mesh_quantized->buffer[curr * 3 + 1] != util_mesh_quantized->buffer[prev * 3 + 1] ||
				    util_mesh_quantized->buffer[curr * 3 + 2] != util_mesh_quantized->buffer[prev * 3 + 2]) {
					is_new_cell = true;
				}
			}
			else {
				is_new_cell = true;
			}
			if (is_new_cell) {
				i32_array_push(unique_indices, indices->buffer[start_of_cell]);
				for (i32 k = start_of_cell; k < i; ++k) {
					remap->buffer[indices->buffer[k]] = new_verts_count;
				}
				new_verts_count++;
				start_of_cell = i;
			}
		}
	}

	i16_array_t *new_va0 = i16_array_create(new_verts_count * 4);
	i16_array_t *new_va1 = i16_array_create(new_verts_count * 2);
	i16_array_t *new_va2 = i16_array_create(new_verts_count * 2);
	for (i32 i = 0; i < new_verts_count; ++i) {
		i32 old_idx                = unique_indices->buffer[i];
		new_va0->buffer[i * 4]     = va0->buffer[old_idx * 4];
		new_va0->buffer[i * 4 + 1] = va0->buffer[old_idx * 4 + 1];
		new_va0->buffer[i * 4 + 2] = va0->buffer[old_idx * 4 + 2];
		new_va0->buffer[i * 4 + 3] = va0->buffer[old_idx * 4 + 3];
		new_va1->buffer[i * 2]     = va1->buffer[old_idx * 2];
		new_va1->buffer[i * 2 + 1] = va1->buffer[old_idx * 2 + 1];
		new_va2->buffer[i * 2]     = va2->buffer[old_idx * 2];
		new_va2->buffer[i * 2 + 1] = va2->buffer[old_idx * 2 + 1];
	}

	u32_array_t *new_inda = u32_array_create_from_raw((u32[]){}, 0);
	for (i32 i = 0; i < math_floor(inda->length / 3.0); ++i) {
		i32 i1 = remap->buffer[inda->buffer[i * 3]];
		i32 i2 = remap->buffer[inda->buffer[i * 3 + 1]];
		i32 i3 = remap->buffer[inda->buffer[i * 3 + 2]];
		if (i1 != i2 && i1 != i3 && i2 != i3) {
			u32_array_push(new_inda, i1);
			u32_array_push(new_inda, i2);
			u32_array_push(new_inda, i3);
		}
	}

	mesh_data_t *raw = GC_ALLOC_INIT(mesh_data_t, {.name          = string("%s_decimated", o->base->name),
	                                               .vertex_arrays = any_array_create_from_raw(
	                                                   (void *[]){
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = new_va0, .attrib = "pos", .data = "short4norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = new_va1, .attrib = "nor", .data = "short2norm"}),
	                                                       GC_ALLOC_INIT(vertex_array_t, {.values = new_va2, .attrib = "tex", .data = "short2norm"}),
	                                                   },
	                                                   3),
	                                               .index_array = u32_array_create_from_array(new_inda),
	                                               .scale_pos   = o->data->scale_pos,
	                                               .scale_tex   = 1.0});

	mesh_data_t *new_data = mesh_data_create(raw);
	o->data               = new_data;
	util_mesh_calc_normals(true);
#ifdef WITH_PLUGINS
	plugin_uv_unwrap_button();
#endif
}

i32 _util_mesh_unique_data_count() {
	return util_mesh_get_unique()->length;
}

void util_mesh_pack_uvs(i16_array_t *texa) {
	// Scale tex coords into global atlas
	i32 atlas_w      = config_get_scene_atlas_res();
	i32 item_i       = _util_mesh_unique_data_count() - 1; // Add the one being imported
	i32 item_w       = config_get_layer_res();
	i32 atlas_stride = atlas_w / (float)item_w;
	i32 atlas_step   = 32767 / (float)atlas_stride;
	i32 item_x       = (item_i % atlas_stride) * atlas_step;
	i32 item_y       = math_floor(item_i / (float)atlas_stride) * atlas_step;
	for (i32 i = 0; i < texa->length / 2.0; ++i) {
		texa->buffer[i * 2]     = texa->buffer[i * 2] / (float)atlas_stride + item_x;
		texa->buffer[i * 2 + 1] = texa->buffer[i * 2 + 1] / (float)atlas_stride + item_y;
	}
}

mesh_object_t_array_t *util_mesh_get_unique() {
	mesh_object_t_array_t *ar = any_array_create_from_raw((void *[]){}, 0);

	for (i32 i = 0; i < project_paint_objects->length; ++i) {
		if (!project_paint_objects->buffer[i]->base->visible) {
			continue;
		}
		bool found = false;
		for (i32 j = 0; j < i; ++j) {
			if (project_paint_objects->buffer[i]->data == project_paint_objects->buffer[j]->data) {
				found = true;
				break;
			}
		}
		if (!found) {
			any_array_push(ar, project_paint_objects->buffer[i]);
		}
	}

	return ar;
}
