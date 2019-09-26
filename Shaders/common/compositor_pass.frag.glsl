#version 450

#define _CToneFilmic
#define _CVignette
#define _CGrainStatic

#include "../std/tonemap.glsl"
#include "../std/math.glsl"
#ifdef _CDOF
#include "../std/dof.glsl"
#endif
#ifdef _CDenoise
#include "../std/denoise.glsl"
#endif

const float compoVignetteStrength = 0.4;

uniform sampler2D tex;
#ifdef _CDepth
uniform sampler2D gbufferD;
#endif

#ifdef _CLensTex
uniform sampler2D lensTexture;
#endif

#ifdef _CLUT
uniform sampler2D lutTexture;
#endif

#ifdef _AutoExposure
uniform sampler2D histogram;
#endif

// #ifdef _CPos
// uniform vec3 eye;
// uniform vec3 eyeLook;
// #endif

#ifdef _CGlare
uniform vec3 light;
uniform mat4 VP;
uniform vec3 eye;
uniform vec3 eyeLook;
uniform float aspectRatio;
#endif

#ifdef _CTexStep
uniform vec2 texStep;
#endif

#ifdef _CGrain
uniform float time;
#endif

#ifdef _DynRes
uniform float dynamicScale;
#endif

#ifdef _CCameraProj
uniform vec2 cameraProj;
#endif

in vec2 texCoord;
// #ifdef _CPos
	// in vec3 viewRay;
// #endif
out vec4 fragColor;

#ifdef _CFog
// const vec3 compoFogColor = vec3(0.5, 0.6, 0.7);
// const float compoFogAmountA = 1.0; // b = 0.01
// const float compoFogAmountB = 1.0; // c = 0.1
// vec3 applyFog(vec3 rgb, // original color of the pixel
		 // float distance, // camera to point distance
		 // vec3 rayOri, // camera position
		 // vec3 rayDir) { // camera to point vector
	// float fogAmount = compoFogAmountB * exp(-rayOri.y * compoFogAmountA) * (1.0 - exp(-distance * rayDir.y * compoFogAmountA)) / rayDir.y;
	// return mix(rgb, compoFogColor, fogAmount);
// }
vec3 applyFog(vec3 rgb, float distance) {
	// float fogAmount = 1.0 - exp(-distance * compoFogAmountA);
	float fogAmount = 1.0 - exp(-distance * (compoFogAmountA / 100));
	return mix(rgb, compoFogColor, fogAmount);
}
#endif

