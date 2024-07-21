#version 450

uniform sampler2D tex0;
uniform sampler2D texa;
uniform float opac;
uniform int blending;

in vec2 texCoord;
out vec4 FragColor;

void main() {
	float col0 = textureLod(tex0, texCoord, 0).r;
	float cola = textureLod(texa, texCoord, 0).r;
	float str = opac;
	float outColor = 0.0;
	if (blending == 0) { // Mix
		outColor = mix(cola, col0, str);
	}
	else if (blending == 1) { // Darken
		outColor = mix(cola, min(cola, col0), str);
	}
	else if (blending == 2) { // Multiply
		outColor = mix(cola, cola * col0, str);
	}
	else if (blending == 3) { // Burn
		outColor = mix(cola, 1.0 - (1.0 - cola) / col0, str);
	}
	else if (blending == 4) { // Lighten
		outColor = max(cola, col0 * str);
	}
	else if (blending == 5) { // Screen
		outColor = (1.0 - ((1.0 - str) + str * (1.0 - col0)) * (1.0 - cola));
	}
	else if (blending == 6) { // Dodge
		outColor = mix(cola, cola / (1.0 - col0), str);
	}
	else if (blending == 7) { // Add
		outColor = mix(cola, cola + col0, str);
	}
	else if (blending == 8) { // Overlay
		outColor = mix(cola, cola < 0.5 ? 2.0 * cola * col0 : 1.0 - 2.0 * (1.0 - cola) * (1.0 - col0), str);
	}
	else if (blending == 9) { // Soft Light
		outColor = ((1.0 - str) * cola + str * ((1.0 - cola) * col0 * cola + cola * (1.0 - (1.0 - col0) * (1.0 - cola))));
	}
	else if (blending == 10) { // Linear Light
		outColor = (cola + str * (2.0 * (col0 - 0.5)));
	}
	else if (blending == 11) { // Difference
		outColor = mix(cola, abs(cola - col0), str);
	}
	else if (blending == 12) { // Subtract
		outColor = mix(cola, cola - col0, str);
	}
	else if (blending == 13) { // Divide
		outColor = (1.0 - str) * cola + str * cola / col0;
	}
	else { // Hue, Saturation, Color, Value
		outColor = mix(cola, col0, str);
	}
	FragColor = vec4(outColor, outColor, outColor, 1.0);
}
