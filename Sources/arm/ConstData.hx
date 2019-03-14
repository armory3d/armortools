package arm;

class ConstData {

	// Simple 2D view shader

	#if kha_direct3d11

	public static var painterVert = "
uniform float4x4 projectionMatrix;
static float4 gl_Position;
static float3 vertexPosition;
static float2 texCoord;
static float2 texPosition;
static float4 color;
static float4 vertexColor;
struct SPIRV_Cross_Input {
    float3 vertexPosition : TEXCOORD0;
    float2 texPosition : TEXCOORD1;
    float4 vertexColor : TEXCOORD2;
};
struct SPIRV_Cross_Output {
    float4 color : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
    float4 gl_Position : SV_Position;
};
void vert_main() {
    gl_Position = mul(float4(vertexPosition, 1.0f), projectionMatrix);
    texCoord = texPosition;
    color = vertexColor;
    gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
}
SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {
    vertexPosition = stage_input.vertexPosition;
    texPosition = stage_input.texPosition;
    vertexColor = stage_input.vertexColor;
    vert_main();
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = gl_Position;
    stage_output.texCoord = texCoord;
    stage_output.color = color;
    return stage_output;
}
";
	public static var painterFrag = "
Texture2D<float4> tex;
SamplerState _tex_sampler;
static float2 texCoord;
static float4 color;
static float4 FragColor;
struct SPIRV_Cross_Input {
    float4 color : TEXCOORD0;
    float2 texCoord : TEXCOORD1;
};
struct SPIRV_Cross_Output {
    float4 FragColor : SV_Target0;
};
void frag_main() {
    float4 texcolor = tex.Sample(_tex_sampler, texCoord) * color;
    FragColor = texcolor;
}
SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {
    texCoord = stage_input.texCoord;
    color = stage_input.color;
    frag_main();
    SPIRV_Cross_Output stage_output;
    stage_output.FragColor = FragColor;
    return stage_output;
}
";

		#else // kha_opengl

		public static var painterVert = "
#version 330
in vec3 vertexPosition;
in vec2 texPosition;
in vec4 vertexColor;
uniform mat4 projectionMatrix;
out vec2 texCoord;
out vec4 color;
void main() {
	gl_Position = projectionMatrix * vec4(vertexPosition, 1.0);
	texCoord = texPosition;
	color = vertexColor;
}
";
		public static var painterFrag = "
#version 330
uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	vec4 texcolor = texture(tex, texCoord) * color;
	FragColor = texcolor;
}
";
		#end
}
