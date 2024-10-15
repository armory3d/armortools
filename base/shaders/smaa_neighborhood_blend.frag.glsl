#version 450

#define _Veloc

uniform sampler2D color_tex;
uniform sampler2D blend_tex;
#ifdef _Veloc
uniform sampler2D sveloc;
#endif
uniform vec2 screen_size_inv;

in vec2 tex_coord;
in vec4 offset;
out vec4 frag_color;

vec4 textureLodA_color_tex(sampler2D color_tex, vec2 coords, float lod) {
#ifdef GLSL
#else
	coords.y = 1.0 - coords.y;
#endif
	return textureLod(color_tex, coords, lod);
}

vec4 textureLodA_sveloc(sampler2D sveloc, vec2 coords, float lod) {
#ifdef GLSL
#else
	coords.y = 1.0 - coords.y;
#endif

	return textureLod(sveloc, coords, lod);
}

vec4 smaa_neighborhood_blending_ps(vec2 texcoord, vec4 offset) {
	vec4 a;
	a.x = textureLod(blend_tex, offset.xy, 0.0).a; // Right
	a.y = textureLod(blend_tex, offset.zw, 0.0).g; // Top
	a.wz = textureLod(blend_tex, texcoord, 0.0).xz; // Bottom / Left

	if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1e-5) {
		vec4 color = textureLod(color_tex, texcoord, 0.0);

#ifdef _Veloc
		vec2 velocity = textureLod(sveloc, tex_coord, 0.0).rg;
		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	else {
		bool h = max(a.x, a.z) > max(a.y, a.w);

		vec4 blending_offset = vec4(0.0, a.y, 0.0, a.w);
		vec2 blending_weight = a.yw;

		if (h) {
			blending_offset.x = a.x;
			blending_offset.y = 0.0;
			blending_offset.z = a.z;
			blending_offset.w = 0.0;
			blending_weight.x = a.x;
			blending_weight.y = a.z;
		}

		blending_weight /= dot(blending_weight, vec2(1.0, 1.0));

#ifdef GLSL
		vec2 tc = texcoord;
#else
		vec2 tc = vec2(texcoord.x, 1.0 - texcoord.y);
#endif
		vec4 blending_coord = blending_offset * vec4(screen_size_inv.xy, -screen_size_inv.xy) + tc.xyxy;

		vec4 color = blending_weight.x * textureLodA_color_tex(color_tex, blending_coord.xy, 0.0);
		color += blending_weight.y * textureLodA_color_tex(color_tex, blending_coord.zw, 0.0);

#ifdef _Veloc
		vec2 velocity = blending_weight.x * textureLodA_sveloc(sveloc, blending_coord.xy, 0.0).rg;
		velocity += blending_weight.y * textureLodA_sveloc(sveloc, blending_coord.zw, 0.0).rg;

		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	return vec4(0.0, 0.0, 0.0, 0.0);
}

void main() {
	frag_color = smaa_neighborhood_blending_ps(tex_coord, offset);
}
