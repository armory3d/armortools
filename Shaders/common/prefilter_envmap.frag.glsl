#version 450

in vec2 texCoord;

uniform sampler2D radiance;
uniform vec4 params;

out vec4 fragColor;

const float PI = 3.14159265358979;
const float PI2 = PI * 2.0;
const int samples = 32;

float rand(const vec2 co) {
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}

vec2 equirect(vec3 normal) {
	float phi = acos(normal.z);
	float theta = atan(-normal.y, normal.x) + PI;
	return vec2(theta / PI2, phi / PI);
}

vec3 reverseEquirect(vec2 co) {
	float theta = co.x * PI2;
	float phi = co.y * PI;
	return vec3(cos(theta), -sin(theta), cos(phi));
}

vec3 cos_weighted_hemisphere_direction(vec3 n, vec2 co, uint seed) {
	vec2 r = vec2(rand(co * seed), rand(co * 2 * seed));
	vec3 uu = normalize(cross(n, vec3(0.0, 1.0, 1.0)));
	vec3 vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra * cos(6.2831 * r.x);
	float ry = ra * sin(6.2831 * r.x);
	float rz = sqrt(1.0 - r.y);
	vec3 rr = vec3(rx * uu + ry * vv + rz * n);
	return normalize(rr);
}

void main() {
	fragColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 n = reverseEquirect(texCoord);
	for (int i = 0; i < samples; i++) {
		vec3 dir = mix(n, cos_weighted_hemisphere_direction(n, texCoord, i), params.x / 32.0);
		fragColor.rgb += texture(radiance, equirect(dir)).rgb;
	}
	fragColor.rgb /= float(samples);
}
