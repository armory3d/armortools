#version 450

#define _Veloc
#define _SMAA

uniform sampler2D tex;
uniform sampler2D tex2;
#ifdef _Veloc
uniform sampler2D sveloc;
#endif

in vec2 texCoord;
out vec4 fragColor;

const float SMAA_REPROJECTION_WEIGHT_SCALE = 30.0;

void main() {
	vec4 current = textureLod(tex, texCoord, 0.0);

#ifdef _Veloc
	// Velocity is assumed to be calculated for motion blur, so we need to inverse it for reprojection
	vec2 velocity = -textureLod(sveloc, texCoord, 0.0).rg;

	#ifdef HLSL
	velocity.y = -velocity.y;
	#endif

	// Reproject current coordinates and fetch previous pixel
	vec4 previous = textureLod(tex2, texCoord + velocity, 0.0);

	// Attenuate the previous pixel if the velocity is different
	#ifdef _SMAA
		float delta = abs(current.a * current.a - previous.a * previous.a) / 5.0;
	#else
		const float delta = 0.0;
	#endif
	float weight = 0.5 * clamp(1.0 - sqrt(delta) * SMAA_REPROJECTION_WEIGHT_SCALE, 0.0, 1.0);

	// Blend the pixels according to the calculated weight:
	fragColor = vec4(mix(current.rgb, previous.rgb, weight), 1.0);
#else
	vec4 previous = textureLod(tex2, texCoord, 0.0);
	fragColor = vec4(mix(current.rgb, previous.rgb, 0.5), 1.0);
#endif
}
