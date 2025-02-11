
const float PI = 3.1415926535;
const float PI2 = PI * 2.0;

vec2 envmap_equirect(const vec3 normal, const float angle) {
	const float PI = 3.1415926535;
	const float PI2 = PI * 2.0;
	float phi = acos(normal.z);
	float theta = atan2(normal.x, -normal.y) + PI + angle;
	return vec2(theta / PI2, phi / PI);
}

float rand(const vec2 co) { // Unreliable
	return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}
