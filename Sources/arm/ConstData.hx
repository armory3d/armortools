package arm;

class ConstData {

	// 2D layer view
	#if kha_direct3d11

	public static var layerViewVert = "
uniform float4x4 projectionMatrix;
struct SPIRV_Cross_Output { float4 color : TEXCOORD0; float2 texCoord : TEXCOORD1; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float3 vertexPosition : TEXCOORD0, float2 texPosition : TEXCOORD1, float4 vertexColor : TEXCOORD2) {
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = mul(float4(vertexPosition, 1.0f), projectionMatrix);
    stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
    stage_output.texCoord = texPosition;
    stage_output.color = vertexColor;
    return stage_output;
}
";
	public static var layerViewFrag = "
Texture2D<float4> tex;
SamplerState _tex_sampler;
float4 main(float4 color : TEXCOORD0, float2 texCoord : TEXCOORD1) : SV_Target0 {
    return tex.SampleLevel(_tex_sampler, texCoord, 0) * color;
}
";

		#else // kha_opengl

		public static var layerViewVert = "#version 330
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
		public static var layerViewFrag = "#version 330
uniform sampler2D tex;
in vec2 texCoord;
in vec4 color;
out vec4 FragColor;
void main() {
	FragColor = textureLod(tex, texCoord, 0) * color;
}
";
		#end


    // 2D layer merge
    #if kha_direct3d11

    public static var layerMergeVert = "
struct SPIRV_Cross_Output { float2 texCoord : TEXCOORD0; float4 gl_Position : SV_Position; };
SPIRV_Cross_Output main(float2 pos : TEXCOORD0) {
    SPIRV_Cross_Output stage_output;
    stage_output.gl_Position = float4(pos.xy, 0.0, 1.0);
    stage_output.gl_Position.z = (stage_output.gl_Position.z + stage_output.gl_Position.w) * 0.5;
    const float2 madd = float2(0.5, 0.5);
    stage_output.texCoord = pos.xy * madd + madd;
    stage_output.texCoord.y = 1.0 - stage_output.texCoord.y;
    return stage_output;
}
";
    public static var layerMergeFrag = "
Texture2D<float4> tex0;
SamplerState _tex0_sampler;
Texture2D<float4> tex1;
SamplerState _tex1_sampler;
Texture2D<float4> tex2;
SamplerState _tex2_sampler;
Texture2D<float4> texa;
SamplerState _texa_sampler;
Texture2D<float4> texb;
SamplerState _texb_sampler;
Texture2D<float4> texc;
SamplerState _texc_sampler;
uniform float opac;
struct SPIRV_Cross_Output { float4 color0 : SV_Target0; float4 color1 : SV_Target1; float4 color2 : SV_Target2; };
SPIRV_Cross_Output main(float2 texCoord : TEXCOORD0) {
    float4 col0 = tex0.SampleLevel(_tex0_sampler, texCoord, 0);
    float4 col1 = tex1.SampleLevel(_tex1_sampler, texCoord, 0);
    float4 col2 = tex2.SampleLevel(_tex2_sampler, texCoord, 0);
    float4 cola = texa.SampleLevel(_texa_sampler, texCoord, 0);
    float4 colb = texb.SampleLevel(_texb_sampler, texCoord, 0);
    float4 colc = texc.SampleLevel(_texc_sampler, texCoord, 0);
    float str = col0.a * opac;
    float invstr = 1.0 - str;
    SPIRV_Cross_Output stage_output;
    stage_output.color0 = float4(cola.rgb * invstr + col0.rgb * str, max(col0.a, cola.a));
    stage_output.color1 = float4(colb * invstr + col1 * str);
    stage_output.color2 = float4(colc * invstr + col2 * str);
    return stage_output;
}
";
    public static var maskMergeFrag = "
Texture2D<float4> tex0;
SamplerState _tex0_sampler;
Texture2D<float4> texa;
SamplerState _texa_sampler;
struct SPIRV_Cross_Output { float4 color0 : SV_Target0; };
SPIRV_Cross_Output main(float2 texCoord : TEXCOORD0) {
    float4 col0 = tex0.SampleLevel(_tex0_sampler, texCoord, 0);
    float mask = texa.SampleLevel(_texa_sampler, texCoord, 0).r;
    SPIRV_Cross_Output stage_output;
    stage_output.color0 = float4(col0.rgb, col0.a * mask);
    return stage_output;
}
";

        #else // kha_opengl

        public static var layerMergeVert = "#version 330
in vec2 pos;
out vec2 texCoord;
void main() {
    gl_Position = vec4(pos.xy, 0.0, 1.0);
    const vec2 madd = vec2(0.5, 0.5);
    texCoord = pos.xy * madd + madd;
}
";
        public static var layerMergeFrag = "#version 330
uniform sampler2D tex0;
uniform sampler2D tex1;
uniform sampler2D tex2;
uniform sampler2D texa;
uniform sampler2D texb;
uniform sampler2D texc;
uniform float opac;
in vec2 texCoord;
out vec4 FragColor[3];
void main() {
    vec4 col0 = textureLod(tex0, texCoord, 0);
    vec4 col1 = textureLod(tex1, texCoord, 0);
    vec4 col2 = textureLod(tex2, texCoord, 0);
    vec4 cola = textureLod(texa, texCoord, 0);
    vec4 colb = textureLod(texb, texCoord, 0);
    vec4 colc = textureLod(texc, texCoord, 0);
    float str = col0.a * opac;
    float invstr = 1.0 - str;
    FragColor[0] = vec4(cola.rgb * invstr + col0.rgb * str, max(col0.a, cola.a));
    FragColor[1] = vec4(colb * invstr + col1 * str);
    FragColor[2] = vec4(colc * invstr + col2 * str);
}
";
        public static var maskMergeFrag = "#version 330
uniform sampler2D tex0;
uniform sampler2D texa;
in vec2 texCoord;
out vec4 FragColor;
void main() {
    vec4 col0 = textureLod(tex0, texCoord, 0);
    float mask = textureLod(texa, texCoord, 0).r;
    FragColor = vec4(col0.rgb, col0.a * mask);
}
";
        #end
}
