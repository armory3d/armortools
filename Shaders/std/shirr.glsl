
vec3 shIrradiance(const vec3 nor, const vec4 shirr[7]) {
	const float c1 = 0.429043;
	const float c2 = 0.511664;
	const float c3 = 0.743125;
	const float c4 = 0.886227;
	const float c5 = 0.247708;
	// TODO: Use padding for 4th component and pass shirr[].xyz directly
	vec3 cl00 = vec3(shirr[0].x, shirr[0].y, shirr[0].z);
	vec3 cl1m1 = vec3(shirr[0].w, shirr[1].x, shirr[1].y);
	vec3 cl10 = vec3(shirr[1].z, shirr[1].w, shirr[2].x);
	vec3 cl11 = vec3(shirr[2].y, shirr[2].z, shirr[2].w);
	vec3 cl2m2 = vec3(shirr[3].x, shirr[3].y, shirr[3].z);
	vec3 cl2m1 = vec3(shirr[3].w, shirr[4].x, shirr[4].y);
	vec3 cl20 = vec3(shirr[4].z, shirr[4].w, shirr[5].x);
	vec3 cl21 = vec3(shirr[5].y, shirr[5].z, shirr[5].w);
	vec3 cl22 = vec3(shirr[6].x, shirr[6].y, shirr[6].z);
	return (
		c1 * cl22 * (nor.y * nor.y - (-nor.z) * (-nor.z)) +
		c3 * cl20 * nor.x * nor.x +
		c4 * cl00 -
		c5 * cl20 +
		2.0 * c1 * cl2m2 * nor.y * (-nor.z) +
		2.0 * c1 * cl21  * nor.y * nor.x +
		2.0 * c1 * cl2m1 * (-nor.z) * nor.x +
		2.0 * c2 * cl11  * nor.y +
		2.0 * c2 * cl1m1 * (-nor.z) +
		2.0 * c2 * cl10  * nor.x
	);
}
