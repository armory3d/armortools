package arm.node;

import zui.Nodes;
import iron.data.SceneFormat;

class MaterialShaderData {
	var material:TMaterial;

	public function new(material:TMaterial) {
		this.material = material;
	}

	public function add_context(props:Dynamic):MaterialShaderContext {
		return new MaterialShaderContext(material, props);
	}
}

class MaterialShaderContext {
	public var vert:MaterialShader;
	public var frag:MaterialShader;
	public var geom:MaterialShader;
	public var tesc:MaterialShader;
	public var tese:MaterialShader;
	public var data:TShaderContext;
	var material:TMaterial;
	var constants:Array<TShaderConstant>;
	var tunits:Array<TTextureUnit>;

	public function new(material:TMaterial, props:Dynamic) {
		this.material = material;
		data = {
			name: props.name,
			depth_write: props.depth_write,
			compare_mode: props.compare_mode,
			cull_mode: props.cull_mode,
			blend_source: props.blend_source,
			blend_destination: props.blend_destination,
			blend_operation: props.blend_operation,
			alpha_blend_source: props.alpha_blend_source,
			alpha_blend_destination: props.alpha_blend_destination,
			alpha_blend_operation: props.alpha_blend_operation,
			fragment_shader: '',
			vertex_shader: '',
			vertex_elements: Reflect.hasField(props, 'vertex_elements') ? props.vertex_elements : [ {name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}]
		};

		if (props.color_writes_red != null)
			data.color_writes_red = props.color_writes_red;
		if (props.color_writes_green != null)
			data.color_writes_green = props.color_writes_green;
		if (props.color_writes_blue != null)
			data.color_writes_blue = props.color_writes_blue;
		if (props.color_writes_alpha != null)
			data.color_writes_alpha = props.color_writes_alpha;

		tunits = data.texture_units = [];
		constants = data.constants = [];
	}

	public function add_elem(name:String, data_type:String) {
		for (e in data.vertex_elements) {
			if (e.name == name) return;
		}
		var elem:TVertexElement = { name: name, data: data_type };
		data.vertex_elements.push(elem);
	}

	public function is_elem(name:String):Bool {
		for (elem in data.vertex_elements)
			if (elem.name == name)
				return true;
		return false;
	}

	public function get_elem(name:String):TVertexElement {
		for (elem in data.vertex_elements) {
			#if cpp
			if (Reflect.field(elem, "name") == name)
			#else
			if (elem.name == name)
			#end {
				return elem;
			}
		}
		return null;
	}

	public function add_constant(ctype:String, name:String, link:String = null) {
		for (c in constants)
			if (c.name == name)
				return;

		var c:TShaderConstant = { name: name, type: ctype };
		if (link != null)
			c.link = link;
		constants.push(c);
	}

	public function add_texture_unit(ctype:String, name:String, link:String = null, is_image = false) {
		for (c in tunits)
			if (c.name == name)
				return;

		var c:TTextureUnit = { name: name };
		if (link != null)
			c.link = link;
		if (is_image)
			c.is_image = is_image;
		tunits.push(c);
	}

	public function make_vert():MaterialShader {
		data.vertex_shader = material.name + '_' + data.name + '.vert';
		vert = new MaterialShader(this, 'vert');
		return vert;
	}

	public function make_frag():MaterialShader {
		data.fragment_shader = material.name + '_' + data.name + '.frag';
		frag = new MaterialShader(this, 'frag');
		return frag;
	}
}

class MaterialShader {

	public var context:MaterialShaderContext;
	var shader_type = '';
	var includes:Array<String> = [];
	public var ins:Array<String> = [];
	public var outs:Array<String> = [];
	public var sharedSamplers:Array<String> = [];
	var uniforms:Array<String> = [];
	var functions = new Map<String, String>();
	public var main = '';
	public var main_init = '';
	public var main_end = '';
	public var main_normal = '';
	public var main_textures = '';
	public var main_attribs = '';
	var header = '';
	public var write_pre = false;
	public var write_normal = 0;
	public var write_textures = 0;
	var vstruct_as_vsin = true;
	var lock = false;

