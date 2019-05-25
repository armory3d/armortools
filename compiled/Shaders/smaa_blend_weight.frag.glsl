#version 450

#define SMAA_MAX_SEARCH_STEPS_DIAG 8
#define SMAA_AREATEX_MAX_DISTANCE 16
#define SMAA_AREATEX_MAX_DISTANCE_DIAG 20
#define SMAA_AREATEX_PIXEL_SIZE (1.0 / vec2(160.0, 560.0))
#define SMAA_AREATEX_SUBTEX_SIZE (1.0 / 7.0)
#define SMAA_SEARCHTEX_SIZE vec2(66.0, 33.0)
#define SMAA_SEARCHTEX_PACKED_SIZE vec2(64.0, 16.0)
#define SMAA_CORNER_ROUNDING 25
#define SMAA_CORNER_ROUNDING_NORM (float(SMAA_CORNER_ROUNDING) / 100.0)
#define SMAA_AREATEX_SELECT(sample) sample.rg
#define SMAA_SEARCHTEX_SELECT(sample) sample.r
#define mad(a, b, c) (a * b + c)
#define saturate(a) clamp(a, 0.0, 1.0)
#define round(a) floor(a + 0.5)

uniform sampler2D edgesTex;
uniform sampler2D areaTex;
uniform sampler2D searchTex;

uniform vec2 screenSize;
uniform vec2 screenSizeInv;

in vec2 texCoord;
in vec2 pixcoord;
in vec4 offset0;
in vec4 offset1;
in vec4 offset2;
out vec4 fragColor;

// Blending Weight Calculation Pixel Shader (Second Pass)
vec2 cdw_end;

vec4 textureLodA(sampler2D tex, vec2 coord, float lod) {
	#ifdef HLSL
	coord.y = 1.0 - coord.y;
	#endif
	return textureLod(tex, coord, lod);
}

#define SMAASampleLevelZeroOffset(tex, coord, offset) textureLodA(tex, coord + offset * screenSizeInv.xy, 0.0)

//-----------------------------------------------------------------------------
// Diagonal Search Functions

// #if !defined(SMAA_DISABLE_DIAG_DETECTION)
/**
 * Allows to decode two binary values from a bilinear-filtered access.
 */
vec2 SMAADecodeDiagBilinearAccess(vec2 e) {
	// Bilinear access for fetching 'e' have a 0.25 offset, and we are
	// interested in the R and G edges:
	//
	// +---G---+-------+
	// |   x o R   x   |
	// +-------+-------+
	//
	// Then, if one of these edge is enabled:
	//   Red:   (0.75 * X + 0.25 * 1) => 0.25 or 1.0
	//   Green: (0.75 * 1 + 0.25 * X) => 0.75 or 1.0
	//
	// This function will unpack the values (mad + mul + round):
	// wolframalpha.com: round(x * abs(5 * x - 5 * 0.75)) plot 0 to 1
	e.r = e.r * abs(5.0 * e.r - 5.0 * 0.75);
	return round(e);
}

vec4 SMAADecodeDiagBilinearAccess(vec4 e) {
	e.rb = e.rb * abs(5.0 * e.rb - 5.0 * 0.75);
	return round(e);
}

/**
 * These functions allows to perform diagonal pattern searches.
 */
vec2 SMAASearchDiag1(vec2 texcoord, vec2 dir/*, out vec2 e*/) {
	vec4 coord = vec4(texcoord, -1.0, 1.0);
	vec3 t = vec3(screenSizeInv.xy, 1.0);
	float cw = coord.w; // TODO: krafix hlsl bug
	while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && cw > 0.9) {
		coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);
		cdw_end /*e*/ = textureLodA(edgesTex, coord.xy, 0.0).rg;
		cw = dot(cdw_end /*e*/, vec2(0.5, 0.5));
	}
	coord.w = cw;
	return coord.zw;
}

