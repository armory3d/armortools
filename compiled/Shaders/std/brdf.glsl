#ifndef _BRDF_GLSL_
#define _BRDF_GLSL_

// http://xlgames-inc.github.io/posts/improvedibl/
// http://blog.selfshadow.com/publications/s2013-shading-course/
vec3 f_schlick(const vec3 f0, const float vh) {
	return f0 + (1.0 - f0) * exp2((-5.55473 * vh - 6.98316) * vh);
}

float v_smithschlick(const float nl, const float nv, const float a) {
	return 1.0 / ((nl * (1.0 - a) + a) * (nv * (1.0 - a) + a));
}

float d_ggx(const float nh, const float a) {
	float a2 = a * a;
	float denom = pow(nh * nh * (a2 - 1.0) + 1.0, 2.0);
	return a2 * (1.0 / 3.1415926535) / denom;
}

vec3 specularBRDF(const vec3 f0, const float roughness, const float nl, const float nh, const float nv, const float vh) {
	float a = roughness * roughness;
	return d_ggx(nh, a) * clamp(v_smithschlick(nl, nv, a), 0.0, 1.0) * f_schlick(f0, vh) / 4.0;
}

// John Hable - Optimizing GGX Shaders
// http://filmicworlds.com/blog/optimizing-ggx-shaders-with-dotlh/
vec3 specularBRDFb(const vec3 f0, const float roughness, const float dotNL, const float dotNH, const float dotLH) {
	// D
	const float pi = 3.1415926535;
	float alpha = roughness * roughness;
	float alphaSqr = alpha * alpha;
	float denom = dotNH * dotNH * (alphaSqr - 1.0) + 1.0;
	float D = alphaSqr / (pi * denom * denom);
	// F
	const float F_a = 1.0;
	float F_b = pow(1.0 - dotLH, 5.0);
	// V
	float vis;
	float k = alpha / 2.0;
	float k2 = k * k;
	float invK2 = 1.0 - k2;
	vis = 1.0 / (dotLH * dotLH * invK2 + k2);
	vec2 FV_helper = vec2((F_a - F_b) * vis, F_b * vis);

	vec3 FV = f0 * FV_helper.x + FV_helper.y;
	vec3 specular = clamp(dotNL, 0.0, 1.0) * D * FV;
	return specular / 4.0; // TODO: get rid of / 4.0
}

vec3 orenNayarDiffuseBRDF(const vec3 albedo, const float roughness, const float nv, const float nl, const float vh) {
	float a = roughness * roughness;
	float s = a;
	float s2 = s * s;
	float vl = 2.0 * vh * vh - 1.0; // Double angle identity
	float Cosri = vl - nv * nl;
	float C1 = 1.0 - 0.5 * s2 / (s2 + 0.33);
	float test = 1.0;
	if (Cosri >= 0.0) test = (1.0 / (max(nl, nv)));
	float C2 = 0.45 * s2 / (s2 + 0.09) * Cosri * test;
	return albedo * max(0.0, nl) * (C1 + C2) * (1.0 + roughness * 0.5);
}

vec3 lambertDiffuseBRDF(const vec3 albedo, const float nl) {
	return albedo * max(0.0, nl);
}

vec3 surfaceAlbedo(const vec3 baseColor, const float metalness) {
	return mix(baseColor, vec3(0.0), metalness);
}

vec3 surfaceF0(const vec3 baseColor, const float metalness) {
	return mix(vec3(0.04), baseColor, metalness);
}

float getMipFromRoughness(const float roughness, const float numMipmaps) {
	// First mipmap level = roughness 0, last = roughness = 1
	return roughness * numMipmaps;
}

float wardSpecular(vec3 N, vec3 H, float dotNL, float dotNV, float dotNH, vec3 fiberDirection, float shinyParallel, float shinyPerpendicular) {
	if(dotNL < 0.0 || dotNV < 0.0) {
		return 0.0;
	}
	// fiberDirection - parse from rotation
	// shinyParallel - roughness
	// shinyPerpendicular - anisotropy
	
	vec3 fiberParallel = normalize(fiberDirection);
	vec3 fiberPerpendicular = normalize(cross(N, fiberDirection));
	float dotXH = dot(fiberParallel, H);
	float dotYH = dot(fiberPerpendicular, H);
	const float PI = 3.1415926535;
	float coeff = sqrt(dotNL/dotNV) / (4.0 * PI * shinyParallel * shinyPerpendicular); 
	float theta = (pow(dotXH/shinyParallel, 2.0) + pow(dotYH/shinyPerpendicular, 2.0)) / (1.0 + dotNH);
	return clamp(coeff * exp(-2.0 * theta), 0.0, 1.0);
}

// https://www.unrealengine.com/en-US/blog/physically-based-shading-on-mobile
// vec3 EnvBRDFApprox(vec3 SpecularColor, float Roughness, float NoV) {
// 	const vec4 c0 = { -1, -0.0275, -0.572, 0.022 };
// 	const vec4 c1 = { 1, 0.0425, 1.04, -0.04 };
// 	vec4 r = Roughness * c0 + c1;
// 	float a004 = min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
// 	vec2 AB = vec2( -1.04, 1.04 ) * a004 + r.zw;
// 	return SpecularColor * AB.x + AB.y;
// }
// float EnvBRDFApproxNonmetal(float Roughness, float NoV) {
// 	// Same as EnvBRDFApprox( 0.04, Roughness, NoV )
// 	const vec2 c0 = { -1, -0.0275 };
// 	const vec2 c1 = { 1, 0.0425 };
// 	vec2 r = Roughness * c0 + c1;
// 	return min( r.x * r.x, exp2( -9.28 * NoV ) ) * r.x + r.y;
// }
float D_Approx(const float Roughness, const float RoL) {
	float a = Roughness * Roughness;
	float a2 = a * a;
	float rcp_a2 = 1.0 / a2;//rcp(a2);
	// 0.5 / ln(2), 0.275 / ln(2)
	float c = 0.72134752 * rcp_a2 + 0.39674113;
	return rcp_a2 * exp2( c * RoL - c );
}

#endif
