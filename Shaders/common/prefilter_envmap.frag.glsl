#version 450

in vec2 texCoord;

uniform sampler2D radiance;
uniform vec4 params;

out vec4 fragColor;

const float PI = 3.14159265358979;
const float PI2 = PI * 2.0;
const int samples = 1024 * 16;

float rand(vec2 co) {
	return fract(sin(mod(dot(co.xy, vec2(12.9898, 78.233)), 3.14)) * 43758.5453);
}

vec2 equirect(vec3 normal) {
	float phi = acos(normal.z);
	float theta = atan(-normal.y, normal.x) + PI;
	return vec2(theta / PI2, phi / PI);
}

vec3 reverseEquirect(vec2 co) {
	float theta = co.x * PI2 - PI;
	float phi = co.y * PI;
	return vec3(sin(phi) * cos(theta), -(sin(phi) * sin(theta)), cos(phi));
}

vec3 cos_weighted_hemisphere_direction(vec3 n, vec2 co, uint seed) {
	vec2 r = vec2(rand(co * seed), rand(co * seed * 2));
	vec3 uu = normalize(cross(n, vec3(0.0, 1.0, 1.0)));
	vec3 vv = cross(uu, n);
	float ra = sqrt(r.y);
	float rx = ra * cos(PI2 * r.x);
	float ry = ra * sin(PI2 * r.x);
	float rz = sqrt(1.0 - r.y);
	vec3 rr = vec3(rx * uu + ry * vv + rz * n);
	return normalize(rr);
}

void main() {
	vec4 paramsLocal = params; // TODO: spirv workaround
	fragColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 n = reverseEquirect(texCoord);
	for (int i = 0; i < samples; i++) {
		vec3 dir = normalize(mix(n, cos_weighted_hemisphere_direction(n, texCoord, i), paramsLocal.x));
		fragColor.rgb += texture(radiance, equirect(dir)).rgb;
	}
	fragColor.rgb /= float(samples);
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
}