	// References
	public var bposition = false;
	public var wposition = false;
	public var mposition = false;
	public var vposition = false;
	public var wvpposition = false;
	public var ndcpos = false;
	public var wtangent = false;
	public var vVec = false;
	public var vVecCam = false;
	public var n = false;
	public var nAttr = false;
	public var dotNV = false;
	public var invTBN = false;

	public function new(context:MaterialShaderContext, shader_type:String) {
		this.context = context;
		this.shader_type = shader_type;
	}

	public function add_include(s:String) {
		includes.push(s);
	}

	public function add_in(s:String) {
		ins.push(s);
	}

	public function add_out(s:String) {
		outs.push(s);
	}

	public function add_uniform(s:String, link:String = null, included = false) {
		var ar = s.split(' ');
		// layout(RGBA8) image3D voxels
		var utype = ar[ar.length - 2];
		var uname = ar[ar.length - 1];
		if (StringTools.startsWith(utype, 'sampler') || StringTools.startsWith(utype, 'image') || StringTools.startsWith(utype, 'uimage')) {
			var is_image = (StringTools.startsWith(utype, 'image') || StringTools.startsWith(utype, 'uimage')) ? true : false;
			context.add_texture_unit(utype, uname, link, is_image);
		}
		else {
			// Prefer vec4[] for d3d to avoid padding
			if (ar[0] == 'float' && ar[1].indexOf('[') >= 0) {
				ar[0] = 'floats';
				ar[1] = ar[1].split('[')[0];
			}
			else if (ar[0] == 'vec4' && ar[1].indexOf('[') >= 0) {
				ar[0] = 'floats';
				ar[1] = ar[1].split('[')[0];
			}
			context.add_constant(ar[0], ar[1], link);
		}
		if (included == false && uniforms.indexOf(s) == -1) {
			uniforms.push(s);
		}
	}

	public function add_shared_sampler(s:String) {
		if (sharedSamplers.indexOf(s) == -1) {
			sharedSamplers.push(s);
			var ar = s.split(' ');
			// layout(RGBA8) sampler2D tex
			var utype = ar[ar.length - 2];
			var uname = ar[ar.length - 1];
			context.add_texture_unit(utype, uname, null, false);
		}
	}

	public function add_function(s:String) {
		var fname = s.split('(')[0];
		if (functions.exists(fname)) return;
		functions.set(fname, s);
	}

	public function contains(s:String):Bool {
		return main.indexOf(s) >= 0 ||
			   main_init.indexOf(s) >= 0 ||
			   main_normal.indexOf(s) >= 0 ||
			   ins.indexOf(s) >= 0 ||
			   main_textures.indexOf(s) >= 0 ||
			   main_attribs.indexOf(s) >= 0;
	}

	public function write_init(s:String) {
		main_init = s + '\n' + main_init;
	}

	public function write(s:String) {
		if (lock) return;
		if (write_textures > 0) {
			main_textures += s + '\n';
		}
		else if (write_normal > 0) {
			main_normal += s + '\n';
		}
		else if (write_pre) {
			main_init += s + '\n';
		}
		else {
			main += s + '\n';
		}
	}

	public function write_header(s:String) {
		header += s + '\n';
	}

	public function write_end(s:String) {
		main_end += s + '\n';
	}

	public function write_attrib(s:String) {
		main_attribs += s + '\n';
	}

	function dataSize(data:String):String {
		if (data == 'float1') return '1';
		else if (data == 'float2') return '2';
		else if (data == 'float3') return '3';
		else if (data == 'float4') return '4';
		else if (data == 'short2norm') return '2';
		else if (data == 'short4norm') return '4';
		else return '1';
	}

