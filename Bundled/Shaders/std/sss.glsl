
// Separable SSS Transmittance Function, ref to sss_pass
vec3 SSSSTransmittance(mat4 LWVP, vec3 p, vec3 n, vec3 l, float lightFar, sampler2DShadow shadowMap) {
	const float translucency = 1.0;
	vec4 shrinkedPos = vec4(p - 0.005 * n, 1.0);
	vec4 shadowPos = LWVP * shrinkedPos;
	float scale = 8.25 * (1.0 - translucency) / (sssWidth / 10.0);
	float d1 = texture(shadowMap, vec3(shadowPos.xy / shadowPos.w, shadowPos.z)).r; // 'd1' has a range of 0..1
	float d2 = shadowPos.z; // 'd2' has a range of 0..'lightFarPlane'
	d1 *= lightFar; // So we scale 'd1' accordingly:
	float d = scale * abs(d1 - d2);

	float dd = -d * d;
	vec3 profile = vec3(0.233, 0.455, 0.649) * exp(dd / 0.0064) +
				   vec3(0.1,   0.336, 0.344) * exp(dd / 0.0484) +
				   vec3(0.118, 0.198, 0.0)   * exp(dd / 0.187) +
				   vec3(0.113, 0.007, 0.007) * exp(dd / 0.567) +
				   vec3(0.358, 0.004, 0.0)   * exp(dd / 1.99) +
				   vec3(0.078, 0.0,   0.0)   * exp(dd / 7.41);
	return profile * clamp(0.3 + dot(l, -n), 0.0, 1.0);
}

vec3 SSSSTransmittanceCube(float translucency, vec4 shadowPos, vec3 n, vec3 l, float lightFar) {
	// TODO
	return vec3(0.0);
}
