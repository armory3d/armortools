#version 450

#include "../std/filters.glsl"

uniform sampler2D tex;
uniform vec2 screenSizeInv;

in vec2 texCoord;
out vec4 fragColor;

void main() {
	// 4X resolve
	fragColor = textureSS(tex, texCoord, screenSizeInv / 4.0);
}
