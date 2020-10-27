#version 330
in vec2 texCoord;
out vec4 fragColor;
uniform vec4 clearColor;
void main() {
	fragColor = clearColor;
	gl_FragDepth = 1.0;
}
