#version 450

uniform mat4 VP;
uniform mat4 invVP;
uniform vec2 mouse;
uniform vec2 tex_step;
uniform float radius;
uniform vec3 camera_right;
uniform sampler2D gbufferD;
#ifdef HLSL
uniform sampler2D texa; // direct3d12 unit align
#endif

in vec4 pos;
in vec2 nor;
in vec2 tex;
out vec2 tex_coord;

vec3 get_pos(vec2 uv) {
	#ifdef HLSL
	float keep = textureLod(texa, vec2(0.0, 0.0), 0.0).r; // direct3d12 unit align
	float keep2 = pos.x + nor.x;
	#endif
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	float depth = textureLod(gbufferD, vec2(uv.x, 1.0 - uv.y), 0.0).r;
	#else
	float depth = textureLod(gbufferD, uv, 0.0).r;
	#endif
	vec4 wpos = vec4(uv * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);
	wpos = invVP * wpos;
	return wpos.xyz / wpos.w;
}

vec3 get_normal(vec3 p0, vec2 uv) {
	vec3 p1 = get_pos(uv + vec2(tex_step.x * 4, 0));
	vec3 p2 = get_pos(uv + vec2(0, tex_step.y * 4));
	return normalize(cross(p2 - p0, p1 - p0));
}

void create_basis(vec3 normal, out vec3 tangent, out vec3 binormal) {
	tangent = normalize(camera_right - normal * dot(camera_right, normal));
	binormal = cross(tangent, normal);
}

void main() {
	tex_coord = tex;
	vec3 wpos = get_pos(mouse);
	vec2 uv1 = mouse + tex_step * 4;
	vec2 uv2 = mouse - tex_step * 4;
	vec3 wpos1 = get_pos(uv1);
	vec3 wpos2 = get_pos(uv2);
	vec3 n = normalize(
		get_normal(wpos, mouse) +
		get_normal(wpos1, uv1) +
		get_normal(wpos2, uv2)
	);
	vec3 n_tan;
	vec3 n_bin;
	create_basis(n, n_tan, n_bin);
	if      (gl_VertexID == 0) wpos += normalize(-n_tan - n_bin) * 0.7 * radius;
	else if (gl_VertexID == 1) wpos += normalize( n_tan - n_bin) * 0.7 * radius;
	else if (gl_VertexID == 2) wpos += normalize( n_tan + n_bin) * 0.7 * radius;
	else if (gl_VertexID == 3) wpos += normalize(-n_tan + n_bin) * 0.7 * radius;
	gl_Position = VP * vec4(wpos, 1.0);
}
