#version 450

#include "compiled.inc"

in vec2 pos;

uniform vec2 screenSize;
uniform vec2 screenSizeInv;

out vec2 texCoord;
out vec2 pixcoord;
out vec4 offset0;
out vec4 offset1;
out vec4 offset2;

const int SMAA_MAX_SEARCH_STEPS = 16;

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	
	// Blend Weight Calculation Vertex Shader
	pixcoord = texCoord * screenSize;

	// We will use these offsets for the searches later on (see @PSEUDO_GATHER4):
	offset0 = screenSizeInv.xyxy * vec4(-0.25, -0.125,  1.25, -0.125) + texCoord.xyxy;
	offset1 = screenSizeInv.xyxy * vec4(-0.125, -0.25, -0.125,  1.25) + texCoord.xyxy;

	// And these for the searches, they indicate the ends of the loops:
	offset2 = screenSizeInv.xxyy *
				(vec4(-2.0, 2.0, -2.0, 2.0) * float(SMAA_MAX_SEARCH_STEPS)) +
				 vec4(offset0.xz, offset1.yw);

	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
