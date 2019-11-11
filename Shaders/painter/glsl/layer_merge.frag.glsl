#version 330
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D texa;
uniform sampler2D texb;
uniform sampler2D texc;
uniform float opac;
uniform int blending;
in vec2 texCoord;
out vec4 FragColor[3];
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
	vec4 col0 = textureLod(tex0, texCoord, 0);
	vec4 col1 = textureLod(tex1, texCoord, 0);
	vec4 col2 = textureLod(tex2, texCoord, 0);
	vec4 cola = textureLod(texa, texCoord, 0);
	vec4 colb = textureLod(texb, texCoord, 0);
	vec4 colc = textureLod(texc, texCoord, 0);
	float str = col0.a * opac;
	if (blending == 0) { // Mix
		FragColor[0] = vec4(mix(cola.rgb, col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 1) { // Darken
		FragColor[0] = vec4(mix(cola.rgb, min(cola.rgb, col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 2) { // Multiply
		FragColor[0] = vec4(mix(cola.rgb, cola.rgb * col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 3) { // Burn
		FragColor[0] = vec4(mix(cola.rgb, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - cola.rgb) / col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 4) { // Lighten
		FragColor[0] = vec4(max(cola.rgb, col0.rgb * str), max(col0.a, cola.a));
	}
	else if (blending == 5) { // Screen
		FragColor[0] = vec4((vec3(1.0, 1.0, 1.0) - (vec3(1.0 - str, 1.0 - str, 1.0 - str) + str * (vec3(1.0, 1.0, 1.0) - col0.rgb)) * (vec3(1.0, 1.0, 1.0) - cola.rgb)), max(col0.a, cola.a));
	}
	else if (blending == 6) { // Dodge
		FragColor[0] = vec4(mix(cola.rgb, cola.rgb / (vec3(1.0, 1.0, 1.0) - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 7) { // Add
		FragColor[0] = vec4(mix(cola.rgb, cola.rgb + col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 8) { // Overlay
		//FragColor[0] = vec4(mix(cola.rgb, (cola.rgb < vec3(0.5, 0.5, 0.5) ? vec3(2.0, 2.0, 2.0) * cola.rgb * col0.rgb : vec3(1.0, 1.0, 1.0) - vec3(2.0, 2.0, 2.0) * (vec3(1.0, 1.0, 1.0) - col0.rgb) * (vec3(1.0, 1.0, 1.0) - cola.rgb)), str), max(col0.a, cola.a)); // TODO
		FragColor[0] = vec4(mix(cola.rgb, col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 9) { // Soft Light
		FragColor[0] = vec4(((1.0 - str) * cola.rgb + str * ((vec3(1.0, 1.0, 1.0) - cola.rgb) * col0.rgb * cola.rgb + cola.rgb * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - col0.rgb) * (vec3(1.0, 1.0, 1.0) - cola.rgb)))), max(col0.a, cola.a));
	}
	else if (blending == 10) { // Linear Light
		FragColor[0] = vec4((cola.rgb + str * (vec3(2.0, 2.0, 2.0) * (col0.rgb - vec3(0.5, 0.5, 0.5)))), max(col0.a, cola.a));
	}
	else if (blending == 11) { // Difference
		FragColor[0] = vec4(mix(cola.rgb, abs(cola.rgb - col0.rgb), str), max(col0.a, cola.a));
	}
	else if (blending == 12) { // Subtract
		FragColor[0] = vec4(mix(cola.rgb, cola.rgb - col0.rgb, str), max(col0.a, cola.a));
	}
	else if (blending == 13) { // Divide
		FragColor[0] = vec4(vec3(1.0 - str, 1.0 - str, 1.0 - str) * cola.rgb + vec3(str, str, str) * cola.rgb / col0.rgb, max(col0.a, cola.a));
	}
	else if (blending == 14) { // Hue
		FragColor[0] = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 15) { // Saturation
		FragColor[0] = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else if (blending == 16) { // Color
		FragColor[0] = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(col0.rgb).r, rgb_to_hsv(col0.rgb).g, rgb_to_hsv(cola.rgb).b)), str), max(col0.a, cola.a));
	}
	else { // Value
		FragColor[0] = vec4(mix(cola.rgb, hsv_to_rgb(vec3(rgb_to_hsv(cola.rgb).r, rgb_to_hsv(cola.rgb).g, rgb_to_hsv(col0.rgb).b)), str), max(col0.a, cola.a));
	}
	FragColor[1] = vec4(mix(colb, col1, str));
	FragColor[2] = vec4(mix(colc, col2, str));
}
