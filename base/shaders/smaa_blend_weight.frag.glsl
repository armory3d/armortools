#version 450

#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_SEARCHTEX_SIZE vec2(66.0, 33.0)
#define SMAA_SEARCHTEX_PACKED_SIZE vec2(64.0, 16.0)
#define SMAA_CORNER_ROUNDING 25
#define SMAA_CORNER_ROUNDING_NORM (float(SMAA_CORNER_ROUNDING) / 100.0)
#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_SEARCHTEX_SELECT(sample) sample.r
#define mad(a, b, c) (a * b + c)
#define saturate(a) clamp(a, 0.0, 1.0)
#define round(a) floor(a + 0.5)

uniform sampler2D edges_tex;
uniform sampler2D area_tex;
uniform sampler2D search_tex;
uniform vec2 screen_size;
uniform vec2 screen_size_inv;

in vec2 tex_coord;
in vec2 pixcoord;
in vec4 offset0;
in vec4 offset1;
in vec4 offset2;
out vec4 frag_color;

vec2 cdw_end;

vec4 textureLod_a(sampler2D edges_tex, vec2 coord, float lod) {
#ifdef GLSL
#else
	coord.y = 1.0 - coord.y;
#endif
	return textureLod(edges_tex, coord, lod);
}

#define smaa_sample_level_zero_offset(edges_tex, coord, offset) textureLod_a(edges_tex, coord + offset * screen_size_inv.xy, 0.0)

vec2 smaa_decode_diag_bilinear_access(vec2 e) {
	e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);
	return round(e);
}

vec4 smaa_decode_diag_bilinear_access(vec4 e) {
	e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);
	return round(e);
}

vec2 smaa_search_diag1(vec2 texcoord, vec2 dir) {
	vec4 coord = vec4(texcoord, -1.0, 1.0);
	vec3 t = vec3(screen_size_inv.xy, 1.0);
	while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && coord.w > 0.9) {
		coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);
		cdw_end = textureLod_a(edges_tex, coord.xy, 0.0).rg;
		coord.w = dot(cdw_end /*e*/, vec2(0.5, 0.5));
	}
	return coord.zw;
}

vec2 smaa_search_diag2(vec2 texcoord, vec2 dir) {
	vec4 coord = vec4(texcoord, -1.0, 1.0);
	coord.x += 0.25 * screen_size_inv.x;
	vec3 t = vec3(screen_size_inv.xy, 1.0);
	float cw = coord.w; // TODO: krafix hlsl bug
	while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && cw > 0.9) {
		coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);
		cdw_end = textureLod_a(edges_tex, coord.xy, 0.0).rg;
		cdw_end = smaa_decode_diag_bilinear_access(cdw_end);
		cw = dot(cdw_end, vec2(0.5, 0.5));
	}
	coord.w = cw;
	return coord.zw;
}

vec2 smaa_area_diag(vec2 dist, vec2 e, float offset) {
	vec2 texcoord = mad(vec2(SMAA_AREATEX_MAX_DISTANCE_DIAG, SMAA_AREATEX_MAX_DISTANCE_DIAG), e, dist);
	texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);
	texcoord.x += 0.5;
	texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;
	return SMAA_AREATEX_SELECT(textureLod(area_tex, texcoord, 0.0));
}

