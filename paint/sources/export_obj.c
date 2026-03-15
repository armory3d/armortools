
#include "global.h"

void export_obj_write_string(u8_array_t *out, char *str) {
	for (i32 i = 0; i < string_length(str); ++i) {
		u8_array_push(out, char_code_at(str, i));
	}
}

void export_obj_run(char *path, mesh_object_t_array_t *paint_objects, bool apply_disp) {
	u8_array_t *o = u8_array_create_from_raw((u8[]){}, 0);
	export_obj_write_string(o, "# armorpaint.org\n");

	i32 poff = 0;
	i32 noff = 0;
	i32 toff = 0;
	for (i32 i = 0; i < paint_objects->length; ++i) {
		mesh_object_t *p    = paint_objects->buffer[i];
		mesh_data_t   *mesh = p->data;
		f32            inv  = 1 / 32767.0;
		f32            sc   = p->data->scale_pos * inv;
		i16_array_t   *posa = mesh->vertex_arrays->buffer[0]->values;
		i16_array_t   *nora = mesh->vertex_arrays->buffer[1]->values;
		i16_array_t   *texa = mesh->vertex_arrays->buffer[2]->values;
		i32            len  = math_floor(posa->length / 4.0);

		// Merge shared vertices and remap indices
		i16_array_t *posa2  = i16_array_create(len * 3);
		i16_array_t *nora2  = i16_array_create(len * 3);
		i16_array_t *texa2  = i16_array_create(len * 2);
		i32_array_t *posmap = i32_array_create(len);
		i32_array_t *normap = i32_array_create(len);
		i32_array_t *texmap = i32_array_create(len);

		i32 pi = 0;
		i32 ni = 0;
		i32 ti = 0;
		for (i32 i = 0; i < len; ++i) {
			bool found = false;
			for (i32 j = 0; j < pi; ++j) {
				if (posa2->buffer[j * 3] == posa->buffer[i * 4] && posa2->buffer[j * 3 + 1] == posa->buffer[i * 4 + 1] &&
				    posa2->buffer[j * 3 + 2] == posa->buffer[i * 4 + 2]) {
					posmap->buffer[i] = j;
					found             = true;
					break;
				}
			}
			if (!found) {
				posmap->buffer[i]         = pi;
				posa2->buffer[pi * 3]     = posa->buffer[i * 4];
				posa2->buffer[pi * 3 + 1] = posa->buffer[i * 4 + 1];
				posa2->buffer[pi * 3 + 2] = posa->buffer[i * 4 + 2];
				pi++;
			}

			found = false;
			for (i32 j = 0; j < ni; ++j) {
				if (nora2->buffer[j * 3] == nora->buffer[i * 2] && nora2->buffer[j * 3 + 1] == nora->buffer[i * 2 + 1] &&
				    nora2->buffer[j * 3 + 2] == posa->buffer[i * 4 + 3]) {
					normap->buffer[i] = j;
					found             = true;
					break;
				}
			}
			if (!found) {
				normap->buffer[i]         = ni;
				nora2->buffer[ni * 3]     = nora->buffer[i * 2];
				nora2->buffer[ni * 3 + 1] = nora->buffer[i * 2 + 1];
				nora2->buffer[ni * 3 + 2] = posa->buffer[i * 4 + 3];
				ni++;
			}

			found = false;
			for (i32 j = 0; j < ti; ++j) {
				if (texa2->buffer[j * 2] == texa->buffer[i * 2] && texa2->buffer[j * 2 + 1] == texa->buffer[i * 2 + 1]) {
					texmap->buffer[i] = j;
					found             = true;
					break;
				}
			}
			if (!found) {
				texmap->buffer[i]         = ti;
				texa2->buffer[ti * 2]     = texa->buffer[i * 2];
				texa2->buffer[ti * 2 + 1] = texa->buffer[i * 2 + 1];
				ti++;
			}
		}
		if (apply_disp) {
			// let height: buffer_t = gpu_get_texture_pixels(layers[0].texpaint_pack);
			// let res: i32 = layers[0].texpaint_pack.width;
			// let strength: f32 = 0.1;
			// for (let i: i32 = 0; i < len; ++i) {
			// 	let x: i32 = math_floor(texa2[i * 2    ] / 32767 * res);
			// 	let y: i32 = math_floor((1.0 - texa2[i * 2 + 1] / 32767) * res);
			// 	let h: f32 = (1.0 - height.get((y * res + x) * 4 + 3) / 255) * strength;
			// 	posa2[i * 3    ] -= math_floor(nora2[i * 3    ] * inv * h / sc);
			// 	posa2[i * 3 + 1] -= math_floor(nora2[i * 3 + 1] * inv * h / sc);
			// 	posa2[i * 3 + 2] -= math_floor(nora2[i * 3 + 2] * inv * h / sc);
			// }
		}

		export_obj_write_string(o, string("o %s\n", p->base->name));
		for (i32 i = 0; i < pi; ++i) {
			export_obj_write_string(o, "v ");
			f32 f = posa2->buffer[i * 3] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = posa2->buffer[i * 3 + 2] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = -posa2->buffer[i * 3 + 1] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}
		for (i32 i = 0; i < ni; ++i) {
			export_obj_write_string(o, "vn ");
			f32 f = nora2->buffer[i * 3] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = nora2->buffer[i * 3 + 2] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = -nora2->buffer[i * 3 + 1] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}
		for (i32 i = 0; i < ti; ++i) {
			export_obj_write_string(o, "vt ");
			f32 f = texa2->buffer[i * 2] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = 1.0 - texa2->buffer[i * 2 + 1] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}

		u32_array_t *inda = mesh->index_array;
		for (i32 i = 0; i < math_floor(inda->length / 3.0); ++i) {
			i32 pi1 = posmap->buffer[inda->buffer[i * 3]] + 1 + poff;
			i32 pi2 = posmap->buffer[inda->buffer[i * 3 + 1]] + 1 + poff;
			i32 pi3 = posmap->buffer[inda->buffer[i * 3 + 2]] + 1 + poff;
			i32 ni1 = normap->buffer[inda->buffer[i * 3]] + 1 + noff;
			i32 ni2 = normap->buffer[inda->buffer[i * 3 + 1]] + 1 + noff;
			i32 ni3 = normap->buffer[inda->buffer[i * 3 + 2]] + 1 + noff;
			i32 ti1 = texmap->buffer[inda->buffer[i * 3]] + 1 + toff;
			i32 ti2 = texmap->buffer[inda->buffer[i * 3 + 1]] + 1 + toff;
			i32 ti3 = texmap->buffer[inda->buffer[i * 3 + 2]] + 1 + toff;
			export_obj_write_string(o, "f ");
			export_obj_write_string(o, i32_to_string(pi1));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti1));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni1));
			export_obj_write_string(o, " ");
			export_obj_write_string(o, i32_to_string(pi2));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti2));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni2));
			export_obj_write_string(o, " ");
			export_obj_write_string(o, i32_to_string(pi3));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti3));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni3));
			export_obj_write_string(o, "\n");
		}
		poff += pi;
		noff += ni;
		toff += ti;
	}

	if (!ends_with(path, ".obj")) {
		path = string("%s.obj", path);
	}
	iron_file_save_bytes(path, o, 0);
}

