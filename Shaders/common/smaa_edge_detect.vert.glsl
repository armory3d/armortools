#version 450

in vec2 pos;

uniform vec2 screenSizeInv;

out vec2 texCoord;
out vec4 offset0;
out vec4 offset1;
out vec4 offset2;

#ifdef HLSL
#define V_DIR(v) -(v)
#else
#define V_DIR(v) v
#endif

void main() {
	// Scale vertex attribute to [0-1] range
	const vec2 madd = vec2(0.5, 0.5);
	texCoord = pos.xy * madd + madd;
	#ifdef HLSL
	texCoord.y = 1.0 - texCoord.y;
	#endif

	offset0 = screenSizeInv.xyxy * vec4(-1.0, 0.0, 0.0, V_DIR(-1.0)) + texCoord.xyxy;
	offset1 = screenSizeInv.xyxy * vec4( 1.0, 0.0, 0.0, V_DIR(1.0)) + texCoord.xyxy;
	offset2 = screenSizeInv.xyxy * vec4(-2.0, 0.0, 0.0, V_DIR(-2.0)) + texCoord.xyxy;

	gl_Position = vec4(pos.xy, 0.0, 1.0);
}