	function vstruct_to_vsin() {
		// if self.shader_type != 'vert' or self.ins != [] or not self.vstruct_as_vsin: # Vertex structure as vertex shader input
			// return
		var vs = context.data.vertex_elements;
		for (e in vs) {
			add_in('vec' + dataSize(e.data) + ' ' + e.name);
		}
	}

	public function get():String {

		if (shader_type == 'vert' && vstruct_as_vsin) {
			vstruct_to_vsin();
		}

		var sharedSampler = 'shared_sampler';
		if (sharedSamplers.length > 0) {
			sharedSampler = sharedSamplers[0].split(' ')[1] + '_sampler';
		}

		#if (kha_direct3d11 || kha_direct3d12)
		var s = '#define HLSL\n';
		s += '#define sampler2D Texture2D\n';
		s += '#define sampler3D Texture3D\n';
		s += '#define texture(tex, coord) tex.Sample(tex ## _sampler, coord)\n';
		s += '#define textureShared(tex, coord) tex.Sample($sharedSampler, coord)\n';
		s += '#define textureLod(tex, coord, lod) tex.SampleLevel(tex ## _sampler, coord, lod)\n';
		s += '#define textureLodShared(tex, coord, lod) tex.SampleLevel($sharedSampler, coord, lod)\n';
		s += '#define texelFetch(tex, coord, lod) tex.Load(float3(coord.xy, lod))\n';
		s += 'uint2 _GetDimensions(Texture2D tex, uint lod) { uint x, y; tex.GetDimensions(x, y); return uint2(x, y); }\n';
		s += '#define textureSize _GetDimensions\n';
		s += '#define mod(a, b) (a % b)\n';
		s += '#define vec2 float2\n';
		s += '#define vec3 float3\n';
		s += '#define vec4 float4\n';
		s += '#define ivec2 int2\n';
		s += '#define ivec3 int3\n';
		s += '#define ivec4 int4\n';
		s += '#define mat2 float2x2\n';
		s += '#define mat3 float3x3\n';
		s += '#define mat4 float4x4\n';
		s += '#define dFdx ddx\n';
		s += '#define dFdy ddy\n';
		s += '#define inversesqrt rsqrt\n';
		s += '#define fract frac\n';
		s += '#define mix lerp\n';
		// s += '#define fma mad\n';
		// s += '#define atan(x, y) atan2(y, x)\n';
		// s += '#define clamp(x, 0.0, 1.0) saturate(x)\n';

		s += header;

		var in_ext = '';
		var out_ext = '';

		for (a in includes)
			s += '#include "' + a + '"\n';

		// Input structure
		var index = 0;
		if (ins.length > 0) {
			s += 'struct SPIRV_Cross_Input {\n';
			index = 0;
			ins.sort(function(a, b):Int {
				// Sort inputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			for (a in ins) {
				s += '$a$in_ext : TEXCOORD$index;\n';
				index++;
			}
			// Built-ins
			if (shader_type == 'vert' && main.indexOf("gl_VertexID") >= 0) {
				s += 'uint gl_VertexID : SV_VertexID;\n';
				ins.push('uint gl_VertexID');
			}
			if (shader_type == 'vert' && main.indexOf("gl_InstanceID") >= 0) {
				s += 'uint gl_InstanceID : SV_InstanceID;\n';
				ins.push('uint gl_InstanceID');
			}
			s += '};\n';
		}

		// Output structure
		var num = 0;
		if (outs.length > 0 || shader_type == 'vert') {
			s += 'struct SPIRV_Cross_Output {\n';
			outs.sort(function(a, b):Int {
				// Sort outputs by name
				return a.substring(4) >= b.substring(4) ? 1 : -1;
			});
			index = 0;
			if (shader_type == 'vert') {
				for (a in outs) {
					s += '$a$out_ext : TEXCOORD$index;\n';
					index++;
				}
				s += 'float4 svpos : SV_POSITION;\n';
			}
			else {
				var out = outs[0];
				// Multiple render targets
				if (out.charAt(out.length - 1) == ']') {
					num = Std.parseInt(out.charAt(out.length - 2));
					s += 'vec4 fragColor[$num] : SV_TARGET0;\n';
				}
				else {
					s += 'vec4 fragColor : SV_TARGET0;\n';
				}
			}
			s += '};\n';
		}

		for (a in uniforms) {
			s += 'uniform ' + a + ';\n';
			#if (kha_direct3d11 || kha_direct3d12)
			if (StringTools.startsWith(a, 'sampler')) {
				s += 'SamplerState ' + a.split(' ')[1] + '_sampler;\n';
			}
			#end
		}

		if (sharedSamplers.length > 0) {
			for (a in sharedSamplers) {
				s += 'uniform ' + a + ';\n';
			}
			#if (kha_direct3d11 || kha_direct3d12)
			s += 'SamplerState $sharedSampler;\n';
			#end
		}

		for (f in functions) {
			s += f + '\n';
		}

		// Begin main
		if (outs.length > 0 || shader_type == 'vert') {
			if (ins.length > 0) {
				s += 'SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {\n';
			}
			else {
				s += 'SPIRV_Cross_Output main() {\n';
			}
		}
		else {
			if (ins.length > 0) {
				s += 'void main(SPIRV_Cross_Input stage_input) {\n';
			}
			else {
				s += 'void main() {\n';
			}
		}

		// Declare inputs
		for (a in ins) {
			var b = a.substring(5); // Remove type 'vec4 '
			s += '$a = stage_input.$b;\n';
		}

		if (shader_type == 'vert') {
			s += 'vec4 gl_Position;\n';
			for (a in outs) {
				s += '$a;\n';
			}
		}
		else {
			if (outs.length > 0) {
				if (num > 0) s += 'vec4 fragColor[$num];\n';
				else s += 'vec4 fragColor;\n';
			}
		}

		s += main_attribs;
		s += main_textures;
		s += main_normal;
		s += main_init;
		s += main;
		s += main_end;

		// Write output structure
		if (shader_type == 'vert') {
			s += 'SPIRV_Cross_Output stage_output;\n';
			s += 'gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;\n';
			s += 'stage_output.svpos = gl_Position;\n';
			for (a in outs) {
				var b = a.substring(5); // Remove type 'vec4 '
				s += 'stage_output.$b = $b;\n';
			}

			s += 'return stage_output;\n';

		}
		else {
			if (outs.length > 0) {
				s += 'SPIRV_Cross_Output stage_output;\n';
				if (num > 0) {
					for (i in 0...num) {
						s += 'stage_output.fragColor[$i] = fragColor[$i];\n';
					}
				}
				else {
					s += 'stage_output.fragColor = fragColor;\n';
				}
				s += 'return stage_output;\n';
			}
		}
		s += '}\n';

		#else // kha_opengl

		#if kha_webgl
		var s = '#version 300 es\n';
		if (shader_type == 'frag') {
			s += 'precision mediump float;\n';
			s += 'precision mediump int;\n';
		}
		#else
		var s = '#version 330\n';
		#end

		s += '#define mul(a, b) b * a\n';
		s += '#define textureShared texture\n';
		s += '#define textureLodShared textureLod\n';
		s += header;

		var in_ext = '';
		var out_ext = '';

		for (a in includes)
			s += '#include "' + a + '"\n';
		for (a in ins)
			s += 'in $a$in_ext;\n';
		for (a in outs)
			s += 'out $a$out_ext;\n';
		for (a in uniforms)
			s += 'uniform ' + a + ';\n';
		for (a in sharedSamplers)
			s += 'uniform ' + a + ';\n';
		for (f in functions)
			s += f + '\n';
		s += 'void main() {\n';
		s += main_attribs;
		s += main_textures;
		s += main_normal;
		s += main_init;
		s += main;
		s += main_end;
		s += '}\n';

		#end

		return s;
	}
}

typedef TMaterial = {
	var name:String;
	var canvas:TNodeCanvas;
}