void export_obj_run_fast(char *path, mesh_object_t_array_t *paint_objects) {
	// Skips merging shared vertices

	u8_array_t *o = u8_array_create_from_raw((u8[]){}, 0);
	export_obj_write_string(o, "# armorpaint.org\n");

	i32 poff = 0;
	i32 noff = 0;
	i32 toff = 0;
	for (i32 i = 0; i < paint_objects->length; ++i) {
		mesh_object_t *p    = paint_objects->buffer[i];
		mesh_data_t   *mesh = p->data;
		f32            inv  = 1 / 32767.0;
		f32            sc   = p->data->scale_pos * inv;
		i16_array_t   *posa = mesh->vertex_arrays->buffer[0]->values;
		i16_array_t   *nora = mesh->vertex_arrays->buffer[1]->values;
		i16_array_t   *texa = mesh->vertex_arrays->buffer[2]->values;

		i32 pi = posa->length / 4.0;
		i32 ni = pi;
		i32 ti = pi;

		export_obj_write_string(o, string("o %s\n", p->base->name));
		for (i32 i = 0; i < pi; ++i) {
			export_obj_write_string(o, "v ");
			f32 f = posa->buffer[i * 4] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = posa->buffer[i * 4 + 2] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = -posa->buffer[i * 4 + 1] * sc;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}
		for (i32 i = 0; i < ni; ++i) {
			export_obj_write_string(o, "vn ");
			f32 f = nora->buffer[i * 2] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = posa->buffer[i * 4 + 3] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = -nora->buffer[i * 2 + 1] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}
		for (i32 i = 0; i < ti; ++i) {
			export_obj_write_string(o, "vt ");
			f32 f = texa->buffer[i * 2] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, " ");
			f = 1.0 - texa->buffer[i * 2 + 1] * inv;
			export_obj_write_string(o, f32_to_string(f));
			export_obj_write_string(o, "\n");
		}

		u32_array_t *inda = mesh->index_array;
		for (i32 i = 0; i < math_floor(inda->length / 3.0); ++i) {
			i32 pi1 = inda->buffer[i * 3] + 1 + poff;
			i32 pi2 = inda->buffer[i * 3 + 1] + 1 + poff;
			i32 pi3 = inda->buffer[i * 3 + 2] + 1 + poff;
			i32 ni1 = inda->buffer[i * 3] + 1 + noff;
			i32 ni2 = inda->buffer[i * 3 + 1] + 1 + noff;
			i32 ni3 = inda->buffer[i * 3 + 2] + 1 + noff;
			i32 ti1 = inda->buffer[i * 3] + 1 + toff;
			i32 ti2 = inda->buffer[i * 3 + 1] + 1 + toff;
			i32 ti3 = inda->buffer[i * 3 + 2] + 1 + toff;
			export_obj_write_string(o, "f ");
			export_obj_write_string(o, i32_to_string(pi1));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti1));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni1));
			export_obj_write_string(o, " ");
			export_obj_write_string(o, i32_to_string(pi2));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti2));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni2));
			export_obj_write_string(o, " ");
			export_obj_write_string(o, i32_to_string(pi3));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ti3));
			export_obj_write_string(o, "/");
			export_obj_write_string(o, i32_to_string(ni3));
			export_obj_write_string(o, "\n");
		}
		poff += pi;
		noff += ni;
		toff += ti;
	}

	if (!ends_with(path, ".obj")) {
		path = string("%s.obj", path);
	}
	iron_file_save_bytes(path, o, 0);
}
