#version 450

uniform vec2 screen_size;
uniform vec2 screen_size_inv;

in vec2 pos;
out vec2 tex_coord;
out vec2 pixcoord;
out vec4 offset0;
out vec4 offset1;
out vec4 offset2;

const int SMAA_MAX_SEARCH_STEPS = 16;

void main() {
	const vec2 madd = vec2(0.5, 0.5);
	tex_coord = pos.xy * madd + madd;

	pixcoord = tex_coord * screen_size;

	offset0 = screen_size_inv.xyxy * vec4(-0.25, -0.125,  1.25, -0.125) + tex_coord.xyxy;
	offset1 = screen_size_inv.xyxy * vec4(-0.125, -0.25, -0.125,  1.25) + tex_coord.xyxy;

	offset2 = screen_size_inv.xxyy *
				(vec4(-2.0, 2.0, -2.0, 2.0) * float(SMAA_MAX_SEARCH_STEPS)) +
				 vec4(offset0.xz, offset1.yw);

	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