vec2 smaa_calculate_diag_weights(vec2 texcoord, vec2 e, vec4 subsample_indices) {
	vec2 weights = vec2(0.0, 0.0);

	vec4 d;
	if (e.r > 0.0) {
		d.xz = smaa_search_diag1(texcoord, vec2(-1.0,  1.0));
		float dadd = cdw_end.y > 0.9 ? 1.0 : 0.0;
		d.x += dadd;
	}
	else {
		d.xz = vec2(0.0, 0.0);
	}
	d.yw = smaa_search_diag1(texcoord, vec2(1.0, -1.0));

	if (d.x + d.y > 2.0) {
		vec4 coords = mad(vec4(-d.x + 0.25, d.x, d.y, -d.y - 0.25), screen_size_inv.xyxy, texcoord.xyxy);
		vec4 c;

		c.xy = smaa_sample_level_zero_offset(edges_tex, coords.xy, vec2(-1,  0)).rg;
		c.zw = smaa_sample_level_zero_offset(edges_tex, coords.zw, vec2( 1,  0)).rg;
		c.yxwz = smaa_decode_diag_bilinear_access(c.xyzw);

		vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

		float a1condx = step(0.9, d.z);
		float a1condy = step(0.9, d.w);
		if (a1condx == 1.0) cc.x = 0.0;
		if (a1condy == 1.0) cc.y = 0.0;

		weights += smaa_area_diag(d.xy, cc, subsample_indices.z);
	}

	d.xz = smaa_search_diag2(texcoord, vec2(-1.0, -1.0));
	if (smaa_sample_level_zero_offset(edges_tex, texcoord, vec2(1, 0)).r > 0.0) {
		d.yw = smaa_search_diag2(texcoord, vec2(1.0, 1.0));
		float dadd = cdw_end.y > 0.9 ? 1.0 : 0.0;
		d.y += dadd;
	}
	else {
		d.yw = vec2(0.0, 0.0);
	}

	if (d.x + d.y > 2.0) {
		vec4 coords = mad(vec4(-d.x, -d.x, d.y, d.y), screen_size_inv.xyxy, texcoord.xyxy);
		vec4 c;
		c.x  = smaa_sample_level_zero_offset(edges_tex, coords.xy, vec2(-1,  0)).g;
		c.y  = smaa_sample_level_zero_offset(edges_tex, coords.xy, vec2( 0, -1)).r;
		c.zw = smaa_sample_level_zero_offset(edges_tex, coords.zw, vec2( 1,  0)).gr;
		vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

		float a1condx = step(0.9, d.z);
		float a1condy = step(0.9, d.w);
		if (a1condx == 1.0) cc.x = 0.0;
		if (a1condy == 1.0) cc.y = 0.0;

		weights += smaa_area_diag(d.xy, cc, subsample_indices.w).gr;
	}

	return weights;
}

float smaa_search_length(vec2 e, float offset) {
	vec2 scale = SMAA_SEARCHTEX_SIZE * vec2(0.5, -1.0);
	vec2 bias = SMAA_SEARCHTEX_SIZE * vec2(offset, 1.0);

	scale += vec2(-1.0, 1.0);
	bias += vec2( 0.5, -0.5);

	scale *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
	bias *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;

	vec2 coord = mad(scale, e, bias);

	return SMAA_SEARCHTEX_SELECT(textureLod(search_tex, coord, 0.0));
}

float smaa_search_x_left(vec2 texcoord, float end) {
	vec2 e = vec2(0.0, 1.0);
	while (texcoord.x > end && e.g > 0.8281 && e.r == 0.0) {
		e = textureLod_a(edges_tex, texcoord, 0.0).rg;
		texcoord = mad(-vec2(2.0, 0.0), screen_size_inv.xy, texcoord);
	}

	float offset = mad(-(255.0 / 127.0), smaa_search_length(e, 0.0), 3.25);
	return mad(screen_size_inv.x, offset, texcoord.x);
}

float smaa_search_x_right(vec2 texcoord, float end) {
	vec2 e = vec2(0.0, 1.0);
	while (texcoord.x < end && e.g > 0.8281 && e.r == 0.0) {
		e = textureLod_a(edges_tex, texcoord, 0.0).rg;
		texcoord = mad(vec2(2.0, 0.0), screen_size_inv.xy, texcoord);
	}

	float offset = mad(-(255.0 / 127.0), smaa_search_length(e, 0.5), 3.25);
	return mad(-screen_size_inv.x, offset, texcoord.x);
}

float smaa_search_y_up(vec2 texcoord, float end) {
	vec2 e = vec2(1.0, 0.0);
	while (texcoord.y > end && e.r > 0.8281 && e.g == 0.0) {
		e = textureLod_a(edges_tex, texcoord, 0.0).rg;
		texcoord = mad(-vec2(0.0, 2.0), screen_size_inv.xy, texcoord);
	}
	float offset = mad(-(255.0 / 127.0), smaa_search_length(e.gr, 0.0), 3.25);
	return mad(screen_size_inv.y, offset, texcoord.y);
}

float smaa_search_y_down(vec2 texcoord, float end) {
	vec2 e = vec2(1.0, 0.0);
	while (texcoord.y < end && e.r > 0.8281 && e.g == 0.0) {
		e = textureLod_a(edges_tex, texcoord, 0.0).rg;
		texcoord = mad(vec2(0.0, 2.0), screen_size_inv.xy, texcoord);
	}
	float offset = mad(-(255.0 / 127.0), smaa_search_length(e.gr, 0.5), 3.25);
	return mad(-screen_size_inv.y, offset, texcoord.y);
}

