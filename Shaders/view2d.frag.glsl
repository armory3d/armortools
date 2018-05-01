#version 450

uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;


void main() {
	// Show basecolor
	vec4 texcolor = texture(tex, texCoord) * color;
	// texcolor.rgb *= color.a;
	FragColor = vec4(texcolor.rgb, 1.0);
}
