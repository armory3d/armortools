#version 450

uniform mat4 P;

in vec3 pos;
in vec2 tex;
in vec4 col;
out vec2 tex_coord;
out vec4 color;

void main() {
	gl_Position = P * vec4(pos, 1.0);
	tex_coord = tex;
	color = col;
}
