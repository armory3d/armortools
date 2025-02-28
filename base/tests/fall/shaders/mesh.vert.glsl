#version 450

in vec4 pos;
in vec2 nor;
in vec2 tex;

out vec3 normal;
out vec2 tex_coord;

uniform mat4 WVP;

void main() {
	normal = vec3(nor.xy, pos.w);
	tex_coord = tex;
	gl_Position = WVP * vec4(pos.xyz, 1.0);
}
