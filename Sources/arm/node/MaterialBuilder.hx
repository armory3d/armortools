package arm.node;

import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.node.MaterialShader;
import arm.Tool;

class MaterialBuilder {

	public static var heightUsed = false;
	public static var emisUsed = false;
	public static var subsUsed = false;

	public static var opacityDiscardDecal = 0.05;
	public static var opacityDiscardScene = 0.5;

	public static function make_paint(data:MaterialShaderData, matcon:TMaterialContext):MaterialShaderContext {
		var layered = Context.layer != Project.layers[0];
		var eraser = Context.tool == ToolEraser;
		var context_id = 'paint';
		var con_paint:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always', // TODO: align texcoords winding order
			// cull_mode: 'counter_clockwise',
			cull_mode: 'none',
			vertex_elements: [{name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}, {name: "tex", data: 'short2norm'}]
		});

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		#if kha_direct3d12
		if (Context.tool == ToolBake && UITrait.inst.bakeType == -1) {
			// Init raytraced bake
			vert.add_out('vec3 position');
			vert.add_out('vec3 normal');
			vert.write('position = pos.xyz;');
			vert.write('normal = vec3(nor.xy, pos.w);');
			vert.add_uniform('vec2 sub', '_sub');
			vert.write('vec2 subtex = tex + sub;');
			vert.write('vec2 tpos = vec2(subtex.x * 2.0 - 1.0, (1.0 - subtex.y) * 2.0 - 1.0);');
			vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');
			frag.add_out('vec4 fragColor[2]');
			frag.write('fragColor[0] = vec4(position, 1.0);');
			frag.write('fragColor[1] = vec4(normal, 1.0);');

			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}
		#end

		if (Context.tool == ToolBake) {
			// Bake into base color, disable other slots
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
			con_paint.data.color_writes_alpha[1] = false;
			con_paint.data.color_writes_red[2] = false;
			con_paint.data.color_writes_green[2] = false;
			con_paint.data.color_writes_blue[2] = false;
			con_paint.data.color_writes_alpha[2] = false;
		}

		if (Context.tool == ToolColorId || Context.tool == ToolPicker) {
			// Mangle vertices to form full screen triangle
			vert.write('gl_Position = vec4(-1.0 + float((gl_VertexID & 1) << 2), -1.0 + float((gl_VertexID & 2) << 1), 0.0, 1.0);');

			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('vec4 inp', '_inputBrush');

			#if (kha_opengl || kha_webgl)
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			#else
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			#end

			if (Context.tool == ToolColorId) {
				frag.add_out('vec4 fragColor');
				frag.add_uniform('sampler2D texcolorid', '_texcolorid');
				frag.write('vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
				frag.write('fragColor = vec4(idcol, 1.0);');
			}
			else if (Context.tool == ToolPicker) {
				frag.add_out('vec4 fragColor[3]');
				frag.add_uniform('sampler2D texpaint');
				frag.add_uniform('sampler2D texpaint_nor');
				frag.add_uniform('sampler2D texpaint_pack');
				frag.write('fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
				frag.write('fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
				frag.write('fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
				frag.write('fragColor[0].a = texCoordInp.x;');
				frag.write('fragColor[1].a = texCoordInp.y;');
			}

			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		var faceFill = Context.tool == ToolFill && UITrait.inst.fillTypeHandle.position == 1;
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;
		if (!faceFill && !decal) { // Fix seams at uv borders
			vert.add_uniform('vec2 sub', '_sub');
			vert.write('vec2 subtex = tex + sub;');
		}
		else {
			vert.write('vec2 subtex = tex;');
		}

		#if (kha_direct3d11 || kha_direct3d12)
		vert.write('vec2 tpos = vec2(subtex.x * 2.0 - 1.0, (1.0 - subtex.y) * 2.0 - 1.0);');
		#else
		vert.write('vec2 tpos = vec2(subtex.xy * 2.0 - 1.0);');
		#end

		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec4 ndc');
		vert.write_attrib('ndc = mul(vec4(pos.xyz, 1.0), WVP);');

		frag.write_attrib('vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		frag.write_attrib('sp.y = 1.0 - sp.y;');
		frag.write_attrib('sp.z -= 0.0001;'); // small bias

		var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UITrait.inst.brushPaint;
		if (uvType == 2) frag.ndcpos = true; // Project

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D gbufferD');

		frag.add_out('vec4 fragColor[4]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushHardness', '_brushHardness');

		if (Context.tool == ToolBrush  ||
			Context.tool == ToolEraser ||
			Context.tool == ToolClone  ||
			Context.tool == ToolBlur   ||
			Context.tool == ToolParticle ||
			decal) {

			var depthReject = !UITrait.inst.xray;
			if (UITrait.inst.brush3d && !UITrait.inst.brushDepthReject) depthReject = false;

			// TODO: sp.z needs to take height channel into account
			if (heightUsed) depthReject = false;

			if (depthReject) {
				#if (kha_opengl || kha_webgl)
				frag.write('if (sp.z > textureLod(gbufferD, vec2(sp.x, 1.0 - sp.y), 0.0).r) discard;');
				#else
				frag.write('if (sp.z > textureLod(gbufferD, sp.xy, 0.0).r) discard;');
				#end
			}

			if (decal || Context.tool == ToolParticle) {
				frag.write('float dist = 0.0;');
			}
			else {
				if (UITrait.inst.brush3d) {

					#if (kha_opengl || kha_webgl)
					frag.write('float depth = textureLod(gbufferD, vec2(inp.x, 1.0 - inp.y), 0.0).r;');
					#else
					frag.write('float depth = textureLod(gbufferD, inp.xy, 0.0).r;');
					#end

					frag.add_uniform('mat4 invVP', '_inverseViewProjectionMatrix');
					frag.write('vec4 winp = vec4(vec2(inp.x, 1.0 - inp.y) * 2.0 - 1.0, depth * 2.0 - 1.0, 1.0);');
					frag.write('winp = mul(winp, invVP);');
					frag.write('winp.xyz /= winp.w;');
					frag.wposition = true;

					if (UITrait.inst.brushAngleReject || UITrait.inst.xray) {
						frag.add_function(MaterialFunctions.str_octahedronWrap);
						frag.add_uniform('sampler2D gbuffer0');
						#if (kha_opengl || kha_webgl)
						frag.write('vec2 g0 = textureLod(gbuffer0, vec2(inp.x, 1.0 - inp.y), 0.0).rg;');
						#else
						frag.write('vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;');
						#end
						frag.write('vec3 wn;');
						frag.write('wn.z = 1.0 - abs(g0.x) - abs(g0.y);');
						frag.write('wn.xy = wn.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);');
						frag.write('wn = normalize(wn);');
						frag.write('float planeDist = dot(wn, winp.xyz - wposition);');

						if (UITrait.inst.brushAngleReject && !UITrait.inst.xray) {
							frag.write('if (planeDist < -0.01) discard;');
							frag.n = true;
							var angle = UITrait.inst.brushAngleRejectDot;
							frag.write('if (dot(wn, n) < $angle) discard;');
						}
					}

					#if (kha_opengl || kha_webgl)
					frag.write('float depthlast = textureLod(gbufferD, vec2(inplast.x, 1.0 - inplast.y), 0.0).r;');
					#else
					frag.write('float depthlast = textureLod(gbufferD, inplast.xy, 0.0).r;');
					#end

					frag.write('vec4 winplast = vec4(vec2(inplast.x, 1.0 - inplast.y) * 2.0 - 1.0, depthlast * 2.0 - 1.0, 1.0);');
					frag.write('winplast = mul(winplast, invVP);');
					frag.write('winplast.xyz /= winplast.w;');

					frag.write('vec3 pa = wposition - winp.xyz;');
					if (UITrait.inst.xray) {
						frag.write('pa += wn * vec3(planeDist, planeDist, planeDist);');
					}
					frag.write('vec3 ba = winplast.xyz - winp.xyz;');
					frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
					frag.write('float dist = length(pa - ba * h);');
					frag.write('if (dist > brushRadius) discard;');

					// Non-continuous
					// frag.write('float dist = distance(wposition, winp.xyz);');
				}
				else { // !brush3d
					frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
					frag.write('binp.x *= aspectRatio;');
					frag.write('binp = binp * 0.5 + 0.5;');

					frag.write('vec2 binplast = inplast.xy * 2.0 - 1.0;');
					frag.write('binplast.x *= aspectRatio;');
					frag.write('binplast = binplast * 0.5 + 0.5;');

					frag.write('vec2 pa = bsp.xy - binp.xy;');
					frag.write('vec2 ba = binplast.xy - binp.xy;');
					frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
					frag.write('float dist = length(pa - ba * h);');

					frag.write('if (dist > brushRadius) discard;');

					// Non-continuous
					// frag.write('float dist = distance(bsp.xy, binp.xy);');
				}
			}
		}
		else { // Fill, Bake
			frag.write('float dist = 0.0;');

			var angleFill = Context.tool == ToolFill && UITrait.inst.fillTypeHandle.position == 2;
			if (angleFill) {
				frag.add_function(MaterialFunctions.str_octahedronWrap);
				frag.add_uniform('sampler2D gbuffer0');
				frag.write('vec2 g0 = textureLod(gbuffer0, inp.xy, 0.0).rg;');
				frag.write('vec3 wn;');
				frag.write('wn.z = 1.0 - abs(g0.x) - abs(g0.y);');
				frag.write('wn.xy = wn.z >= 0.0 ? g0.xy : octahedronWrap(g0.xy);');
				frag.write('wn = normalize(wn);');
				frag.n = true;
				var angle = UITrait.inst.brushAngleRejectDot;
				frag.write('if (dot(wn, n) < $angle) discard;');
			}
		}

		if (UITrait.inst.colorIdPicked) {
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(subtex);');
			frag.add_uniform('sampler2D texpaint_colorid'); // 1x1 picker
			frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
			frag.add_uniform('vec2 texcoloridSize', '_texcoloridSize'); // color map
			frag.write('vec3 c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
			frag.write('vec3 c2 = texelFetch(texcolorid, ivec2(texCoordPick * texcoloridSize), 0).rgb;');
			frag.write('if (any(c1 != c2)) discard;');
		}
		else if (faceFill) { // TODO: allow to combine with colorid mask
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(subtex);');
			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('sampler2D textrianglemap', '_textrianglemap'); // triangle map
			frag.add_uniform('float textrianglemapSize', '_texpaintSize');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			#if (kha_opengl || kha_webgl)
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, (1.0 - inp.y) * gbufferSize.y), 0).ba;');
			#else
			frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(inp.x * gbufferSize.x, inp.y * gbufferSize.y), 0).ba;');
			#end
			frag.write('vec4 c1 = texelFetch(textrianglemap, ivec2(texCoordInp * textrianglemapSize), 0);');
			frag.write('vec4 c2 = texelFetch(textrianglemap, ivec2(texCoordPick * textrianglemapSize), 0);');
			frag.write('if (any(c1 != c2)) discard;');
		}

		if (UITrait.inst.pickerMaskHandle.position == 1) { // material id mask
			frag.wvpposition = true;
			frag.write('vec2 picker_sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('picker_sample_tc.y = 1.0 - picker_sample_tc.y;');
			#end
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			var matid = UITrait.inst.materialIdPicked / 255;
			frag.write('if ($matid != textureLod(texpaint_nor_undo, picker_sample_tc, 0.0).a) discard;');
		}

		// TexCoords - project
		var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UITrait.inst.brushPaint;
		if (uvType == 2 || decal) {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (decal) {
				frag.write_attrib('uvsp -= inp.xy;');
				frag.write_attrib('uvsp.x *= aspectRatio;');

				frag.write_attrib('uvsp *= 0.21 / (brushRadius * 0.9);');

				frag.add_uniform('float brushScaleX', '_brushScaleX');
				frag.write_attrib('uvsp.x *= brushScaleX;');

				frag.write_attrib('uvsp += vec2(0.5, 0.5);');

				frag.write_attrib('if (uvsp.x < 0.01 || uvsp.y < 0.01 || uvsp.x > 0.99 || uvsp.y > 0.99) discard;');
			}
			else {
				frag.write_attrib('uvsp.x *= aspectRatio;');
			}

			frag.write_attrib('vec2 texCoord = fract(uvsp * brushScale);');

			var uvRot = Context.layer.material_mask != null ? Context.layer.uvRot : UITrait.inst.brushRot;
			if (uvRot > 0.0) {
				var a = uvRot * (Math.PI / 180);
				frag.write('texCoord = vec2(texCoord.x * ${Math.cos(a)} - texCoord.y * ${Math.sin(a)}, texCoord.x * ${Math.sin(a)} + texCoord.y * ${Math.cos(a)});');
			}
		}
		// TexCoords - uvmap
		else if (uvType == 0) {
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = subtex * brushScale;');

			var uvRot = Context.layer.material_mask != null ? Context.layer.uvRot : UITrait.inst.brushRot;
			if (uvRot > 0.0) {
				var a = uvRot * (Math.PI / 180);
				vert.write('texCoord = vec2(texCoord.x * ${Math.cos(a)} - texCoord.y * ${Math.sin(a)}, texCoord.x * ${Math.sin(a)} + texCoord.y * ${Math.cos(a)});');
			}
		}
		else { // Triplanar
			frag.wposition = true;
			frag.n = true;
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec3 triWeight = wnormal * wnormal;'); // n * n
			frag.write_attrib('float triMax = max(triWeight.x, max(triWeight.y, triWeight.z));');
			frag.write_attrib('triWeight = max(triWeight - triMax * 0.75, 0.0);');
			frag.write_attrib('vec3 texCoordBlend = triWeight * (1.0 / (triWeight.x + triWeight.y + triWeight.z));');
			frag.write_attrib('vec2 texCoord = fract(wposition.yz * brushScale * 0.5);');
			frag.write_attrib('vec2 texCoord1 = fract(wposition.xz * brushScale * 0.5);');
			frag.write_attrib('vec2 texCoord2 = fract(wposition.xy * brushScale * 0.5);');
		}

		if (Context.tool == ToolClone || Context.tool == ToolBlur) {

			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');

			if (Context.tool == ToolClone) {
				frag.add_uniform('vec2 cloneDelta', '_cloneDelta');
				#if (kha_opengl || kha_webgl)
				frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDelta.x) * gbufferSize.x, (1.0 - (sp.y + cloneDelta.y)) * gbufferSize.y), 0).ba;');
				#else
				frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2((sp.x + cloneDelta.x) * gbufferSize.x, (sp.y + cloneDelta.y) * gbufferSize.y), 0).ba;');
				#end

				frag.write('vec3 texpaint_pack_sample = textureLod(texpaint_pack_undo, texCoordInp, 0.0).rgb;');
				var base = 'textureLod(texpaint_undo, texCoordInp, 0.0).rgb';
				var rough = 'texpaint_pack_sample.g';
				var met = 'texpaint_pack_sample.b';
				var occ = 'texpaint_pack_sample.r';
				var nortan = 'textureLod(texpaint_nor_undo, texCoordInp, 0.0).rgb';
				var height = '0.0';
				var opac = '1.0';
				frag.write('vec3 basecol = $base;');
				frag.write('float roughness = $rough;');
				frag.write('float metallic = $met;');
				frag.write('float occlusion = $occ;');
				frag.write('vec3 nortan = $nortan;');
				frag.write('float height = $height;');
				frag.write('float opacity = $opac * brushOpacity;');
				if (Context.material.paintEmis) {
					frag.write('float emis = 0.0;');
				}
				if (Context.material.paintSubs) {
					frag.write('float subs = 0.0;');
				}
			}
			else { // Blur
				#if (kha_opengl || kha_webgl)
				frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, (1.0 - sp.y) * gbufferSize.y), 0).ba;');
				#else
				frag.write('vec2 texCoordInp = texelFetch(gbuffer2, ivec2(sp.x * gbufferSize.x, sp.y * gbufferSize.y), 0).ba;');
				#end

				frag.write('vec3 basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('float roughness = 0.0;');
				frag.write('float metallic = 0.0;');
				frag.write('float occlusion = 0.0;');
				frag.write('vec3 nortan = vec3(0.0, 0.0, 0.0);');
				frag.write('float height = 0.0;');
				frag.write('float opacity = 1.0 * brushOpacity;');
				if (Context.material.paintEmis) {
					frag.write('float emis = 0.0;');
				}
				if (Context.material.paintSubs) {
					frag.write('float subs = 0.0;');
				}
				#if (kha_direct3d11 || kha_direct3d12)
				frag.write('const float blur_weight[15] = {0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0};');
				#else
				frag.write('const float blur_weight[15] = float[](0.034619 / 2.0, 0.044859 / 2.0, 0.055857 / 2.0, 0.066833 / 2.0, 0.076841 / 2.0, 0.084894 / 2.0, 0.090126 / 2.0, 0.09194 / 2.0, 0.090126 / 2.0, 0.084894 / 2.0, 0.076841 / 2.0, 0.066833 / 2.0, 0.055857 / 2.0, 0.044859 / 2.0, 0.034619 / 2.0);');
				#end
				frag.add_uniform('float texpaintSize', '_texpaintSize');
				frag.write('float blur_step = 1.0 / texpaintSize;');
				frag.write('for (int i = -7; i <= 7; i++) {');
				frag.write('basecol += texture(texpaint_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
				frag.write('vec3 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
				frag.write('roughness += texpaint_pack_sample.g;');
				frag.write('metallic += texpaint_pack_sample.b;');
				frag.write('occlusion += texpaint_pack_sample.r;');
				frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(blur_step * i, 0.0)).rgb * blur_weight[i + 7];');
				frag.write('}');
				frag.write('for (int i = -7; i <= 7; i++) {');
				frag.write('basecol += texture(texpaint_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
				frag.write('vec3 texpaint_pack_sample = texture(texpaint_pack_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
				frag.write('roughness += texpaint_pack_sample.g;');
				frag.write('metallic += texpaint_pack_sample.b;');
				frag.write('occlusion += texpaint_pack_sample.r;');
				frag.write('nortan += texture(texpaint_nor_undo, texCoordInp + vec2(0.0, blur_step * i)).rgb * blur_weight[i + 7];');
				frag.write('}');
			}
		}
		else {
			Material.parse_emission = Context.material.paintEmis;
			Material.parse_subsurface = Context.material.paintSubs;
			Material.parse_height = Context.material.paintHeight;
			Material.parse_height_as_channel = true;
			var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UITrait.inst.brushPaint;
			Material.triplanar = uvType == 1;
			var sout = Material.parse(UINodes.inst.canvas, con_paint, vert, frag, null, null, null, matcon);
			Material.parse_emission = false;
			Material.parse_subsurface = false;
			Material.parse_height_as_channel = false;
			Material.parse_height = false;
			var base = sout.out_basecol;
			var rough = sout.out_roughness;
			var met = sout.out_metallic;
			var occ = sout.out_occlusion;
			var nortan = Material.out_normaltan;
			var height = sout.out_height;
			var opac = sout.out_opacity;
			var emis = sout.out_emission;
			var subs = sout.out_subsurface;
			frag.write('vec3 basecol = $base;');
			frag.write('float roughness = $rough;');
			frag.write('float metallic = $met;');
			frag.write('float occlusion = $occ;');
			frag.write('vec3 nortan = $nortan;');
			frag.write('float height = $height;');
			frag.write('float opacity = $opac;');
			if (Context.layer.material_mask == null) {
				frag.write('opacity *= brushOpacity;');
			}
			if (Context.material.paintEmis) {
				frag.write('float emis = $emis;');
			}
			if (Context.material.paintSubs) {
				frag.write('float subs = $subs;');
			}
			if (height != '0' && !heightUsed) {
				heightUsed = true;
				// Height used for the first time, also rebuild vertex shader
				return make_paint(data, matcon);
			}
			if (emis != '0') emisUsed = true;
			if (subs != '0') subsUsed = true;
		}

		if (Context.tool == ToolDecal) {
			frag.add_uniform('sampler2D texdecalmask', '_texdecalmask');
			frag.write('opacity *= textureLod(texdecalmask, texCoord, 0.0).r;');
		}
		else if (Context.tool == ToolText) {
			frag.add_uniform('sampler2D textexttool', '_textexttool');
			frag.write('opacity *= textureLod(textexttool, texCoord, 0.0).r;');
		}

		if (UITrait.inst.brushMaskImage != null && Context.tool == ToolBrush) {
			frag.add_uniform('sampler2D texbrushmask', '_texbrushmask');
			frag.write('vec2 binp_mask = inp.xy * 2.0 - 1.0;');
			frag.write('binp_mask.x *= aspectRatio;');
			frag.write('binp_mask = binp_mask * 0.5 + 0.5;');
			frag.write('vec2 pa_mask = bsp.xy - binp_mask.xy;');
			frag.write('pa_mask /= brushRadius;');
			if (UITrait.inst.brush3d) {
				frag.add_uniform('vec3 eye', '_cameraPosition');
				frag.write('pa_mask *= distance(eye, winp.xyz) / 1.5;');
			}
			frag.write('pa_mask = pa_mask.xy * 0.5 + 0.5;');
			frag.write('opacity *= textureLod(texbrushmask, pa_mask, 0).r;');
		}

		if (Context.tool == ToolParticle) { // particle mask
			frag.add_uniform('sampler2D texparticle', '_texparticle');
			#if (kha_opengl || kha_webgl)
			frag.write('float str = textureLod(texparticle, vec2(sp.x, (1.0 - sp.y)), 0).r;');
			#else
			frag.write('float str = textureLod(texparticle, sp.xy, 0).r;');
			#end
		}
		else { // brush cursor mask
			frag.write('float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');
		}

		// Manual blending to preserve memory
		frag.wvpposition = true;
		frag.write('vec2 sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		#if (kha_direct3d11 || kha_direct3d12)
		frag.write('sample_tc.y = 1.0 - sample_tc.y;');
		#end
		frag.add_uniform('sampler2D paintmask');
		frag.write('float sample_mask = textureLod(paintmask, sample_tc, 0.0).r;');
		frag.write('str = max(str, sample_mask);');

		frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
		frag.write('vec4 sample_undo = textureLod(texpaint_undo, sample_tc, 0.0);');

		var matid = Context.material.id / 255;
		if (UITrait.inst.pickerMaskHandle.position == 1) { // material id mask
			matid = UITrait.inst.materialIdPicked / 255; // Keep existing material id in place when mask is set
		}
		frag.write('float matid = $matid;');

		// TODO: Use emission/subsurface matid
		// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
		if (Context.material.paintSubs) {
			frag.write('if (subs > 0) { matid = 254 / 255; }');
		}
		if (Context.material.paintEmis) {
			frag.write('if (emis > 0) { matid = 255 / 255; }');
		}

		if (layered) {
			if (eraser) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, vec3(0.0, 0.0, 0.0), str), sample_undo.a - str);');
				frag.write('nortan = vec3(0.5, 0.5, 1.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 0.0;');
				frag.write('metallic = 0.0;');
				frag.write('matid = 0.0;');
			}
			else if (decal) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, basecol, str), max(str, sample_undo.a));');
			}
			else {
				frag.write('fragColor[0] = vec4(' + blendMode(frag, UITrait.inst.brushBlending, 'sample_undo.rgb', 'basecol', 'opacity') + ', max(str, sample_undo.a));');

			}
			frag.write('fragColor[1] = vec4(nortan, matid);');

			var height = '0.0';
			if (Context.material.paintHeight && heightUsed) {
				height = 'height';
			}

			if (decal) {
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');
				frag.write('fragColor[2] = mix(sample_pack_undo, vec4(occlusion, roughness, metallic, $height), str);');
			}
			else {
				frag.write('fragColor[2] = vec4(occlusion, roughness, metallic, $height);');
			}
		}
		else {
			if (eraser) {
				frag.write('fragColor[0] = vec4(mix(sample_undo.rgb, vec3(${Layers.defaultBase}, ${Layers.defaultBase}, ${Layers.defaultBase}), str), sample_undo.a - str);');
				frag.write('fragColor[1] = vec4(0.5, 0.5, 1.0, 0.0);');
				frag.write('fragColor[2] = vec4(1.0, ${Layers.defaultRough}, 0.0, 0.0);');
			}
			else {
				frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_nor_undo = textureLod(texpaint_nor_undo, sample_tc, 0.0);');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');
				frag.write('fragColor[0] = vec4(' + blendMode(frag, UITrait.inst.brushBlending, 'sample_undo.rgb', 'basecol', 'str') + ', 0.0);');
				frag.write('fragColor[1] = vec4(mix(sample_nor_undo.rgb, nortan, str), matid);');
				if (Context.material.paintHeight && heightUsed) {
					frag.write('fragColor[2] = mix(sample_pack_undo, vec4(occlusion, roughness, metallic, height), str);');
				}
				else {
					frag.write('fragColor[2] = vec4(mix(sample_pack_undo.rgb, vec3(occlusion, roughness, metallic), str), 0.0);');
				}
			}
		}
		frag.write('fragColor[3] = vec4(str, 0.0, 0.0, 1.0);');

		if (!Context.material.paintBase) {
			con_paint.data.color_writes_red[0] = false;
			con_paint.data.color_writes_green[0] = false;
			con_paint.data.color_writes_blue[0] = false;
		}
		if (!Context.material.paintNor) {
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
		}
		if (!Context.material.paintOcc) {
			con_paint.data.color_writes_red[2] = false;
		}
		if (!Context.material.paintRough) {
			con_paint.data.color_writes_green[2] = false;
		}
		if (!Context.material.paintMet) {
			con_paint.data.color_writes_blue[2] = false;
		}
		if (!Context.material.paintHeight) {
			con_paint.data.color_writes_alpha[2] = false;
		}

		// Base color only as mask
		var isMask = Context.layerIsMask;
		if (isMask) {
			// TODO: Apply opacity into base
			// frag.write('fragColor[0].rgb *= fragColor[0].a;');
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
			con_paint.data.color_writes_red[2] = false;
			con_paint.data.color_writes_green[2] = false;
			con_paint.data.color_writes_blue[2] = false;
			con_paint.data.color_writes_alpha[2] = false;
		}

		if (Context.tool == ToolBake) {
			if (UITrait.inst.bakeType == 0) { // AO
				// Apply normal channel
				frag.wposition = true;
				frag.n = true;
				frag.vVec = true;
				frag.add_function(MaterialFunctions.str_cotangentFrame);
				#if (kha_direct3d11 || kha_direct3d12)
				frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
				#else
				frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
				#end
				frag.write('n = nortan * 2.0 - 1.0;');
				frag.write('n.y = -n.y;');
				frag.write('n = normalize(mul(n, TBN));');

				frag.write(voxelgiHalfExtents());
				frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
				frag.add_uniform('sampler3D voxels');
				frag.add_function(MaterialFunctions.str_traceAO);
				frag.n = true;
				var strength = UITrait.inst.bakeAoStrength;
				var radius = UITrait.inst.bakeAoRadius;
				var offset = UITrait.inst.bakeAoOffset;
				frag.write('float ao = traceAO(voxpos, n, $radius, $offset) * $strength;');
				if (UITrait.inst.bakeAxis > 0) {
					var axis = axisString(UITrait.inst.bakeAxis);
					frag.write('ao *= dot(n, $axis);');
				}
				frag.write('ao = 1.0 - ao;');
				frag.write('fragColor[0] = vec4(ao, ao, ao, 1.0);');
			}
			else if (UITrait.inst.bakeType == 1) { // Curvature
				var strength = UITrait.inst.bakeCurvStrength * 2.0;
				var radius = (1.0 / UITrait.inst.bakeCurvRadius) * 0.25;
				var offset = UITrait.inst.bakeCurvOffset / 10;
				frag.n = true;
				frag.write('vec3 dx = dFdx(n);');
				frag.write('vec3 dy = dFdy(n);');
				frag.write('float curvature = max(dot(dx, dx), dot(dy, dy));');
				frag.write('curvature = clamp(pow(curvature, $radius) * $strength + $offset, 0.0, 1.0);');
				if (UITrait.inst.bakeAxis > 0) {
					var axis = axisString(UITrait.inst.bakeAxis);
					frag.write('curvature *= dot(n, $axis);');
				}
				frag.write('fragColor[0] = vec4(curvature, curvature, curvature, 1.0);');
			}
			else if (UITrait.inst.bakeType == 2) { // Normal (Tangent)
				frag.n = true;
				frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
				frag.write('vec3 n0 = textureLod(texpaint_undo, texCoord, 0.0).rgb * vec3(2.0, 2.0, 2.0) - vec3(1.0, 1.0, 1.0);');
				frag.add_function(MaterialFunctions.str_cotangentFrame);
				frag.write('mat3 invTBN = transpose(cotangentFrame(n, n, texCoord));');
				frag.write('vec3 res = normalize(mul(n0, invTBN)) * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5);');
				frag.write('fragColor[0] = vec4(res, 1.0);');
			}
			else if (UITrait.inst.bakeType == 3) { // Normal (World)
				frag.n = true;
				frag.write('fragColor[0] = vec4(n * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			}
			else if (UITrait.inst.bakeType == 4) { // Position
				frag.wposition = true;
				frag.write('fragColor[0] = vec4(wposition * vec3(0.5, 0.5, 0.5) + vec3(0.5, 0.5, 0.5), 1.0);');
			}
			else if (UITrait.inst.bakeType == 5) { // TexCoord
				frag.write('fragColor[0] = vec4(texCoord.xy, 0.0, 1.0);');
			}
			else if (UITrait.inst.bakeType == 6) { // Material ID
				frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
				frag.write('float sample_matid = textureLod(texpaint_nor_undo, texCoord, 0.0).a;');
				frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 20.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[0] = vec4(matid_r, matid_g, matid_b, 1.0);');
			}
			else if (UITrait.inst.bakeType == 7) { // Object ID
				frag.add_uniform('float objectId', '_objectId');
				frag.write('float id_r = fract(sin(dot(vec2(objectId, objectId * 20.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_g = fract(sin(dot(vec2(objectId * 20.0, objectId), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float id_b = fract(sin(dot(vec2(objectId, objectId * 40.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[0] = vec4(id_r, id_g, id_b, 1.0);');
			}
		}

		Material.finalize(con_paint);
		Material.triplanar = false;
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}

	static function blendMode(frag:MaterialShader, blending:Int, cola:String, colb:String, opac:String):String {
		if (blending == 0) { // Mix
			return 'mix($cola, $colb, $opac)';
		}
		else if (blending == 1) { // Darken
			return 'mix($cola, min($cola, $colb), $opac)';
		}
		else if (blending == 2) { // Multiply
			return 'mix($cola, $cola * $colb, $opac)';
		}
		else if (blending == 3) { // Burn
			return 'mix($cola, vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $cola) / $colb, $opac)';
		}
		else if (blending == 4) { // Lighten
			return 'max($cola, $colb * $opac)';
		}
		else if (blending == 5) { // Screen
			return '(vec3(1.0, 1.0, 1.0) - (vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) + $opac * (vec3(1.0, 1.0, 1.0) - $colb)) * (vec3(1.0, 1.0, 1.0) - $cola))';
		}
		else if (blending == 6) { // Dodge
			return 'mix($cola, $cola / (vec3(1.0, 1.0, 1.0) - $colb), $opac)';
		}
		else if (blending == 7) { // Add
			return 'mix($cola, $cola + $colb, $opac)';
		}
		else if (blending == 8) { // Overlay
			#if (kha_direct3d11 || kha_direct3d12)
			return 'mix($cola, ($cola < vec3(0.5, 0.5, 0.5) ? vec3(2.0, 2.0, 2.0) * $cola * $colb : vec3(1.0, 1.0, 1.0) - vec3(2.0, 2.0, 2.0) * (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola)), $opac)';
			#else
			return 'mix($cola, $colb, $opac)'; // TODO
			#end
		}
		else if (blending == 9) { // Soft Light
			return '((1.0 - $opac) * $cola + $opac * ((vec3(1.0, 1.0, 1.0) - $cola) * $colb * $cola + $cola * (vec3(1.0, 1.0, 1.0) - (vec3(1.0, 1.0, 1.0) - $colb) * (vec3(1.0, 1.0, 1.0) - $cola))))';
		}
		else if (blending == 10) { // Linear Light
			return '($cola + $opac * (vec3(2.0, 2.0, 2.0) * ($colb - vec3(0.5, 0.5, 0.5))))';
		}
		else if (blending == 11) { // Difference
			return 'mix($cola, abs($cola - $colb), $opac)';
		}
		else if (blending == 12) { // Subtract
			return 'mix($cola, $cola - $colb, $opac)';
		}
		else if (blending == 13) { // Divide
			return 'vec3(1.0 - $opac, 1.0 - $opac, 1.0 - $opac) * $cola + vec3($opac, $opac, $opac) * $cola / $colb';
		}
		else if (blending == 14) { // Hue
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($cola).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == 15) { // Saturation
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else if (blending == 16) { // Color
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($colb).r, rgb_to_hsv($colb).g, rgb_to_hsv($cola).b)), $opac)';
		}
		else { // Value
			frag.add_function(MaterialFunctions.str_hue_sat);
			return 'mix($cola, hsv_to_rgb(vec3(rgb_to_hsv($cola).r, rgb_to_hsv($cola).g, rgb_to_hsv($colb).b)), $opac)';
		}
	}

	static function axisString(i:Int):String {
		return i == 1 ? "vec3(1,0,0)" :
			   i == 2 ? "vec3(0,1,0)" :
			   i == 3 ? "vec3(0,0,1)" :
			   i == 4 ? "vec3(-1,0,0)" :
			   i == 5 ? "vec3(0,-1,0)" :
			   			"vec3(0,0,-1)";
	}

	public static function make_mesh_preview(data:MaterialShaderData, matcon:TMaterialContext):MaterialShaderContext {
		var isScene = UITrait.inst.worktab.position == SpaceScene;
		var context_id = 'mesh';
		var con_mesh:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: (Config.raw.rp_culling || !isScene) ? 'clockwise' : 'none',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		frag.ins = vert.outs;

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write_attrib('gl_Position = mul(vec4(pos.xyz, 1.0), WVP);');

		vert.add_out('vec2 texCoord');
		vert.add_uniform('float brushScale', '_brushScale');
		vert.write_attrib('texCoord = tex * brushScale;');

		if (heightUsed) {
			frag.bposition = true;
		}

		Material.parse_height = heightUsed;
		var sout = Material.parse(UINodes.inst.canvas, con_mesh, vert, frag, null, null, null, matcon);
		Material.parse_height = false;
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var opac = sout.out_opacity;
		var nortan = Material.out_normaltan;
		frag.write('vec3 basecol = pow($base, vec3(2.2, 2.2, 2.2));');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('float opacity = $opac;');
		frag.write('vec3 nortan = $nortan;');

		var decal = UITrait.inst.decalPreview;
		if (decal) {
			if (Context.tool == ToolDecal) {
				frag.add_uniform('sampler2D texdecalmask', '_texdecalmask');
				frag.write('opacity *= textureLod(texdecalmask, texCoord, 0.0).r;');
			}
			else if (Context.tool == ToolText) {
				frag.add_uniform('sampler2D textexttool', '_textexttool');
				frag.write('opacity *= textureLod(textexttool, texCoord, 0.0).r;');
			}
		}
		if (decal || isScene) {
			var opac = isScene ? opacityDiscardScene : opacityDiscardDecal;
			frag.write('if (opacity < $opac) discard;');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;

		frag.add_function(MaterialFunctions.str_packFloatInt16);
		frag.add_function(MaterialFunctions.str_packFloat2);
		frag.add_function(MaterialFunctions.str_cotangentFrame);
		frag.add_function(MaterialFunctions.str_octahedronWrap);

		// Apply normal channel
		if (decal) {
			// TODO
		}
		else {
			frag.vVec = true;
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');
		}

		frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
		frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
		// uint matid = 0;
		frag.write('fragColor[0] = vec4(n.x, n.y, roughness, packFloatInt16(metallic, uint(0)));'); // metallic/matid
		frag.write('fragColor[1] = vec4(basecol.r, basecol.g, basecol.b, packFloat2(occlusion, 1.0));'); // occ/spec
		frag.write('fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);'); // veloc

		Material.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	public static function make_particle(data:MaterialShaderData):MaterialShaderContext {
		var context_id = 'mesh';
		var con_part:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always',
			cull_mode: 'clockwise',
			vertex_elements: [{name: "pos", data: 'short4norm'}]
		});

		var vert = con_part.make_vert();
		var frag = con_part.make_frag();
		frag.ins = vert.outs;

		vert.write_attrib('vec4 spos = vec4(pos.xyz, 1.0);');

		vert.add_uniform('float brushRadius', '_brushRadius');
		vert.write_attrib('vec3 emitFrom = vec3(fhash(gl_InstanceID), fhash(gl_InstanceID * 2), fhash(gl_InstanceID * 3));');
		vert.write_attrib('emitFrom = emitFrom * brushRadius - brushRadius / 2;');
		vert.write_attrib('spos.xyz += emitFrom * 256;');

		vert.add_uniform('mat4 pd', '_particleData');

		var str_tex_hash = "float fhash(float n) { return fract(sin(n) * 43758.5453); }\n";
		vert.add_function(str_tex_hash);
		vert.add_out('float p_age');
		vert.write('p_age = pd[3][3] - gl_InstanceID * pd[0][1];');
		vert.write('p_age -= p_age * fhash(gl_InstanceID) * pd[2][3];');

		vert.write('if (pd[0][0] > 0 && p_age < 0) p_age += (int(-p_age / pd[0][0]) + 1) * pd[0][0];');

		vert.add_out('float p_lifetime');
		vert.write('p_lifetime = pd[0][2];');
		vert.write('if (p_age < 0 || p_age > p_lifetime) {');
		// vert.write('    SPIRV_Cross_Output stage_output;');
		// vert.write('    stage_output.svpos /= 0.0;');
		// vert.write('    return stage_output;');
		vert.write('    spos /= 0.0;');
		vert.write('}');

		vert.add_out('vec3 p_velocity');
		vert.write('p_velocity = vec3(pd[1][0], pd[1][1], pd[1][2]);');
		vert.write('p_velocity.x += fhash(gl_InstanceID)                * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.y += fhash(gl_InstanceID +     pd[0][3]) * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.z += fhash(gl_InstanceID + 2 * pd[0][3]) * pd[1][3] - pd[1][3] / 2;');
		vert.write('p_velocity.x += (pd[2][0] * p_age) / 5;');
		vert.write('p_velocity.y += (pd[2][1] * p_age) / 5;');
		vert.write('p_velocity.z += (pd[2][2] * p_age) / 5;');

		vert.add_out('vec3 p_location');
		vert.write('p_location = p_velocity * p_age;');
		vert.write('spos.xyz += p_location;');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('gl_Position = mul(spos, WVP);');

		vert.add_uniform('vec4 inp', '_inputBrush');
		vert.write('vec2 binp = vec2(inp.x, 1.0 - inp.y);');
		vert.write('binp = binp * 2.0 - 1.0;');
		vert.write('binp *= gl_Position.w;');
		vert.write('gl_Position.xy += binp;');

		vert.add_out('float p_fade');
		vert.write('p_fade = sin(min((p_age / 8) * 3.141592, 3.141592));');

		frag.add_out('float fragColor');
		frag.write('fragColor = p_fade;');

		// vert.add_out('vec4 wvpposition');
		// vert.write('wvpposition = gl_Position;');
		// frag.write('vec2 texCoord = wvpposition.xy / wvpposition.w;');
		// frag.add_uniform('sampler2D gbufferD');
		// frag.write('fragColor *= 1.0 - clamp(distance(textureLod(gbufferD, texCoord, 0).r, wvpposition.z), 0.0, 1.0);');

		// Material.finalize(con_part);
		con_part.data.shader_from_source = true;
		con_part.data.vertex_shader = vert.get();
		con_part.data.fragment_shader = frag.get();

		return con_part;
	}

	public static function make_mesh(data:MaterialShaderData):MaterialShaderContext {
		var context_id = 'mesh';
		var con_mesh:MaterialShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: Config.raw.rp_culling ? 'clockwise' : 'none',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		var vert = con_mesh.make_vert();
		var frag = con_mesh.make_frag();

		vert.add_out('vec2 texCoord');
		frag.wvpposition = true;
		vert.add_out('vec4 prevwvpposition');
		vert.add_uniform('mat4 VP', '_viewProjectionMatrix');
		vert.add_uniform('mat4 prevWVP', '_prevWorldViewProjectionMatrix');
		vert.wposition = true;

		// Height
		// TODO: can cause TAA issues
		if (heightUsed) {
			var displaceStrength = getDisplaceStrength();
			vert.n = true;
			vert.write('float height = 0.0;');
			var numLayers = 0;
			for (l in Project.layers) {
				if (!l.visible) continue;
				if (numLayers > 16) break;
				numLayers++;
				vert.add_uniform('sampler2D texpaint_pack_vert' + l.id, '_texpaint_pack_vert' + l.id);
				vert.write('height += textureLod(texpaint_pack_vert' + l.id + ', tex, 0.0).a;');
			}
			vert.write('wposition += wnormal * vec3(height, height, height) * vec3($displaceStrength, $displaceStrength, $displaceStrength);');
		}
		//

		vert.write('gl_Position = mul(vec4(wposition.xyz, 1.0), VP);');
		vert.write('texCoord = tex;');
		if (heightUsed) {
			vert.add_uniform('mat4 invW', '_inverseWorldMatrix');
			vert.write('prevwvpposition = mul(mul(vec4(wposition, 1.0), invW), prevWVP);');
		}
		else {
			vert.write('prevwvpposition = mul(vec4(pos.xyz, 1.0), prevWVP);');
		}

		frag.ins = vert.outs;

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;
		frag.add_function(MaterialFunctions.str_packFloatInt16);
		frag.add_function(MaterialFunctions.str_packFloat2);

		if (Context.tool == ToolColorId) {
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));'); // met/rough
			frag.write('vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
		}
		else {
			frag.add_function(MaterialFunctions.str_octahedronWrap);

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float specular;');
			frag.write('float matid = 0.0;');

			frag.vVec = true;
			frag.add_function(MaterialFunctions.str_cotangentFrame);
			#if (kha_direct3d11 || kha_direct3d12)
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			if (Project.layers[0].visible) {

				if (Context.layer.paintBase) {
					frag.add_shared_sampler('sampler2D texpaint');
					frag.write('basecol = textureLodShared(texpaint, texCoord, 0.0).rgb;');
				}
				else {
					frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				}

				if (Context.layer.paintNor ||
					emisUsed) {
					frag.add_shared_sampler('sampler2D texpaint_nor');
					frag.write('vec4 texpaint_nor_sample = textureLodShared(texpaint_nor, texCoord, 0.0);');

					if (emisUsed) {
						frag.write('matid = texpaint_nor_sample.a;');
					}

					if (Context.layer.paintNor) {
						frag.write('vec3 ntex = texpaint_nor_sample.rgb;');
						frag.write('n = ntex * 2.0 - 1.0;');
						frag.write('n.y = -n.y;');
						frag.write('n = normalize(mul(n, TBN));');
					}
				}

				if (heightUsed ||
					Context.layer.paintOcc ||
					Context.layer.paintRough ||
					Context.layer.paintMet) {
					frag.add_shared_sampler('sampler2D texpaint_pack');
					frag.write('vec4 pack = textureLodShared(texpaint_pack, texCoord, 0.0);');
				}

				// Height
				if (heightUsed) {
					var ds = getDisplaceStrength() * 5;
					if (ds < 0.1) ds = 0.1;
					else if (ds > 2.0) ds = 2.0;
					frag.wposition = true;
					frag.write('float3 dpdx = dFdx(wposition);');
					frag.write('float3 dpdy = dFdy(wposition);');
					frag.write('float dhdx = dFdx(pack.a * $ds);');
					frag.write('float dhdy = dFdy(pack.a * $ds);');
					frag.write('float3 cross_x = cross(n, dpdx);');
					frag.write('float3 cross_y = cross(dpdy, n);');
					frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
					frag.write('n = normalize(n - ngrad);');

					// frag.add_uniform('float texpaintSize', '_texpaintSize');
					// frag.write('float tex_step = 1.0 / texpaintSize;');
					// frag.wposition = true;
					// frag.write('float pack_a = textureLodShared(texpaint_pack, vec2(texCoord.x + tex_step, texCoord.y), 0.0).a;');
					// frag.write('float pack_b = textureLodShared(texpaint_pack, vec2(texCoord.x - tex_step, texCoord.y), 0.0).a;');
					// frag.write('float pack_c = textureLodShared(texpaint_pack, vec2(texCoord.x, texCoord.y + tex_step), 0.0).a;');
					// frag.write('float pack_d = textureLodShared(texpaint_pack, vec2(texCoord.x, texCoord.y - tex_step), 0.0).a;');
					// frag.write('float3 dpdx = dFdx(wposition);');
					// frag.write('float3 dpdy = dFdy(wposition);');
					// frag.write('float dhdx = pack_a - pack_b;');
					// frag.write('float dhdy = pack_c - pack_d;');
					// frag.write('float3 cross_x = cross(n, dpdx);');
					// frag.write('float3 cross_y = cross(dpdy, n);');
					// frag.write('vec3 ngrad = (cross_y * dhdx + cross_x * dhdy) / dot(dpdx, cross_y);');
					// frag.write('n = normalize(n - ngrad);');
				}
				//

				if (Context.layer.paintOcc) {
					frag.write('occlusion = pack.r;');
				}
				else {
					frag.write('occlusion = 1.0;');
				}
				if (Context.layer.paintRough) {
					frag.write('roughness = pack.g;');
				}
				else {
					frag.write('roughness = 1.0;');
				}
				if (Context.layer.paintMet) {
					frag.write('metallic = pack.b;');
				}
				else {
					frag.write('metallic = 0.0;');
				}

				var l = Project.layers[0];
				if (l.maskOpacity < 1) {
					frag.write('basecol *= ${l.maskOpacity};');
					frag.write('occlusion *= ${l.maskOpacity};');
					frag.write('roughness *= ${l.maskOpacity};');
					frag.write('metallic *= ${l.maskOpacity};');
				}
			}
			else {
				frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				frag.write('occlusion = 1.0;');
				frag.write('roughness = 1.0;');
				frag.write('metallic = 0.0;');
				frag.write('specular = 1.0;');
			}

			if (Project.layers.length > 1) {
				frag.write('float factor0;');
				frag.write('vec4 col_tex0;');
				frag.write('vec4 col_nor0;');
				frag.write('vec4 col_pack0;');
				frag.write('vec3 n0;');
				var len = Project.layers.length;
				var start = len - 1;
				var maxLayers = getMaxVisibleLayers();
				var count = 1;
				for (i in 1...len) {
					if (start == 1) break;
					start--;
					var l = Project.layers[len - i];
					if (l.visible) {
						count++;
						if (count >= maxLayers) break;
					}
				}
				for (i in start...len) {
					var l = Project.layers[i];
					if (!l.visible) continue;
					var id = l.id;

					if (l.objectMask > 0) {
						var uid = Project.paintObjects[l.objectMask - 1].uid;
						frag.add_uniform('int objectId', '_uid');
						frag.write('if ($uid == objectId) {');
					}

					frag.add_shared_sampler('sampler2D texpaint' + id);
					frag.write('col_tex0 = textureLodShared(texpaint' + id + ', texCoord, 0.0);');
					frag.write('factor0 = col_tex0.a;');

					if (l.texpaint_mask != null) {
						frag.add_shared_sampler('sampler2D texpaint_mask' + id);
						frag.write('factor0 *= textureLodShared(texpaint_mask' + id + ', texCoord, 0.0).r;');
					}
					if (l.maskOpacity < 1) {
						frag.write('factor0 *= ${l.maskOpacity};');
					}

					if (Context.layer.paintBase) {
						frag.write('basecol = ' + blendMode(frag, l.blending, 'basecol', 'col_tex0.rgb', 'factor0') + ';');
					}

					if (emisUsed ||
						Context.layer.paintNor) {

						frag.add_shared_sampler('sampler2D texpaint_nor' + id);
						frag.write('col_nor0 = textureLodShared(texpaint_nor' + id + ', texCoord, 0.0);');

						if (emisUsed) {
							frag.write('matid = col_nor0.a;');
						}

						if (Context.layer.paintNor) {
							frag.write('n0 = col_nor0.rgb * 2.0 - 1.0;');
							frag.write('n0.y = -n0.y;');
							frag.write('n0 = normalize(mul(n0, TBN));');
							frag.write('n = normalize(mix(n, n0, factor0));');
						}
					}

					if (Context.layer.paintOcc ||
						Context.layer.paintRough ||
						Context.layer.paintMet) {
						frag.add_shared_sampler('sampler2D texpaint_pack' + id);
						frag.write('col_pack0 = textureLodShared(texpaint_pack' + id + ', texCoord, 0.0);');

						if (Context.layer.paintOcc) {
							frag.write('occlusion = mix(occlusion, col_pack0.r, factor0);');
						}
						if (Context.layer.paintRough) {
							frag.write('roughness = mix(roughness, col_pack0.g, factor0);');
						}
						if (Context.layer.paintMet) {
							frag.write('metallic = mix(metallic, col_pack0.b, factor0);');
						}
					}

					if (l.objectMask > 0) {
						frag.write('}');
					}
				}
			}

			if (UITrait.inst.drawTexels) {
				frag.add_uniform('float texpaintSize', '_texpaintSize');
				frag.write('vec2 texel = texCoord * texpaintSize;');
				frag.write('basecol *= max(float(mod(int(texel.x), 2.0) == mod(int(texel.y), 2.0)), 0.9);');
			}

			if (UITrait.inst.drawWireframe) {
				// GL_NV_fragment_shader_barycentric
				// VK_AMD_shader_explicit_vertex_parameter
				frag.add_uniform('sampler2D texuvmap', '_texuvmap');
				frag.write('basecol *= 1.0 - textureLod(texuvmap, texCoord, 0.0).r;');
				// frag.write('if (basecol == vec3(0,0,0)) discard;');
			}

			frag.write('n /= (abs(n.x) + abs(n.y) + abs(n.z));');
			frag.write('n.xy = n.z >= 0.0 ? n.xy : octahedronWrap(n.xy);');
			frag.write('basecol = pow(basecol, vec3(2.2, 2.2, 2.2));');
			if (UITrait.inst.viewportMode == 0) { // Render
				if (emisUsed) {
					frag.write('if (matid == 1.0) basecol *= 10.0;'); // Boost for bloom
				}
				frag.write('fragColor[0] = vec4(n.xy, roughness, packFloatInt16(metallic, uint(matid)));');
				frag.write('fragColor[1] = vec4(basecol, packFloat2(occlusion, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 1) { // Basecol
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(basecol, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 2) { // Normal Map
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(ntex.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 3) { // Occ
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 4) { // Rough
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 5) { // Met
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 6) { // Texcoord
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(texCoord, 0.0, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 7) { // Normal
				frag.nAttr = true;
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(nAttr, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 8) { // MaterialID
				frag.write('float sample_matid = textureLodShared(texpaint_nor, texCoord, 0.0).a;');
				frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 2.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 2.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 4.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('fragColor[1] = vec4(matid_r, matid_g, matid_b, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 9) { // Mask
				frag.write('fragColor[0] = vec4(n.xy, 1.0, packFloatInt16(0.0, uint(0)));');
				frag.write('float sample_mask = 1.0;');
				if (Context.layer.texpaint_mask != null) {
					frag.add_uniform('sampler2D texpaint_mask_view', '_texpaint_mask');
					frag.write('sample_mask = textureLod(texpaint_mask_view, texCoord, 0.0).r;');
				}
				frag.write('fragColor[1] = vec4(sample_mask, sample_mask, sample_mask, packFloat2(1.0, 1.0));'); // occ/spec
			}
		}

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		frag.write('fragColor[2] = vec4(posa - posb, texCoord.xy);');

		Material.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();
		return con_mesh;
	}

	static function getMaxVisibleLayers():Int {
		#if (kha_direct3d11 || kha_direct3d12)
		// 128 texture slots available
		// 4 textures per layer (3 + 1 mask)
		// 32 layers - base + 31 on top
		return 31;
		#else
		// 32 texture slots available
		return 4; // base + 4 on top
		#end
	}

	#if rp_voxelao
	public static function make_voxel(data:iron.data.ShaderData.ShaderContext) {
		var structure = new kha.graphics4.VertexStructure();
		structure.add("pos", kha.graphics4.VertexData.Short4Norm);
		structure.add("nor", kha.graphics4.VertexData.Short2Norm);
		structure.add("tex", kha.graphics4.VertexData.Short2Norm);

		var pipeState = data.pipeState;
		pipeState.inputLayout = [structure];
		data.raw.vertex_elements = [{name: "pos", data: 'short4norm'}, {name: "nor", data: 'short2norm'}, {name: "tex", data: 'short2norm'}];

		var ds = getDisplaceStrength();
		pipeState.vertexShader = kha.graphics4.VertexShader.fromSource(
		#if (kha_direct3d11 || kha_direct3d12)
		"#define vec3 float3
		uniform float4x4 W;
		uniform float3x3 N;
		Texture2D<float4> texpaint_pack;
		SamplerState _texpaint_pack_sampler;
		struct SPIRV_Cross_Input { float4 pos : TEXCOORD1; float2 nor : TEXCOORD0; float2 tex : TEXCOORD2; };
		struct SPIRV_Cross_Output { float4 svpos : SV_POSITION; };
		SPIRV_Cross_Output main(SPIRV_Cross_Input stage_input) {
			SPIRV_Cross_Output stage_output;
			" + voxelgiHalfExtents() + "
			stage_output.svpos.xyz = mul(float4(stage_input.pos.xyz, 1.0), W).xyz / voxelgiHalfExtents.xxx;
			float3 wnormal = normalize(mul(float3(stage_input.nor.xy, stage_input.pos.w), N));
			float height = texpaint_pack.SampleLevel(_texpaint_pack_sampler, stage_input.tex, 0.0).a;
			stage_output.svpos.xyz += wnormal * height.xxx * float3(" + ds + "," + ds + "," + ds + ");
			stage_output.svpos.w = 1.0;
			return stage_output;
		}"
		#else
		"#version 450
		in vec4 pos;
		in vec2 nor;
		in vec2 tex;
		out vec3 voxpositionGeom;
		uniform mat4 W;
		uniform mat3 N;
		uniform sampler2D texpaint_pack;
		void main() {
			" + voxelgiHalfExtents() + "
			voxpositionGeom = vec3(W * vec4(pos.xyz, 1.0)) / voxelgiHalfExtents;
			vec3 wnormal = normalize(N * vec3(nor.xy, pos.w));
			float height = textureLod(texpaint_pack, tex, 0.0).a;
			voxpositionGeom += wnormal * vec3(height) * vec3(" + ds + ");
		}"
		#end
		);

		pipeState.compile();
		data.raw.constants = [{ name: "W", type: "mat4", link: "_worldMatrix" }, { name: "N", type: "mat3", link: "_normalMatrix" }];
		data.constants = [pipeState.getConstantLocation("W"), pipeState.getConstantLocation("N")];
		data.raw.texture_units = [{name: "texpaint_pack"}, {name: "voxels", is_image: true}];
		data.textureUnits = [pipeState.getTextureUnit("texpaint_pack"), pipeState.getTextureUnit("voxels")];
	}
	#end

	static inline function getDisplaceStrength():Float {
		var sc = Context.mainObject().transform.scale.x;
		return UITrait.inst.displaceStrength * 0.02 * sc;
	}

	static inline function voxelgiHalfExtents():String {
		var ext = UITrait.inst.vxaoExt;
		return 'const vec3 voxelgiHalfExtents = vec3($ext, $ext, $ext);';
	}
}
