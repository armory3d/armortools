#version 330
uniform sampler2D tex;
uniform int channel;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	if (channel == 1) {
		FragColor = textureLod(tex, texCoord, 0).rrra * color;
	}
	else if (channel == 2) {
		FragColor = textureLod(tex, texCoord, 0).ggga * color;
	}
	else if (channel == 3) {
		FragColor = textureLod(tex, texCoord, 0).bbba * color;
	}
	else if (channel == 4) {
		FragColor = textureLod(tex, texCoord, 0).aaaa * color;
	}
	else {
		FragColor = textureLod(tex, texCoord, 0).rgba * color;
	}
}
