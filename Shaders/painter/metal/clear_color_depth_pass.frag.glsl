#version 330
in vec2 texCoord;
out vec4 fragColor;
void main() {
	fragColor = vec4(0.0, 0.0, 0.0, 1.0);
	gl_FragDepth = 1.0;
}
