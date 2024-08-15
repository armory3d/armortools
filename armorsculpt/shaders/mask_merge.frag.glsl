#version 450

uniform sampler2D tex0;
uniform sampler2D texa;
uniform float opac;
uniform int blending;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	float col0 = textureLod(tex0, tex_coord, 0).r;
	float cola = textureLod(texa, tex_coord, 0).r;
	float str = opac;
	float out_color = 0.0;
	if (blending == 0) { // Mix
		out_color = mix(cola, col0, str);
	}
	else if (blending == 1) { // Darken
		out_color = mix(cola, min(cola, col0), str);
	}
	else if (blending == 2) { // Multiply
		out_color = mix(cola, cola * col0, str);
	}
	else if (blending == 3) { // Burn
		out_color = mix(cola, 1.0 - (1.0 - cola) / col0, str);
	}
	else if (blending == 4) { // Lighten
		out_color = max(cola, col0 * str);
	}
	else if (blending == 5) { // Screen
		out_color = (1.0 - ((1.0 - str) + str * (1.0 - col0)) * (1.0 - cola));
	}
	else if (blending == 6) { // Dodge
		out_color = mix(cola, cola / (1.0 - col0), str);
	}
	else if (blending == 7) { // Add
		out_color = mix(cola, cola + col0, str);
	}
	else if (blending == 8) { // Overlay
		out_color = mix(cola, cola < 0.5 ? 2.0 * cola * col0 : 1.0 - 2.0 * (1.0 - cola) * (1.0 - col0), str);
	}
	else if (blending == 9) { // Soft Light
		out_color = ((1.0 - str) * cola + str * ((1.0 - cola) * col0 * cola + cola * (1.0 - (1.0 - col0) * (1.0 - cola))));
	}
	else if (blending == 10) { // Linear Light
		out_color = (cola + str * (2.0 * (col0 - 0.5)));
	}
	else if (blending == 11) { // Difference
		out_color = mix(cola, abs(cola - col0), str);
	}
	else if (blending == 12) { // Subtract
		out_color = mix(cola, cola - col0, str);
	}
	else if (blending == 13) { // Divide
		out_color = (1.0 - str) * cola + str * cola / col0;
	}
	else { // Hue, Saturation, Color, Value
		out_color = mix(cola, col0, str);
	}
	frag_color = vec4(out_color, out_color, out_color, 1.0);
}