vec2 SMAASearchDiag2(vec2 texcoord, vec2 dir) {
	vec4 coord = vec4(texcoord, -1.0, 1.0);
	coord.x += 0.25 * screenSizeInv.x; // See @SearchDiag2Optimization
	vec3 t = vec3(screenSizeInv.xy, 1.0);
	float cw = coord.w; // TODO: krafix hlsl bug
	while (coord.z < float(SMAA_MAX_SEARCH_STEPS_DIAG - 1) && cw > 0.9) {
		coord.xyz = mad(t, vec3(dir, 1.0), coord.xyz);
		// @SearchDiag2Optimization
		// Fetch both edges at once using bilinear filtering:
		cdw_end /*e*/ = textureLodA(edgesTex, coord.xy, 0.0).rg;
		cdw_end /*e*/ = SMAADecodeDiagBilinearAccess(cdw_end /*e*/);
		cw = dot(cdw_end /*e*/, vec2(0.5, 0.5));
	}
	coord.w = cw;
	return coord.zw;
}

/** 
 * Similar to SMAAArea, this calculates the area corresponding to a certain
 * diagonal distance and crossing edges 'e'.
 */
vec2 SMAAAreaDiag(vec2 dist, vec2 e, float offset) {
	vec2 texcoord = mad(vec2(SMAA_AREATEX_MAX_DISTANCE_DIAG, SMAA_AREATEX_MAX_DISTANCE_DIAG), e, dist);

	// We do a scale and bias for mapping to texel space:
	texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);

	// Diagonal areas are on the second half of the texture:
	texcoord.x += 0.5;

	// Move to proper place, according to the subpixel offset:
	texcoord.y += SMAA_AREATEX_SUBTEX_SIZE * offset;

	// Do it!
	return SMAA_AREATEX_SELECT(textureLod(areaTex, texcoord, 0.0));
}

/**
 * This searches for diagonal patterns and returns the corresponding weights.
 */
vec2 SMAACalculateDiagWeights(vec2 texcoord, vec2 e, vec4 subsampleIndices) {
	vec2 weights = vec2(0.0, 0.0);

	// Search for the line ends:
	vec4 d;
	if (e.r > 0.0) {
		d.xz = SMAASearchDiag1(texcoord, vec2(-1.0,  1.0)/*, cdw_end*/);
		float dadd = cdw_end.y > 0.9 ? 1.0 : 0.0;
		d.x += dadd;
	}
	else {
		d.xz = vec2(0.0, 0.0);
	}
	d.yw = SMAASearchDiag1(texcoord, vec2(1.0, -1.0)/*, cdw_end*/);

	//SMAA_BRANCH
	if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
		// Fetch the crossing edges:
		vec4 coords = mad(vec4(-d.x + 0.25, d.x, d.y, -d.y - 0.25), screenSizeInv.xyxy, texcoord.xyxy);
		vec4 c;
		
		c.xy = SMAASampleLevelZeroOffset(edgesTex, coords.xy, ivec2(-1,  0)).rg;
		c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, ivec2( 1,  0)).rg;
		c.yxwz = SMAADecodeDiagBilinearAccess(c.xyzw);

		// Merge crossing edges at each side into a single value:
		vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

		// Remove the crossing edge if we didn't found the end of the line:
		// SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0, 0.0));
		float a1condx = step(0.9, d.z);
		float a1condy = step(0.9, d.w);
		if (a1condx == 1.0) cc.x = 0.0;
		if (a1condy == 1.0) cc.y = 0.0;

		// Fetch the areas for this line:
		weights += SMAAAreaDiag(d.xy, cc, subsampleIndices.z);
	}

	// Search for the line ends:
	d.xz = SMAASearchDiag2(texcoord, vec2(-1.0, -1.0)/*, cdw_end*/);
	if (SMAASampleLevelZeroOffset(edgesTex, texcoord, ivec2(1, 0)).r > 0.0) {
		d.yw = SMAASearchDiag2(texcoord, vec2(1.0, 1.0)/*, cdw_end*/);
		float dadd = cdw_end.y > 0.9 ? 1.0 : 0.0;
		d.y += dadd;
	} 
	else {
		d.yw = vec2(0.0, 0.0);
	}

	// SMAA_BRANCH
	if (d.x + d.y > 2.0) { // d.x + d.y + 1 > 3
		// Fetch the crossing edges:
		vec4 coords = mad(vec4(-d.x, -d.x, d.y, d.y), screenSizeInv.xyxy, texcoord.xyxy);
		vec4 c;
		c.x  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, ivec2(-1,  0)).g;
		c.y  = SMAASampleLevelZeroOffset(edgesTex, coords.xy, ivec2( 0, -1)).r;
		c.zw = SMAASampleLevelZeroOffset(edgesTex, coords.zw, ivec2( 1,  0)).gr;
		vec2 cc = mad(vec2(2.0, 2.0), c.xz, c.yw);

		// Remove the crossing edge if we didn't found the end of the line:
		// SMAAMovc(bvec2(step(0.9, d.zw)), cc, vec2(0.0, 0.0));
		float a1condx = step(0.9, d.z);
		float a1condy = step(0.9, d.w);
		if (a1condx == 1.0) cc.x = 0.0;
		if (a1condy == 1.0) cc.y = 0.0;

		// Fetch the areas for this line:
		weights += SMAAAreaDiag(d.xy, cc, subsampleIndices.w).gr;
	}

	return weights;
}
// #endif

