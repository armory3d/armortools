#version 450

#define _Veloc

uniform sampler2D color_tex;
uniform sampler2D blend_tex;
#ifdef _Veloc
uniform sampler2D sveloc;
#endif
uniform vec2 screen_size_inv;

in vec2 tex_coord;
in vec4 offset;
out vec4 frag_color;

//-----------------------------------------------------------------------------
// Neighborhood Blending Pixel Shader (Third Pass)

vec4 textureLodA(sampler2D tex, vec2 coords, float lod) {
	#if defined(HLSL) || defined(METAL) || defined(SPIRV)
	coords.y = 1.0 - coords.y;
	#endif
	return textureLod(tex, coords, lod);
}

vec4 smaa_neighborhood_blending_ps(vec2 texcoord, vec4 offset) {
	// Fetch the blending weights for current pixel:
	vec4 a;
	a.x = textureLod(blend_tex, offset.xy, 0.0).a; // Right
	a.y = textureLod(blend_tex, offset.zw, 0.0).g; // Top
	a.wz = textureLod(blend_tex, texcoord, 0.0).xz; // Bottom / Left

	// Is there any blending weight with a value greater than 0.0?
	//SMAA_BRANCH
	if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1e-5) {
		vec4 color = textureLod(color_tex, texcoord, 0.0);

#ifdef _Veloc
		vec2 velocity = textureLod(sveloc, tex_coord, 0.0).rg;
		// Pack velocity into the alpha channel:
		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	else {
		bool h = max(a.x, a.z) > max(a.y, a.w); // max(horizontal) > max(vertical)

		// Calculate the blending offsets:
		vec4 blending_offset = vec4(0.0, a.y, 0.0, a.w);
		vec2 blending_weight = a.yw;

		if (h) {
			blending_offset.x = a.x;
			blending_offset.y = 0.0;
			blending_offset.z = a.z;
			blending_offset.w = 0.0;
			blending_weight.x = a.x;
			blending_weight.y = a.z;
		}

		blending_weight /= dot(blending_weight, vec2(1.0, 1.0));

		// Calculate the texture coordinates:
		#if defined(HLSL) || defined(METAL) || defined(SPIRV)
		vec2 tc = vec2(texcoord.x, 1.0 - texcoord.y);
		#else
		vec2 tc = texcoord;
		#endif
		vec4 blending_coord = blending_offset * vec4(screen_size_inv.xy, -screen_size_inv.xy) + tc.xyxy;

		// We exploit bilinear filtering to mix current pixel with the chosen
		// neighbor:
		vec4 color = blending_weight.x * textureLodA(color_tex, blending_coord.xy, 0.0);
		color += blending_weight.y * textureLodA(color_tex, blending_coord.zw, 0.0);

#ifdef _Veloc
		// Antialias velocity for proper reprojection in a later stage:
		vec2 velocity = blending_weight.x * textureLodA(sveloc, blending_coord.xy, 0.0).rg;
		velocity += blending_weight.y * textureLodA(sveloc, blending_coord.zw, 0.0).rg;

		// Pack velocity into the alpha channel:
		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	return vec4(0.0);
}

void main() {
	frag_color = smaa_neighborhood_blending_ps(tex_coord, offset);
}
