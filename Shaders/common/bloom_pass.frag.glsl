#version 450

const float bloomThreshold = 1.5;

uniform sampler2D tex;

in vec2 texCoord;
out vec4 fragColor;

void main() {
	vec3 col = textureLod(tex, texCoord, 0.0).rgb;
	float brightness = dot(col, vec3(0.2126, 0.7152, 0.0722));
	if (brightness > bloomThreshold) {
		fragColor.rgb = col;
	}
	else {
		fragColor.rgb = vec3(0.0);
	}
}
