#version 450

uniform vec3 tint;

in vec2 tex_coord;
out vec4 frag_color;

void main() {
	float radius = 0.45;
	float thickness = 0.03;
	float dist = distance(tex_coord, vec2(0.5, 0.5));
	float ring = smoothstep(radius - thickness, radius, dist) -
				 smoothstep(radius, radius + thickness, dist);
	frag_color = vec4(tint, min(ring, 0.6));
}