//-----------------------------------------------------------------------------
// Horizontal/Vertical Search Functions

/**
 * This allows to determine how much length should we add in the last step
 * of the searches. It takes the bilinearly interpolated edge (see 
 * @PSEUDO_GATHER4), and adds 0, 1 or 2, depending on which edges and
 * crossing edges are active.
 */
float SMAASearchLength(vec2 e, float offset) {
	// The texture is flipped vertically, with left and right cases taking half
	// of the space horizontally:
	vec2 scale = SMAA_SEARCHTEX_SIZE * vec2(0.5, -1.0);
	vec2 bias = SMAA_SEARCHTEX_SIZE * vec2(offset, 1.0);

	// Scale and bias to access texel centers:
	scale += vec2(-1.0, 1.0);
	bias += vec2( 0.5, -0.5);

	// Convert from pixel coordinates to texcoords:
	// (We use SMAA_SEARCHTEX_PACKED_SIZE because the texture is cropped)
	scale *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;
	bias *= 1.0 / SMAA_SEARCHTEX_PACKED_SIZE;

	vec2 coord = mad(scale, e, bias);

	// Lookup the search texture:
	return SMAA_SEARCHTEX_SELECT(textureLod(searchTex, coord, 0.0));
}

/**
 * Horizontal/vertical search functions for the 2nd pass.
 */
float SMAASearchXLeft(vec2 texcoord, float end) {
	/**
	 * @PSEUDO_GATHER4
	 * This texcoord has been offset by (-0.25, -0.125) in the vertex shader to
	 * sample between edge, thus fetching four edges in a row.
	 * Sampling with different offsets in each direction allows to disambiguate
	 * which edges are active from the four fetched ones.
	 */
	vec2 e = vec2(0.0, 1.0);
	while (texcoord.x > end && 
		   e.g > 0.8281 && // Is there some edge not activated?
		   e.r == 0.0) { // Or is there a crossing edge that breaks the line?
		e = textureLodA(edgesTex, texcoord, 0.0).rg;
		texcoord = mad(-vec2(2.0, 0.0), screenSizeInv.xy, texcoord);
	}

	float offset = mad(-(255.0 / 127.0), SMAASearchLength(e, 0.0), 3.25);
	return mad(screenSizeInv.x, offset, texcoord.x);
}

float SMAASearchXRight(vec2 texcoord, float end) {
	vec2 e = vec2(0.0, 1.0);
	while (texcoord.x < end && 
		   e.g > 0.8281 && // Is there some edge not activated?
		   e.r == 0.0) { // Or is there a crossing edge that breaks the line?
		e = textureLodA(edgesTex, texcoord, 0.0).rg;
		texcoord = mad(vec2(2.0, 0.0), screenSizeInv.xy, texcoord);
	}
	
	float offset = mad(-(255.0 / 127.0), SMAASearchLength(e, 0.5), 3.25);
	return mad(-screenSizeInv.x, offset, texcoord.x);
}