vec2 smaa_area(vec2 dist, float e1, float e2, float offset) {
	vec2 texcoord = mad(vec2(SMAA_AREATEX_MAX_DISTANCE, SMAA_AREATEX_MAX_DISTANCE), round(4.0 * vec2(e1, e2)), dist);
	texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);
	texcoord.y = mad(SMAA_AREATEX_SUBTEX_SIZE, offset, texcoord.y);
	return SMAA_AREATEX_SELECT(textureLod(area_tex, texcoord, 0.0));
}

vec2 smaa_detect_horizontal_corner_pattern(vec2 weights, vec4 texcoord, vec2 d) {
	vec2 left_right = step(d.xy, d.yx);
	vec2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * left_right;

	rounding /= left_right.x + left_right.y;

	vec2 factor = vec2(1.0, 1.0);
	factor.x -= rounding.x * smaa_sample_level_zero_offset(edges_tex, texcoord.xy, vec2(0,  1)).r;
	factor.x -= rounding.y * smaa_sample_level_zero_offset(edges_tex, texcoord.zw, vec2(1,  1)).r;
	factor.y -= rounding.x * smaa_sample_level_zero_offset(edges_tex, texcoord.xy, vec2(0, -2)).r;
	factor.y -= rounding.y * smaa_sample_level_zero_offset(edges_tex, texcoord.zw, vec2(1, -2)).r;

	weights *= saturate(factor);
	return weights;
}

vec2 smaa_detect_vertical_corner_pattern(vec2 weights, vec4 texcoord, vec2 d) {
	vec2 left_right = step(d.xy, d.yx);
	vec2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * left_right;

	rounding /= left_right.x + left_right.y;

	vec2 factor = vec2(1.0, 1.0);
	factor.x -= rounding.x * smaa_sample_level_zero_offset(edges_tex, texcoord.xy, vec2( 1, 0)).g;
	factor.x -= rounding.y * smaa_sample_level_zero_offset(edges_tex, texcoord.zw, vec2( 1, 1)).g;
	factor.y -= rounding.x * smaa_sample_level_zero_offset(edges_tex, texcoord.xy, vec2(-2, 0)).g;
	factor.y -= rounding.y * smaa_sample_level_zero_offset(edges_tex, texcoord.zw, vec2(-2, 1)).g;

	weights *= saturate(factor);
	return weights;
}


vec4 smaa_blending_weight_calculation_ps(vec2 texcoord, vec2 pixcoord, vec4 subsample_indices) {
	vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);

	vec2 e = textureLod_a(edges_tex, texcoord, 0.0).rg;

	if (e.g > 0.0) {
		weights.rg = smaa_calculate_diag_weights(texcoord, e, subsample_indices);
		if (weights.r == -weights.g) {
			vec2 d;

			vec3 coords;
			coords.x = smaa_search_x_left(offset0.xy, offset2.x);
			coords.y = offset1.y;
			d.x = coords.x;

			float e1 = textureLod_a(edges_tex, coords.xy, 0.0).r;

			coords.z = smaa_search_x_right(offset0.zw, offset2.y);
			d.y = coords.z;

			d = abs(round(mad(screen_size.xx, d, -pixcoord.xx)));

			vec2 sqrt_d = sqrt(d);

			float e2 = smaa_sample_level_zero_offset(edges_tex, coords.zy, vec2(1, 0)).r;

			weights.rg = smaa_area(sqrt_d, e1, e2, subsample_indices.y);

			coords.y = texcoord.y;
			weights.rg = smaa_detect_horizontal_corner_pattern(weights.rg, coords.xyzy, d);
		}
		else {
			e.r = 0.0;
		}
	}

	if (e.r > 0.0) {
		vec2 d;

		vec3 coords;
		coords.y = smaa_search_y_up(offset1.xy, offset2.z);
		coords.x = offset0.x;
		d.x = coords.y;

		float e1 = textureLod_a(edges_tex, coords.xy, 0.0).g;

		coords.z = smaa_search_y_down(offset1.zw, offset2.w);
		d.y = coords.z;

		d = abs(round(mad(screen_size.yy, d, -pixcoord.yy)));

		vec2 sqrt_d = sqrt(d);

		float e2 = smaa_sample_level_zero_offset(edges_tex, coords.xz, vec2(0, 1)).g;

		weights.ba = smaa_area(sqrt_d, e1, e2, subsample_indices.x);

		coords.x = texcoord.x;
		weights.ba = smaa_detect_vertical_corner_pattern(weights.ba, coords.xyxz, d);
	}

	return weights;
}

void main() {
	frag_color = smaa_blending_weight_calculation_ps(tex_coord, pixcoord, vec4(0.0, 0.0, 0.0, 0.0));
}
