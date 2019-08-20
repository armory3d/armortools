#version 450

#include "compiled.inc"

uniform sampler2D colorTex;
uniform sampler2D blendTex;
#ifdef _Veloc
uniform sampler2D sveloc;
#endif

uniform vec2 screenSizeInv;

in vec2 texCoord;
in vec4 offset;
out vec4 fragColor;

//-----------------------------------------------------------------------------
// Neighborhood Blending Pixel Shader (Third Pass)

vec4 textureLodA(sampler2D tex, vec2 coords, float lod) {
	#ifdef HLSL
	coords.y = 1.0 - coords.y;
	#endif
	return textureLod(tex, coords, lod);
}

vec4 SMAANeighborhoodBlendingPS(vec2 texcoord, vec4 offset) {
	// Fetch the blending weights for current pixel:
	vec4 a;
	a.x = textureLod(blendTex, offset.xy, 0.0).a; // Right
	a.y = textureLod(blendTex, offset.zw, 0.0).g; // Top
	a.wz = textureLod(blendTex, texcoord, 0.0).xz; // Bottom / Left

	// Is there any blending weight with a value greater than 0.0?
	//SMAA_BRANCH
	if (dot(a, vec4(1.0, 1.0, 1.0, 1.0)) < 1e-5) {
		vec4 color = textureLod(colorTex, texcoord, 0.0);

#ifdef _Veloc
		vec2 velocity = textureLod(sveloc, texCoord, 0.0).rg;
		// Pack velocity into the alpha channel:
		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	else {
		bool h = max(a.x, a.z) > max(a.y, a.w); // max(horizontal) > max(vertical)

		// Calculate the blending offsets:
		vec4 blendingOffset = vec4(0.0, a.y, 0.0, a.w);
		vec2 blendingWeight = a.yw;
		
		if (h) {
			blendingOffset.x = a.x;
			blendingOffset.y = 0.0;
			blendingOffset.z = a.z;
			blendingOffset.w = 0.0;
			blendingWeight.x = a.x;
			blendingWeight.y = a.z;
		}
		
		blendingWeight /= dot(blendingWeight, vec2(1.0, 1.0));

		// Calculate the texture coordinates:
		#ifdef HLSL
		vec2 tc = vec2(texcoord.x, 1.0 - texcoord.y);
		#else
		vec2 tc = texcoord;
		#endif
		vec4 blendingCoord = blendingOffset * vec4(screenSizeInv.xy, -screenSizeInv.xy) + tc.xyxy;

		// We exploit bilinear filtering to mix current pixel with the chosen
		// neighbor:
		vec4 color = blendingWeight.x * textureLodA(colorTex, blendingCoord.xy, 0.0);
		color += blendingWeight.y * textureLodA(colorTex, blendingCoord.zw, 0.0);

#ifdef _Veloc
		// Antialias velocity for proper reprojection in a later stage:
		vec2 velocity = blendingWeight.x * textureLodA(sveloc, blendingCoord.xy, 0.0).rg;
		velocity += blendingWeight.y * textureLodA(sveloc, blendingCoord.zw, 0.0).rg;

		// Pack velocity into the alpha channel:
		color.a = sqrt(5.0 * length(velocity));
#endif
		return color;
	}
	return vec4(0.0);
}

void main() {
	fragColor = SMAANeighborhoodBlendingPS(texCoord, offset);
}