vec4 LUTlookup(in vec4 textureColor, in sampler2D lookupTable) {

    //Clamp to prevent weird results
    textureColor = clamp(textureColor, 0.0, 1.0);

    mediump float blueColor = textureColor.b * 63.0;
    mediump vec2 quad1;

    quad1.y = floor(floor(blueColor) / 8.0);
    quad1.x = floor(blueColor) - (quad1.y * 8.0);

    mediump vec2 quad2;
    quad2.y = floor(ceil(blueColor) / 8.0);
    quad2.x = ceil(blueColor) - (quad2.y * 8.0);

    highp vec2 texelPosition1;
    texelPosition1.x = (quad1.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
    texelPosition1.y = (quad1.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);

    highp vec2 texelPosition2;
    texelPosition2.x = (quad2.x * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.r);
    texelPosition2.y = (quad2.y * 0.125) + 0.5/512.0 + ((0.125 - 1.0/512.0) * textureColor.g);

    lowp vec4 newColor1 = textureLod(lookupTable, texelPosition1, 0.0);
    lowp vec4 newColor2 = textureLod(lookupTable, texelPosition2, 0.0);

    lowp vec4 colorGradedResult = mix(newColor1, newColor2, fract(blueColor));

    return colorGradedResult;

}

#ifdef _CVignette
float vignette() {
	return (1.0 - compoVignetteStrength) + compoVignetteStrength * pow(16.0 * texCoord.x * texCoord.y * (1.0 - texCoord.x) * (1.0 - texCoord.y), 0.2);
}
#endif

#ifdef _CGlare
// Based on lense flare implementation by musk
// https://www.shadertoy.com/view/4sX3Rs 
vec3 lensflare(vec2 uv, vec2 pos) {
	vec2 uvd = uv * (length(uv));
	float f2 = max(1.0/(1.0+32.0*pow(length(uvd+0.8*pos),2.0)),0.0)*0.25;
	float f22 = max(1.0/(1.0+32.0*pow(length(uvd+0.85*pos),2.0)),0.0)*0.23;
	float f23 = max(1.0/(1.0+32.0*pow(length(uvd+0.9*pos),2.0)),0.0)*0.21;
	
	vec2 uvx = mix(uv, uvd, -0.5);
	float f4 = max(0.01-pow(length(uvx+0.4*pos),2.4),0.0)*6.0;
	float f42 = max(0.01-pow(length(uvx+0.45*pos),2.4),0.0)*5.0;
	float f43 = max(0.01-pow(length(uvx+0.5*pos),2.4),0.0)*3.0;
	
	uvx = mix(uv, uvd, -0.4);
	float f5 = max(0.01-pow(length(uvx+0.2*pos),5.5),0.0)*2.0;
	float f52 = max(0.01-pow(length(uvx+0.4*pos),5.5),0.0)*2.0;
	float f53 = max(0.01-pow(length(uvx+0.6*pos),5.5),0.0)*2.0;
	
	uvx = mix(uv, uvd, -0.5);
	float f6 = max(0.01-pow(length(uvx-0.3*pos),1.6),0.0)*6.0;
	float f62 = max(0.01-pow(length(uvx-0.325*pos),1.6),0.0)*3.0;
	float f63 = max(0.01-pow(length(uvx-0.35*pos),1.6),0.0)*5.0;
	
	vec3 c = vec3(0.0);
	c.r += f2 + f4 + f5 + f6;
	c.g += f22 + f42 + f52 + f62;
	c.b += f23 + f43 + f53 + f63;
	return c;
}
#endif

void main() {
	vec2 texCo = texCoord;
#ifdef _DynRes
	texCo *= dynamicScale;
#endif

#ifdef _CFishEye
	const float fishEyeStrength = -0.01;
	const vec2 m = vec2(0.5, 0.5);
	vec2 d = texCo - m;
	float r = sqrt(dot(d, d));
	float power = (2.0 * PI / (2.0 * sqrt(dot(m, m)))) * fishEyeStrength;
	float bind;
	if (power > 0.0) { bind = sqrt(dot(m, m)); }
	else { bind = m.x; }
	if (power > 0.0) {
		texCo = m + normalize(d) * tan(r * power) * bind / tan(bind * power);
	}
	else {
		texCo = m + normalize(d) * atan(r * -power * 10.0) * bind / atan(-power * bind * 10.0);
	}
#endif

#ifdef _CDepth
	float depth = textureLod(gbufferD, texCo, 0.0).r * 2.0 - 1.0;
#endif

#ifdef _CFXAA
	const float FXAA_REDUCE_MIN = 1.0 / 128.0;
	const float FXAA_REDUCE_MUL = 1.0 / 8.0;
	const float FXAA_SPAN_MAX = 8.0;
	
	vec2 tcrgbNW = (texCo + vec2(-1.0, -1.0) * texStep);
	vec2 tcrgbNE = (texCo + vec2(1.0, -1.0) * texStep);
	vec2 tcrgbSW = (texCo + vec2(-1.0, 1.0) * texStep);
	vec2 tcrgbSE = (texCo + vec2(1.0, 1.0) * texStep);
	vec2 tcrgbM = vec2(texCo);
	
	vec3 rgbNW = textureLod(tex, tcrgbNW, 0.0).rgb;
	vec3 rgbNE = textureLod(tex, tcrgbNE, 0.0).rgb;
	vec3 rgbSW = textureLod(tex, tcrgbSW, 0.0).rgb;
	vec3 rgbSE = textureLod(tex, tcrgbSE, 0.0).rgb;
	vec3 rgbM  = textureLod(tex, tcrgbM, 0.0).rgb;
	vec3 luma = vec3(0.299, 0.587, 0.114);
	float lumaNW = dot(rgbNW, luma);
	float lumaNE = dot(rgbNE, luma);
	float lumaSW = dot(rgbSW, luma);
	float lumaSE = dot(rgbSE, luma);
	float lumaM  = dot(rgbM,  luma);
	float lumaMin = min(lumaM, min(min(lumaNW, lumaNE), min(lumaSW, lumaSE)));
	float lumaMax = max(lumaM, max(max(lumaNW, lumaNE), max(lumaSW, lumaSE)));
	
	vec2 dir;
	dir.x = -((lumaNW + lumaNE) - (lumaSW + lumaSE));
	dir.y =  ((lumaNW + lumaSW) - (lumaNE + lumaSE));
	
	float dirReduce = max((lumaNW + lumaNE + lumaSW + lumaSE) *
						  (0.25 * FXAA_REDUCE_MUL), FXAA_REDUCE_MIN);
	
	float rcpDirMin = 1.0 / (min(abs(dir.x), abs(dir.y)) + dirReduce);
	dir = min(vec2(FXAA_SPAN_MAX, FXAA_SPAN_MAX),
			  max(vec2(-FXAA_SPAN_MAX, -FXAA_SPAN_MAX),
			  dir * rcpDirMin)) * texStep;
			  
	vec3 rgbA = 0.5 * (
		textureLod(tex, texCo + dir * (1.0 / 3.0 - 0.5), 0.0).rgb +
		textureLod(tex, texCo + dir * (2.0 / 3.0 - 0.5), 0.0).rgb);
	vec3 rgbB = rgbA * 0.5 + 0.25 * (
		textureLod(tex, texCo + dir * -0.5, 0.0).rgb +
		textureLod(tex, texCo + dir * 0.5, 0.0).rgb);
	
	float lumaB = dot(rgbB, luma);
	if ((lumaB < lumaMin) || (lumaB > lumaMax)) fragColor = vec4(rgbA, 1.0);
	else fragColor = vec4(rgbB, 1.0);

#else
	
	#ifdef _CDOF
	fragColor = vec4(dof(texCo, depth, tex, gbufferD, texStep, cameraProj), 1.0);
	#else
		#ifdef _CDenoise
		fragColor = smartDeNoise(tex, texCo, 5.0, 2.0, 0.100);
		#else
		fragColor = textureLod(tex, texCo, 0.0);
		#endif
	#endif

#endif
	
#ifdef _CSharpen
	vec3 col1 = textureLod(tex, texCo + vec2(-texStep.x, -texStep.y) * 1.5, 0.0).rgb;
	vec3 col2 = textureLod(tex, texCo + vec2(texStep.x, -texStep.y) * 1.5, 0.0).rgb;
	vec3 col3 = textureLod(tex, texCo + vec2(-texStep.x, texStep.y) * 1.5, 0.0).rgb;
	vec3 col4 = textureLod(tex, texCo + vec2(texStep.x, texStep.y) * 1.5, 0.0).rgb;
	vec3 colavg = (col1 + col2 + col3 + col4) * 0.25;
	fragColor.rgb += (fragColor.rgb - colavg) * compoSharpenStrength;
#endif

#ifdef _CFog
	// if (depth < 1.0) {
		// vec3 pos = getPos(depth, cameraProj);
		// float dist = distance(pos, eye);
		float dist = linearize(depth, cameraProj);
		// vec3 eyedir = eyeLook;// normalize(eye + pos);
		// fragColor.rgb = applyFog(fragColor.rgb, dist, eye, eyedir);
		fragColor.rgb = applyFog(fragColor.rgb, dist);
	// }
#endif

#ifdef _CGlare
	if (dot(light, eyeLook) > 0.0) { // Facing light
		vec4 lndc = VP * vec4(light, 1.0);
		lndc.xy /= lndc.w;
		vec2 lss = lndc.xy * 0.5 + 0.5;
		float lssdepth = linearize(textureLod(gbufferD, lss, 0.0).r * 2.0 - 1.0, cameraProj);
		float lightDistance = distance(eye, light);
		if (lightDistance <= lssdepth) {
			vec2 lensuv = texCo * 2.0 - 1.0;
			lensuv.x *= aspectRatio;
			vec3 lensflarecol = vec3(1.4, 1.2, 1.0) * lensflare(lensuv, lndc.xy);
			fragColor.rgb += lensflarecol;
		}
	}
#endif

#ifdef _CGrain
	// const float compoGrainStrength = 4.0;
	float x = (texCo.x + 4.0) * (texCo.y + 4.0) * (time * 10.0);
	fragColor.rgb += vec3(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * compoGrainStrength;
#endif
	
#ifdef _CGrainStatic
	float x = (texCo.x + 4.0) * (texCo.y + 4.0) * 10.0;
	fragColor.rgb += vec3(mod((mod(x, 13.0) + 1.0) * (mod(x, 123.0) + 1.0), 0.01) - 0.005) * 0.09;
#endif

#ifdef _CVignette
	fragColor.rgb *= vignette();
#endif

#ifdef _CExposure
	fragColor.rgb += fragColor.rgb * compoExposureStrength;
#endif

#ifdef _AutoExposure
	float expo = 2.0 - clamp(length(textureLod(histogram, vec2(0.5, 0.5), 0).rgb), 0.0, 1.0);
	fragColor.rgb *= pow(expo, autoExposureStrength * 2.0);
#endif

#ifdef _CToneFilmic
	fragColor.rgb = tonemapFilmic(fragColor.rgb); // With gamma
#endif
#ifdef _CToneFilmic2
	fragColor.rgb = acesFilm(fragColor.rgb);
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
#endif
#ifdef _CToneReinhard
	fragColor.rgb = tonemapReinhard(fragColor.rgb);
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2));
#endif
#ifdef _CToneUncharted
	fragColor.rgb = tonemapUncharted2(fragColor.rgb);
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2)); // To gamma
#endif
#ifdef _CToneNone
	fragColor.rgb = pow(fragColor.rgb, vec3(1.0 / 2.2)); // To gamma
