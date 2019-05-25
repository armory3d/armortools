// DoF with bokeh GLSL shader by Martins Upitis (martinsh) (devlog-martinsh.blogspot.com)
// Creative Commons Attribution 3.0 Unported License

#include "compiled.inc"
#include "std/math.glsl"

// const float compoDOFDistance = 10.0; // Focal distance value in meters
// const float compoDOFLength = 160.0; // Focal length in mm 18-200
// const float compoDOFFstop = 128.0; // F-stop value

const int samples = 6; // Samples on the first ring
const int rings = 6; // Ring count
const vec2 focus = vec2(0.5, 0.5);
const float coc = 0.11; // Circle of confusion size in mm (35mm film = 0.03mm)
const float maxblur = 1.0;
const float threshold = 0.5; // Highlight threshold
const float gain = 2.0; // Highlight gain
const float bias = 0.5; // Bokeh edge bias
const float fringe = 0.7; // Bokeh chromatic aberration/fringing
const float namount = 0.0001; // Dither amount

vec3 color(vec2 coords, const float blur, const sampler2D tex, const vec2 texStep) {
	vec3 col = vec3(0.0);
	col.r = textureLod(tex, coords + vec2(0.0, 1.0) * texStep * fringe * blur, 0.0).r;
	col.g = textureLod(tex, coords + vec2(-0.866, -0.5) * texStep * fringe * blur, 0.0).g;
	col.b = textureLod(tex, coords + vec2(0.866, -0.5) * texStep * fringe * blur, 0.0).b;
	
	const vec3 lumcoeff = vec3(0.299, 0.587, 0.114);
	float lum = dot(col.rgb, lumcoeff);
	float thresh = max((lum - threshold) * gain, 0.0);
	return col + mix(vec3(0.0), col, thresh * blur);
}

vec3 dof(const vec2 texCoord, const float gdepth, const sampler2D tex, const sampler2D gbufferD, const vec2 texStep, const vec2 cameraProj) {
	float depth = linearize(gdepth, cameraProj);
	// const float fDepth = compoDOFDistance;
	float fDepth = linearize(textureLod(gbufferD, focus, 0.0).r * 2.0 - 1.0, cameraProj); // Autofocus
	
	const float f = compoDOFLength; // Focal length in mm
	const float d = fDepth * 1000.0; // Focal plane in mm
	float o = depth * 1000.0; // Depth in mm
	float a = (o * f) / (o - f); 
	float b = (d * f) / (d - f); 
	float c = (d - f) / (d * compoDOFFstop * coc); 
	float blur = abs(a - b) * c;
	blur = clamp(blur, 0.0, 1.0);
	
	vec2 noise = rand2(texCoord) * namount * blur;
	float w = (texStep.x) * blur * maxblur + noise.x;
	float h = (texStep.y) * blur * maxblur + noise.y;
	vec3 col = vec3(0.0);
	if (blur < 0.05) {
		col = textureLod(tex, texCoord, 0.0).rgb;
	}
	else {
		col = textureLod(tex, texCoord, 0.0).rgb;
		float s = 1.0;
		int ringsamples;
		
		for (int i = 1; i <= rings; ++i) {   
			ringsamples = i * samples;
			for (int j = 0 ; j < ringsamples; ++j) {
				float step = PI2 / float(ringsamples);
				float pw = (cos(float(j) * step) * float(i));
				float ph = (sin(float(j) * step) * float(i));
				float p = 1.0;
				// if (pentagon) p = penta(vec2(pw, ph));
				col += color(texCoord + vec2(pw * w, ph * h), blur, tex, texStep) * mix(1.0, (float(i)) / (float(rings)), bias) * p;  
				s += 1.0 * mix(1.0, (float(i)) / (float(rings)), bias) * p;  
			}
		}
		col /= s;
	}
	return col;
}

