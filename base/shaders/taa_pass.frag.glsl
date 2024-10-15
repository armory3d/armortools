#version 450

uniform sampler2D tex;
uniform sampler2D tex2;
uniform sampler2D sveloc;

in vec2 tex_coord;
out vec4 frag_color;

const float SMAA_REPROJECTION_WEIGHT_SCALE = 30.0;

void main() {
	vec4 current = textureLod(tex, tex_coord, 0.0);

	// Velocity is assumed to be calculated for motion blur, so we need to inverse it for reprojection
	vec2 velocity = -textureLod(sveloc, tex_coord, 0.0).rg;

#ifdef GLSL
#else
	velocity.y = -velocity.y;
#endif

	// Reproject current coordinates and fetch previous pixel
	vec4 previous = textureLod(tex2, tex_coord + velocity, 0.0);

	// Attenuate the previous pixel if the velocity is different
	float delta = abs(current.a * current.a - previous.a * previous.a) / 5.0;
	float weight = 0.5 * clamp(1.0 - sqrt(delta) * SMAA_REPROJECTION_WEIGHT_SCALE, 0.0, 1.0);

	// Blend the pixels according to the calculated weight:
	frag_color = vec4(mix(current.rgb, previous.rgb, weight), 1.0);
}