float SMAASearchYUp(vec2 texcoord, float end) {
	vec2 e = vec2(1.0, 0.0);
	while (texcoord.y > end && 
		   e.r > 0.8281 && // Is there some edge not activated?
		   e.g == 0.0) { // Or is there a crossing edge that breaks the line?
		e = textureLodA(edgesTex, texcoord, 0.0).rg;
		texcoord = mad(-vec2(0.0, 2.0), screenSizeInv.xy, texcoord);
	}
	float offset = mad(-(255.0 / 127.0), SMAASearchLength(e.gr, 0.0), 3.25);
	return mad(screenSizeInv.y, offset, texcoord.y);
}

float SMAASearchYDown(vec2 texcoord, float end) {
	vec2 e = vec2(1.0, 0.0);
	while (texcoord.y < end && 
		   e.r > 0.8281 && // Is there some edge not activated?
		   e.g == 0.0) { // Or is there a crossing edge that breaks the line?
		e = textureLodA(edgesTex, texcoord, 0.0).rg;
		texcoord = mad(vec2(0.0, 2.0), screenSizeInv.xy, texcoord);
	}
	float offset = mad(-(255.0 / 127.0), SMAASearchLength(/*searchTex,*/ e.gr, 0.5), 3.25);
	return mad(-screenSizeInv.y, offset, texcoord.y);
}

/** 
 * Ok, we have the distance and both crossing edges. So, what are the areas
 * at each side of current edge?
 */
vec2 SMAAArea(vec2 dist, float e1, float e2, float offset) {
	// Rounding prevents precision errors of bilinear filtering:
	vec2 texcoord = mad(vec2(SMAA_AREATEX_MAX_DISTANCE, SMAA_AREATEX_MAX_DISTANCE), round(4.0 * vec2(e1, e2)), dist);
	
	// We do a scale and bias for mapping to texel space:
	texcoord = mad(SMAA_AREATEX_PIXEL_SIZE, texcoord, 0.5 * SMAA_AREATEX_PIXEL_SIZE);

	// Move to proper place, according to the subpixel offset:
	texcoord.y = mad(SMAA_AREATEX_SUBTEX_SIZE, offset, texcoord.y);

	// Do it!
	return SMAA_AREATEX_SELECT(textureLod(areaTex, texcoord, 0.0));
}

//-----------------------------------------------------------------------------
// Corner Detection Functions

vec2 SMAADetectHorizontalCornerPattern(vec2 weights, vec4 texcoord, vec2 d) {
	// #if !defined(SMAA_DISABLE_CORNER_DETECTION)
	vec2 leftRight = step(d.xy, d.yx);
	vec2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * leftRight;

	rounding /= leftRight.x + leftRight.y; // Reduce blending for pixels in the center of a line.

	vec2 factor = vec2(1.0, 1.0);
	factor.x -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, ivec2(0,  1)).r;
	factor.x -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, ivec2(1,  1)).r;
	factor.y -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, ivec2(0, -2)).r;
	factor.y -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, ivec2(1, -2)).r;

	weights *= saturate(factor);
	return weights; //
	// #endif
}

vec2 SMAADetectVerticalCornerPattern(vec2 weights, vec4 texcoord, vec2 d) {
	//#if !defined(SMAA_DISABLE_CORNER_DETECTION)
	vec2 leftRight = step(d.xy, d.yx);
	vec2 rounding = (1.0 - SMAA_CORNER_ROUNDING_NORM) * leftRight;

	rounding /= leftRight.x + leftRight.y;

	vec2 factor = vec2(1.0, 1.0);
	factor.x -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, ivec2( 1, 0)).g;
	factor.x -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, ivec2( 1, 1)).g;
	factor.y -= rounding.x * SMAASampleLevelZeroOffset(edgesTex, texcoord.xy, ivec2(-2, 0)).g;
	factor.y -= rounding.y * SMAASampleLevelZeroOffset(edgesTex, texcoord.zw, ivec2(-2, 1)).g;

	weights *= saturate(factor);
	return weights; //
	// #endif
}


