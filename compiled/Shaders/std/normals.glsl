// http://www.thetenthplanet.de/archives/1180
mat3 cotangentFrame(const vec3 n, const vec3 p, const vec2 duv1, const vec2 duv2) {
	// Get edge vectors of the pixel triangle
	vec3 dp1 = dFdx(p);
	vec3 dp2 = dFdy(p);
 
	// Solve the linear system
	vec3 dp2perp = cross(dp2, n);
	vec3 dp1perp = cross(n, dp1);
	vec3 t = dp2perp * duv1.x + dp1perp * duv2.x;
	vec3 b = dp2perp * duv1.y + dp1perp * duv2.y;
 
	// Construct a scale-invariant frame 
	float invmax = inversesqrt(max(dot(t, t), dot(b, b)));
	return mat3(t * invmax, b * invmax, n);
}

mat3 cotangentFrame(const vec3 n, const vec3 p, const vec2 texCoord) {
	return cotangentFrame(n, p, dFdx(texCoord), dFdy(texCoord));
}

// vec3 perturbNormal(vec3 n, vec3 v, vec2 texCoord) {
	// Assume N, the interpolated vertex normal and V, the view vector (vertex to eye)
	// vec3 map = texture(snormal, texCoord).xyz * (255.0 / 127.0) - (128.0 / 127.0);
// WITH_NORMALMAP_2CHANNEL
	// map.z = sqrt(1.0 - dot(map.xy, map.xy));
// WITH_NORMALMAP_GREEN_UP
	// map.y = -map.y;
	// mat3 TBN = cotangentFrame(n, -v, texCoord);
	// return normalize(TBN * map);
// }
