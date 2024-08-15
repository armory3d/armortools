#version 450

uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D texmask;
uniform sampler2D texa;
uniform float opac;
uniform int blending;

in vec2 tex_coord;
out vec4 frag_color;

vec3 hsv_to_rgb(const vec3 c) {
	const vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
	vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
	return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

vec3 rgb_to_hsv(const vec3 c) {
	const vec4 K = vec4(0.0, -1.0 / 3.0, 2.0 / 3.0, -1.0);
	vec4 p = mix(vec4(c.bg, K.wz), vec4(c.gb, K.xy), step(c.b, c.g));
	vec4 q = mix(vec4(p.xyw, c.r), vec4(c.r, p.yzx), step(p.x, c.r));
	float d = q.x - min(q.w, q.y);
	float e = 1.0e-10;
	return vec3(abs(q.z + (q.w - q.y) / (6.0 * d + e)), d / (q.x + e), q.x);
}

void main() {
	vec4 col0 = textureLod(tex0, tex_coord, 0);
	vec4 cola = textureLod(texa, tex_coord, 0);
	float str = col0.a * opac;
	str *= textureLod(texmask, tex_coord, 0).r;
	if (blending == -1) { // Merging _nor and _pack
		vec4 col1 = textureLod(tex1, tex_coord, 0);
		frag_color = vec4(mix(cola, col1, str));
	}
	else if (blending == -2) { // Merging _nor with normal blending
		vec4 col1 = textureLod(tex1, tex_coord, 0);
		// Whiteout blend
		vec3 n1 = cola.rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);
		vec3 n2 = mix(vec3(0.5, 0.5, 1.0), col1.rgb, str) * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);
		frag_color = vec4(normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), max(col1.a, cola.a));
	}
	else if (blending == -3) { // Merging _pack with height blending
		vec4 col1 = textureLod(tex1, tex_coord, 0);
		frag_color = vec4(mix(cola.rgb, col1.rgb, str), cola.a + col1.a);
	}
	else if (blending == -4) { // Merge _pack.height into _nor
		float tex_step = 1.0 / textureSize(tex1, 0).x;
		float height0 = textureLod(tex1, vec2(tex_coord.x - tex_step, tex_coord.y), 0.0).a;
		float height1 = textureLod(tex1, vec2(tex_coord.x + tex_step, tex_coord.y), 0.0).a;
		float height2 = textureLod(tex1, vec2(tex_coord.x, tex_coord.y - tex_step), 0.0).a;
		float height3 = textureLod(tex1, vec2(tex_coord.x, tex_coord.y + tex_step), 0.0).a;
		float height_dx = height0 - height1;
		float height_dy = height2 - height3;
		// Whiteout blend
		vec3 n1 = col0.rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);
		vec3 n2 = normalize(vec3(height_dx * 16.0, height_dy * 16.0, 1.0));
		frag_color = vec4(normalize(vec3(n1.xy + n2.xy, n1.z * n2.z)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), col0.a);
	}
	else if (blending == 0) { // Mix
		frag_color = vec4(mix(cola.rgb, col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 1) { // Darken
		frag_color = vec4(mix(cola.rgb, min(cola.rgb, col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 2) { // Multiply
		frag_color = vec4(mix(cola.rgb, cola.rgb * col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 3) { // Burn
		frag_color = vec4(mix(cola.rgb, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - cola.rgb) / col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 4) { // Lighten
		frag_color = vec4(max(cola.rgb, col0.rgb * str), max(col0.a, cola.a));
	}
	else if (blending == 5) { // Screen
		frag_color = vec4((vec3(1.0, 1.0, 1.0) - (vec3(1.0 - str, 1.0 - str, 1.0 - str) + str * (vec3(1.0, 1.0, 1.0) - col0.rgb)) * (vec3(1.0, 1.0, 1.0) - cola.rgb)), max(col0.a, cola.a));
	}
	else if (blending == 6) { // Dodge
		frag_color = vec4(mix(cola.rgb, cola.rgb / (vec3(1.0, 1.0, 1.0) - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 7) { // Add
		frag_color = vec4(mix(cola.rgb, cola.rgb + col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 8) { // Overlay
		frag_color = vec4(mix(cola.rgb, vec3(
			cola.r < 0.5 ? 2.0 * cola.r * col0.r : 1.0 - 2.0 * (1.0 - cola.r) * (1.0 - col0.r),
			cola.g < 0.5 ? 2.0 * cola.g * col0.g : 1.0 - 2.0 * (1.0 - cola.g) * (1.0 - col0.g),
			cola.b < 0.5 ? 2.0 * cola.b * col0.b : 1.0 - 2.0 * (1.0 - cola.b) * (1.0 - col0.b)
		), str), max(col0.a, cola.a));
	}
	else if (blending == 9) { // Soft Light
		frag_color = vec4(((1.0 - str) * cola.rgb + str * ((vec3(1.0, 1.0, 1.0) - cola.rgb) * col0.rgb * cola.rgb + cola.rgb * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - col0.rgb) * (vec3(1.0, 1.0, 1.0) - cola.rgb)))), max(col0.a, cola.a));
	}
	else if (blending == 10) { // Linear Light
		frag_color = vec4((cola.rgb + str * (vec3(2.0, 2.0, 2.0) * (col0.rgb - vec3(0.5, 0.5, 0.5)))), max(col0.a, cola.a));
	}
	else if (blending == 11) { // Difference
		frag_color = vec4(mix(cola.rgb, abs(cola.rgb - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 12) { // Subtract
		frag_color = vec4(mix(cola.rgb, cola.rgb - col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 13) { // Divide
		frag_color = vec4(vec3(1.0 - str, 1.0 - str, 1.0 - str) * cola.rgb + vec3(str, str, str) * cola.rgb / col0.rgb, max(col0.a, cola.a));
	}
	else if (blending == 14) { // Hue
		frag_color = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 15) { // Saturation
		frag_color = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 16) { // Color
		frag_color = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else { // Value
		frag_color = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(col0.rgb).b)), str), max(col0.a, cola.a));
	}
}