vec4 SMAABlendingWeightCalculationPS(vec2 texcoord, vec2 pixcoord,
									 vec4 subsampleIndices) { // Just pass zero for SMAA 1x, see @SUBSAMPLE_INDICES.
	vec4 weights = vec4(0.0, 0.0, 0.0, 0.0);

	vec2 e = textureLodA(edgesTex, texcoord, 0.0).rg;

	//SMAA_BRANCH
	if (e.g > 0.0) { // Edge at north
		//#if !defined(SMAA_DISABLE_DIAG_DETECTION)
		// Diagonals have both north and west edges, so searching for them in
		// one of the boundaries is enough.
		weights.rg = SMAACalculateDiagWeights(texcoord, e, subsampleIndices);

		// We give priority to diagonals, so if we find a diagonal we skip 
		// horizontal/vertical processing.
		//SMAA_BRANCH
		if (weights.r == -weights.g) { // weights.r + weights.g == 0.0
		//#endif

		vec2 d;

		// Find the distance to the left:
		vec3 coords;
		coords.x = SMAASearchXLeft(offset0.xy, offset2.x);
		coords.y = offset1.y; // offset[1].y = texcoord.y - 0.25 * screenSizeInv.y (@CROSSING_OFFSET)
		d.x = coords.x;

		// Now fetch the left crossing edges, two at a time using bilinear
		// filtering. Sampling at -0.25 (see @CROSSING_OFFSET) enables to
		// discern what value each edge has:
		float e1 = textureLodA(edgesTex, coords.xy, 0.0).r;

		// Find the distance to the right:
		coords.z = SMAASearchXRight(offset0.zw, offset2.y);
		d.y = coords.z;

		// We want the distances to be in pixel units (doing this here allow to
		// better interleave arithmetic and memory accesses):
		d = abs(round(mad(screenSize.xx, d, -pixcoord.xx)));

		// SMAAArea below needs a sqrt, as the areas texture is compressed
		// quadratically:
		vec2 sqrt_d = sqrt(d);

		// Fetch the right crossing edges:
		float e2 = SMAASampleLevelZeroOffset(edgesTex, coords.zy, ivec2(1, 0)).r;

		// Ok, we know how this pattern looks like, now it is time for getting
		// the actual area:
		weights.rg = SMAAArea(sqrt_d, e1, e2, subsampleIndices.y);

		// Fix corners:
		coords.y = texcoord.y;
		weights.rg = SMAADetectHorizontalCornerPattern(weights.rg, coords.xyzy, d);

		//#if !defined(SMAA_DISABLE_DIAG_DETECTION)
		}
		else {
			e.r = 0.0; // Skip vertical processing.
		}
		//#endif
	}

	//SMAA_BRANCH
	if (e.r > 0.0) { // Edge at west
		vec2 d;

		// Find the distance to the top:
		vec3 coords;
		coords.y = SMAASearchYUp(/*edgesTex, searchTex,*/ offset1.xy, offset2.z);
		coords.x = offset0.x; // offset[1].x = texcoord.x - 0.25 * screenSizeInv.x;
		d.x = coords.y;

		// Fetch the top crossing edges:
		float e1 = textureLodA(edgesTex, coords.xy, 0.0).g;

		// Find the distance to the bottom:
		coords.z = SMAASearchYDown(offset1.zw, offset2.w);
		d.y = coords.z;

		// We want the distances to be in pixel units:
		d = abs(round(mad(screenSize.yy, d, -pixcoord.yy)));

		// SMAAArea below needs a sqrt, as the areas texture is compressed 
		// quadratically:
		vec2 sqrt_d = sqrt(d);

		// Fetch the bottom crossing edges:
		float e2 = SMAASampleLevelZeroOffset(edgesTex, coords.xz, ivec2(0, 1)).g;

		// Get the area for this direction:
		weights.ba = SMAAArea(sqrt_d, e1, e2, subsampleIndices.x);

		// Fix corners:
		coords.x = texcoord.x;
		weights.ba = SMAADetectVerticalCornerPattern(weights.ba, coords.xyxz, d);
	}

	return weights;
}

void main() {
	fragColor = SMAABlendingWeightCalculationPS(texCoord, pixcoord, vec4(0.0));
}
