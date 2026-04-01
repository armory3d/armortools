
#include "../global.h"

char                        *str_tex_gabor = "\
fun gabor_hash3(p: float3): float3 { \
	var q: float3 = float3(dot(p, float3(127.1, 311.7, 74.7)), \
						   dot(p, float3(269.5, 183.3, 246.1)), \
						   dot(p, float3(113.5, 271.9, 124.6))); \
	return frac3(float3(sin(q.x) * 43758.5453, sin(q.y) * 43758.5453, sin(q.z) * 43758.5453)); \
} \
fun gabor_hash1(p: float3): float { \
	return frac(sin(dot(p, float3(12.9898, 78.233, 53.539))) * 43758.5453); \
} \
fun gabor_random_unit_vector(p: float3): float3 { \
	var h1: float = gabor_hash1(p); \
	var h2: float = gabor_hash1(p + float3(1.1, 1.1, 1.1)); \
	var theta: float = acos(2.0 * h1 - 1.0); \
	var phi: float = 2.0 * 3.14159 * h2; \
	var sin_theta: float = sin(theta); \
	return float3(sin_theta * cos(phi), sin_theta * sin(phi), cos(theta)); \
} \
fun tex_gabor(co: float3, scale: float, frequency: float, anisotropy: float, orientation: float3): float3 { \
	var p: float3 = co * scale; \
	var ip: float3 = floor3(p); \
	var fp: float3 = frac3(p); \
	var value: float = 0.0; \
	var intensity: float = 0.0; \
	var phase_sin: float = 0.0; \
	var phase_cos: float = 0.0; \
	var pi: float = 3.14159; \
	var a: float = 1.0; \
	for (var k: int = 0; k <= 2; k += 1) { \
		for (var j: int = 0; j <= 2; j += 1) { \
			for (var i: int = 0; i <= 2; i += 1) { \
				var b: float3 = float3(float(i - 1), float(j - 1), float(k - 1)); \
				var h: float3 = gabor_hash3(ip + b); \
				var r: float3 = b - fp + h; \
				var dir: float3 = normalize(orientation); \
				if (anisotropy < 1.0) { \
					var hr_p: float3 = ip + b + float3(2.2, 2.2, 2.2);\
					var hr: float = gabor_hash1(hr_p); \
					if (hr > anisotropy) { \
						var dir_p: float3 = ip + b + float3(3.3, 3.3, 3.3);\
						dir = gabor_random_unit_vector(dir_p); \
					} \
				} \
				var dot_rd: float = dot(r, dir); \
				var r_parallel: float3 = dot_rd * dir; \
				var r_perp: float3 = r - r_parallel; \
				var a_parallel: float = a * (1.0 - anisotropy) + 0.001; \
				var a_perp: float = a; \
				var d_eff: float = a_parallel * a_parallel * dot(r_parallel, r_parallel) + a_perp * a_perp * dot(r_perp, r_perp); \
				var g: float = exp(-pi * d_eff); \
				var random_phase: float = 2.0 * pi * gabor_hash1(ip + b + float3(1.1, 1.1, 1.1)); \
				var theta: float = 2.0 * pi * frequency * dot_rd + random_phase; \
				value += g * sin(theta); \
				intensity += g; \
				phase_sin += sin(theta); \
				phase_cos += cos(theta); \
			} \
		} \
	} \
	intensity = intensity * 0.5 + 0.5; \
	var phase: float = atan2(phase_sin, phase_cos) / (2.0 * pi) + 0.5; \
	return float3(value, phase, intensity); \
} \
";

void gabor_texture_node_init() {

	char *gabor_dimensions_data = string("%s\n%s", _tr("2D"), _tr("3D"));
	gabor_texture_node_def =
	    GC_ALLOC_INIT(ui_node_t, {.id     = 0,
	                              .name   = _tr("Gabor Texture"),
	                              .type   = "TEX_GABOR",
	                              .x      = 0,
	                              .y      = 0,
	                              .color  = 0xff4982a0,
	                              .inputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Vector"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(0.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Scale"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(5.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Frequency"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(1.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 10.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Anisotropy"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Orientation"),
	                                                                       .type          = "VECTOR",
	                                                                       .color         = 0xff6363c7,
	                                                                       .default_value = f32_array_create_xyz(1.0, 0.0, 0.0),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  5),
	                              .outputs = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Value"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Phase"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                      GC_ALLOC_INIT(ui_node_socket_t, {.id            = 0,
	                                                                       .node_id       = 0,
	                                                                       .name          = _tr("Intensity"),
	                                                                       .type          = "VALUE",
	                                                                       .color         = 0xffa1a1a1,
	                                                                       .default_value = f32_array_create_x(0.5),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .display       = 0}),
	                                  },
	                                  3),
	                              .buttons = any_array_create_from_raw(
	                                  (void *[]){
	                                      GC_ALLOC_INIT(ui_node_button_t, {.name          = _tr("Dimensions"),
	                                                                       .type          = "ENUM",
	                                                                       .output        = -1,
	                                                                       .default_value = f32_array_create_x(1),
	                                                                       .data          = u8_array_create_from_string(gabor_dimensions_data),
	                                                                       .min           = 0.0,
	                                                                       .max           = 1.0,
	                                                                       .precision     = 100,
	                                                                       .height        = 0}),
	                                  },
	                                  1),
	                              .width = 0,
	                              .flags = 0});
	gc_root(gabor_texture_node_def);

	any_array_push(nodes_material_texture, gabor_texture_node_def);
	any_map_set(parser_material_node_values, "TEX_GABOR", gabor_texture_node_value);
}

char *gabor_texture_node_value(ui_node_t *node, ui_node_socket_t *socket) {
	node_shader_add_function(parser_material_kong, str_tex_gabor);
	char *co          = parser_material_get_coord(node);
	char *scale       = parser_material_parse_value_input(node->inputs->buffer[1], false);
	char *frequency   = parser_material_parse_value_input(node->inputs->buffer[2], false);
	char *anisotropy  = parser_material_parse_value_input(node->inputs->buffer[3], false);
	char *orientation = parser_material_parse_vector_input(node->inputs->buffer[4]);
	char *res         = string("tex_gabor(%s, %s, %s, %s, %s)", co, scale, frequency, anisotropy, orientation);
	if (socket == node->outputs->buffer[0]) { // Value
		return string("%s.x", res);
	}
	else if (socket == node->outputs->buffer[1]) { // Phase
		return string("%s.y", res);
	}
	else { // Intensity
		return string("%s.z", res);
	}
}