#endif
	
#ifdef _CBW
	// fragColor.rgb = vec3(clamp(dot(fragColor.rgb, fragColor.rgb), 0.0, 1.0));
	fragColor.rgb = vec3((fragColor.r * 0.3 + fragColor.g * 0.59 + fragColor.b * 0.11) / 3.0) * 2.5;
#endif

// #ifdef _CContrast
	// -0.5 - 0.5
	// const float compoContrast = 0.2;
	// fragColor.rgb = ((fragColor.rgb - 0.5) * max(compoContrast + 1.0, 0.0)) + 0.5;
// #endif

// #ifdef _CBrighness
	// fragColor.rgb += compoBrightness;
// #endif

#ifdef _CLensTex
	fragColor.rgb += textureLod(lensTexture, texCo, 0.0).rgb;
#endif

#ifdef _CLetterbox
	// const float compoLetterboxSize = 0.1;
	fragColor.rgb *= 1.0 - step(0.5 - compoLetterboxSize, abs(0.5 - texCo.y));
#endif

//3D LUT Implementation from GPUGems 2 by Nvidia
//https://developer.nvidia.com/gpugems/GPUGems2/gpugems2_chapter24.html

#ifdef _CLUT
	fragColor = LUTlookup(fragColor, lutTexture);
#endif
}
