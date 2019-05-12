package arm;

import armory.system.Cycles;
import armory.system.CyclesShader;
import armory.system.CyclesShader.CyclesShaderContext;
import iron.data.SceneFormat;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.Tool;

class MaterialBuilder {

	static var heightUsed = false;
	static var emisUsed = false;
	static var subsUsed = false;

	public static function make_paint(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var layered = UITrait.inst.selectedLayer != UITrait.inst.layers[0];
		var eraser = UITrait.inst.selectedTool == ToolEraser;
		var context_id = 'paint';
		var con_paint:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: false,
			compare_mode: 'always', // TODO: align texcoords winding order
			// cull_mode: 'counter_clockwise',
			cull_mode: 'none',
			vertex_elements: [{name: "pos", data: 'short4norm'},{name: "nor", data: 'short2norm'},{name: "tex", data: 'short2norm'}] });

		con_paint.data.color_writes_red = [true, true, true, true];
		con_paint.data.color_writes_green = [true, true, true, true];
		con_paint.data.color_writes_blue = [true, true, true, true];
		con_paint.data.color_writes_alpha = [true, true, true, true];

		if (UITrait.inst.selectedTool == ToolBake) {
			con_paint.data.color_writes_red[0] = false;
			con_paint.data.color_writes_green[0] = false;
			con_paint.data.color_writes_blue[0] = false;
			con_paint.data.color_writes_alpha[0] = false;
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
			con_paint.data.color_writes_alpha[1] = false;
			con_paint.data.color_writes_green[2] = false; // No rough
			con_paint.data.color_writes_blue[2] = false; // No met
			con_paint.data.color_writes_alpha[2] = false;
		}

		var vert = con_paint.make_vert();
		var frag = con_paint.make_frag();
		frag.ins = vert.outs;

		if (UITrait.inst.selectedTool == ToolColorId || UITrait.inst.selectedTool == ToolPicker) {
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

			if (UITrait.inst.selectedTool == ToolColorId) {
				frag.add_out('vec4 fragColor');
				frag.add_uniform('sampler2D texcolorid', '_texcolorid');
				frag.write('vec3 idcol = textureLod(texcolorid, texCoordInp, 0.0).rgb;');
				frag.write('fragColor = vec4(idcol, 1.0);');
			}
			else if (UITrait.inst.selectedTool == ToolPicker) {
				frag.add_out('vec4 fragColor[3]');
				frag.add_uniform('sampler2D texpaint');
				frag.add_uniform('sampler2D texpaint_nor');
				frag.add_uniform('sampler2D texpaint_pack');
				frag.write('fragColor[0] = textureLod(texpaint, texCoordInp, 0.0);');
				frag.write('fragColor[1] = textureLod(texpaint_nor, texCoordInp, 0.0);');
				frag.write('fragColor[2] = textureLod(texpaint_pack, texCoordInp, 0.0);');
			}

			con_paint.data.shader_from_source = true;
			con_paint.data.vertex_shader = vert.get();
			con_paint.data.fragment_shader = frag.get();
			return con_paint;
		}

		#if kha_direct3d11
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, (1.0 - tex.y) * 2.0 - 1.0);');
		#else
		vert.write('vec2 tpos = vec2(tex.x * 2.0 - 1.0, tex.y * 2.0 - 1.0);');
		#end

		var faceFill = UITrait.inst.selectedTool == ToolFill && UITrait.inst.fillTypeHandle.position == 1;
		var decal = UITrait.inst.selectedTool == ToolDecal || UITrait.inst.selectedTool == ToolText;

		if (!faceFill && !decal) {
			// Fix seams at uv borders
			vert.add_uniform('vec2 sub', '_sub');
			vert.write('tpos += sub;');
		}

		vert.write('gl_Position = vec4(tpos, 0.0, 1.0);');

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.add_out('vec4 ndc');
		vert.write_attrib('ndc = mul(vec4(pos.xyz, 1.0), WVP);');

		frag.write_attrib('vec3 sp = vec3((ndc.xyz / ndc.w) * 0.5 + 0.5);');
		frag.write_attrib('sp.y = 1.0 - sp.y;');
		frag.add_uniform('float paintDepthBias', '_paintDepthBias');
		frag.write_attrib('sp.z -= paintDepthBias;'); // small bias or xray

		if (UITrait.inst.brushPaint == 1) frag.ndcpos = true; // Project

		if (UITrait.inst.selectedTool == ToolBake) {
			frag.wposition = true;
			frag.n = true;
		}

		frag.add_uniform('vec4 inp', '_inputBrush');
		frag.add_uniform('vec4 inplast', '_inputBrushLast');
		frag.add_uniform('float aspectRatio', '_aspectRatioWindowF');
		frag.write('vec2 bsp = sp.xy * 2.0 - 1.0;');
		frag.write('bsp.x *= aspectRatio;');
		frag.write('bsp = bsp * 0.5 + 0.5;');

		frag.add_uniform('sampler2D paintdb');

		frag.add_out('vec4 fragColor[4]');

		frag.add_uniform('float brushRadius', '_brushRadius');
		frag.add_uniform('float brushOpacity', '_brushOpacity');
		frag.add_uniform('float brushHardness', '_brushHardness');

		if (UITrait.inst.selectedTool == ToolBrush  ||
			UITrait.inst.selectedTool == ToolEraser ||
			UITrait.inst.selectedTool == ToolClone  ||
			UITrait.inst.selectedTool == ToolBlur   ||
			UITrait.inst.selectedTool == ToolParticle ||
			decal) {
			
			#if (kha_opengl || kha_webgl)
			frag.write('if (sp.z > textureLod(paintdb, vec2(sp.x, 1.0 - sp.y), 0.0).r) { discard; }');
			#else
			frag.write('if (sp.z > textureLod(paintdb, vec2(sp.x, sp.y), 0.0).r) { discard; }');
			#end

			if (decal || UITrait.inst.selectedTool == ToolParticle) {
				frag.write('float dist = 0.0;');
			}
			else {
				frag.write('vec2 binp = inp.xy * 2.0 - 1.0;');
				frag.write('binp.x *= aspectRatio;');
				frag.write('binp = binp * 0.5 + 0.5;');

				// Continuos paint
				frag.write('vec2 binplast = inplast.xy * 2.0 - 1.0;');
				frag.write('binplast.x *= aspectRatio;');
				frag.write('binplast = binplast * 0.5 + 0.5;');
				
				frag.write('vec2 pa = bsp.xy - binp.xy, ba = binplast.xy - binp.xy;');
				frag.write('float h = clamp(dot(pa, ba) / dot(ba, ba), 0.0, 1.0);');
				frag.write('float dist = length(pa - ba * h);');
				
				if (!UITrait.inst.mirrorX) {
					frag.write('if (dist > brushRadius) { discard; }');
				}
				else {
					frag.write('vec2 binp2 = vec2(0.5 - (binp.x - 0.5), binp.y), binplast2 = vec2(0.5 - (binplast.x - 0.5), binplast.y);');
					frag.write('vec2 pa2 = bsp.xy - binp2.xy, ba2 = binplast2.xy - binp2.xy;');
					frag.write('float h2 = clamp(dot(pa2, ba2) / dot(ba2, ba2), 0.0, 1.0);');
					frag.write('float dist2 = length(pa2 - ba2 * h2);');
					frag.write('if (dist > brushRadius && dist2 > brushRadius) { discard; }');
				}
				//

				// frag.write('float dist = distance(bsp.xy, binp.xy);');
				// frag.write('if (dist > brushRadius) { discard; }');
			}
		}
		else { // Fill, Bake
			frag.write('float dist = 0.0;');
		}

		if (UITrait.inst.colorIdPicked) {
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(tex);');
			frag.add_uniform('sampler2D texpaint_colorid'); // 1x1 picker
			frag.add_uniform('sampler2D texcolorid', '_texcolorid'); // color map
			frag.add_uniform('vec2 texcoloridSize', '_texcoloridSize'); // color map
			frag.write('vec3 c1 = texelFetch(texpaint_colorid, ivec2(0, 0), 0).rgb;');
			frag.write('vec3 c2 = texelFetch(texcolorid, ivec2(texCoordPick * texcoloridSize), 0).rgb;');
			frag.write('if (any(c1 != c2)) { discard; }');
		}
		else if (faceFill) { // TODO: allow to combine with colorid mask
			vert.add_out('vec2 texCoordPick');
			vert.write('texCoordPick = fract(tex);');
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
			frag.write('if (any(c1 != c2)) { discard; }');
		}

		if (UITrait.inst.pickerMaskHandle.position == 1) { // material id mask
			frag.wvpposition = true;
			frag.write('vec2 picker_sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
			#if kha_direct3d11
			frag.write('picker_sample_tc.y = 1.0 - picker_sample_tc.y;');
			#end
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			var matid = UITrait.inst.materialIdPicked / 255;
			frag.write('if ($matid != textureLod(texpaint_nor_undo, picker_sample_tc, 0.0).a) discard;');
		}

		// TexCoords - project
		if (UITrait.inst.brushPaint == 1 || decal) {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (decal) {
				frag.write_attrib('uvsp -= inp.xy;');
				frag.write_attrib('uvsp.x *= aspectRatio;');

				frag.write_attrib('uvsp *= 0.21 / (brushRadius * 0.9);');
				frag.write_attrib('uvsp += vec2(0.5, 0.5);');

				if (UITrait.inst.mirrorX) {
					frag.write_attrib('vec2 uvsp2 = sp.xy - vec2(1.0 - inp.x, inp.y);');
					frag.write_attrib('uvsp2.x *= aspectRatio;');
					frag.write_attrib('uvsp2 *= 0.21 / brushRadius;');
					frag.write_attrib('uvsp2 += vec2(0.5, 0.5);');
					frag.write_attrib('if ((uvsp.x < 0.01 || uvsp.y < 0.01 || uvsp.x > 0.99 || uvsp.y > 0.99) && (uvsp2.x < 0.01 || uvsp2.y < 0.01 || uvsp2.x > 0.99 || uvsp2.y > 0.99)) { discard; }');
				}
				else {
					frag.write_attrib('if (uvsp.x < 0.01 || uvsp.y < 0.01 || uvsp.x > 0.99 || uvsp.y > 0.99) { discard; }');
				}
			}
			else {
				frag.write_attrib('uvsp.x *= aspectRatio;');
			}
			
			frag.write_attrib('vec2 texCoord = fract(uvsp * brushScale);');

			if (UITrait.inst.brushRot > 0.0) {
				var a = UITrait.inst.brushRot * (Math.PI / 180);
				frag.write('texCoord = vec2(texCoord.x * ${Math.cos(a)} - texCoord.y * ${Math.sin(a)}, texCoord.x * ${Math.sin(a)} + texCoord.y * ${Math.cos(a)});');
			}
		}
		// TexCoords - uvmap
		else if (UITrait.inst.brushPaint == 0) {
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = tex * brushScale;');

			if (UITrait.inst.brushRot > 0.0) {
				var a = UITrait.inst.brushRot * (Math.PI / 180);
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
			frag.write_attrib('vec2 texCoord = fract(wposition.yz * brushScale);');
			frag.write_attrib('vec2 texCoord1 = fract(wposition.xz * brushScale);');
			frag.write_attrib('vec2 texCoord2 = fract(wposition.xy * brushScale);');
		}

		if (UITrait.inst.selectedTool == ToolClone || UITrait.inst.selectedTool == ToolBlur) {
			
			frag.add_uniform('sampler2D gbuffer2');
			frag.add_uniform('vec2 gbufferSize', '_gbufferSize');
			frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
			frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
			frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');

			if (UITrait.inst.selectedTool == ToolClone) {
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
				if (UITrait.inst.selectedMaterial.paintEmis) {
					frag.write('float emis = 0.0;');
				}
				if (UITrait.inst.selectedMaterial.paintSubs) {
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
				if (UITrait.inst.selectedMaterial.paintEmis) {
					frag.write('float emis = 0.0;');
				}
				if (UITrait.inst.selectedMaterial.paintSubs) {
					frag.write('float subs = 0.0;');
				}
				#if kha_direct3d11
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
			Cycles.parse_emission = UITrait.inst.selectedMaterial.paintEmis;
			Cycles.parse_subsurface = UITrait.inst.selectedMaterial.paintSubs;
			Cycles.parse_height = UITrait.inst.selectedMaterial.paintHeight;
			Cycles.parse_height_as_channel = true;
			Cycles.triplanar = UITrait.inst.brushPaint == 2;
			var sout = Cycles.parse(UINodes.inst.canvas, con_paint, vert, frag, null, null, null, matcon);
			Cycles.parse_emission = false;
			Cycles.parse_subsurface = false;
			Cycles.parse_height_as_channel = false;
			Cycles.parse_height = false;
			var base = sout.out_basecol;
			var rough = sout.out_roughness;
			var met = sout.out_metallic;
			var occ = sout.out_occlusion;
			var nortan = Cycles.out_normaltan;
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
			frag.write('float opacity = $opac * brushOpacity;');
			if (UITrait.inst.selectedMaterial.paintEmis) {
				frag.write('float emis = $emis;');
			}
			if (UITrait.inst.selectedMaterial.paintSubs) {
				frag.write('float subs = $subs;');
			}
			if (height != '0') heightUsed = true;
			if (emis != '0') emisUsed = true;
			if (subs != '0') subsUsed = true;
		}

		if (UITrait.inst.selectedTool == ToolDecal) {
			frag.add_uniform('sampler2D texdecalmask', '_texdecalmask');
			frag.write('opacity *= textureLod(texdecalmask, texCoord, 0.0).r;');
		}
		else if (UITrait.inst.selectedTool == ToolText) {
			frag.add_uniform('sampler2D textexttool', '_textexttool');
			frag.write('opacity *= textureLod(textexttool, texCoord, 0.0).r;');
		}

		if (UITrait.inst.selectedTool == ToolParticle) { // particle mask
			frag.add_uniform('sampler2D texparticle', '_texparticle');
			#if (kha_opengl || kha_webgl)
			frag.write('float str = textureLod(texparticle, vec2(sp.x, (1.0 - sp.y)), 0).r;');
			#else
			frag.write('float str = textureLod(texparticle, sp.xy, 0).r;');
			#end
		}
		else { // brush cursor mask
			frag.write('float str = clamp((brushRadius - dist) * brushHardness * 400.0, 0.0, 1.0) * opacity;');
			if (UITrait.inst.mirrorX && UITrait.inst.selectedTool == ToolBrush) {
				frag.write('str += clamp((brushRadius - dist2) * brushHardness * 400.0, 0.0, 1.0) * opacity;');
				frag.write('str = clamp(str, 0.0, 1.0);');
			}
		}

		// Manual blending to preserve memory
		frag.wvpposition = true;
		frag.write('vec2 sample_tc = vec2(wvpposition.x / wvpposition.w, wvpposition.y / wvpposition.w) * 0.5 + 0.5;');
		#if kha_direct3d11
		frag.write('sample_tc.y = 1.0 - sample_tc.y;');
		#end
		frag.add_uniform('sampler2D paintmask');
		frag.write('float sample_mask = textureLod(paintmask, sample_tc, 0.0).r;');
		frag.write('str = max(str, sample_mask);');

		frag.add_uniform('sampler2D texpaint_undo', '_texpaint_undo');
		frag.write('vec4 sample_undo = textureLod(texpaint_undo, sample_tc, 0.0);');

		var matid = UITrait.inst.selectedMaterial.id / 255;
		if (UITrait.inst.pickerMaskHandle.position == 1) { // material id mask
			matid = UITrait.inst.materialIdPicked / 255; // Keep existing material id in place when mask is set
		}
		frag.write('float matid = $matid;');

		// TODO: Use emission/subsurface matid
		// matid % 3 == 0 - normal, 1 - emission, 2 - subsurface
		if (UITrait.inst.selectedMaterial.paintSubs) {
			frag.write('if (subs > 0) { matid = 254 / 255; }');
		}
		if (UITrait.inst.selectedMaterial.paintEmis) {
			frag.write('if (emis > 0) { matid = 255 / 255; }');
		}

		if (layered) {
			if (eraser) {
				frag.write('fragColor[0] = vec4(basecol, sample_undo.a - str);');
				frag.write('matid = 0;');
			}
			else if (decal) {
				frag.write('float invstr = 1.0 - str;');
				frag.write('fragColor[0] = vec4(basecol * str + sample_undo.rgb * invstr, max(str, sample_undo.a));');
			}
			else {
				frag.write('fragColor[0] = vec4(basecol, max(str, sample_undo.a));');
			}
			frag.write('fragColor[1] = vec4(nortan, matid);');
			
			var height = '0.0';
			if (UITrait.inst.selectedMaterial.paintHeight && heightUsed) {
				height = 'height';
			}

			if (decal) {
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');
				frag.write('fragColor[2] = vec4(
					occlusion * str + sample_pack_undo.r * invstr,
					roughness * str + sample_pack_undo.g * invstr,
					metallic * str + sample_pack_undo.b * invstr,
					$height * str + sample_pack_undo.a * invstr);');
			}
			else {
				frag.write('fragColor[2] = vec4(occlusion, roughness, metallic, $height);');
			}
		}
		else {
			if (eraser) {
				frag.write('fragColor[0] = vec4(0.0, 0.0, 0.0, 0.0);');
				frag.write('fragColor[1] = vec4(0.5, 0.5, 1.0, 0.0);');
				frag.write('fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);');
			}
			else {
				frag.add_uniform('sampler2D texpaint_nor_undo', '_texpaint_nor_undo');
				frag.add_uniform('sampler2D texpaint_pack_undo', '_texpaint_pack_undo');
				frag.write('vec4 sample_nor_undo = textureLod(texpaint_nor_undo, sample_tc, 0.0);');
				frag.write('vec4 sample_pack_undo = textureLod(texpaint_pack_undo, sample_tc, 0.0);');

				frag.write('float invstr = 1.0 - str;');
				frag.write('fragColor[0] = vec4(basecol * str + sample_undo.rgb * invstr, 0.0);');
				frag.write('fragColor[1] = vec4(nortan * str + sample_nor_undo.rgb * invstr, matid);');
				if (UITrait.inst.selectedMaterial.paintHeight && heightUsed) {
					frag.write('fragColor[2] = vec4(occlusion * str + sample_pack_undo.r * invstr, roughness * str + sample_pack_undo.g * invstr, metallic * str + sample_pack_undo.b * invstr, height * str + sample_pack_undo.a * invstr);');
				}
				else {
					frag.write('fragColor[2] = vec4(occlusion * str + sample_pack_undo.r * invstr, roughness * str + sample_pack_undo.g * invstr, metallic * str + sample_pack_undo.b * invstr, 0.0);');
				}
			}
		}
		frag.write('fragColor[3] = vec4(str, 0.0, 0.0, 1.0);');

		if (!UITrait.inst.selectedMaterial.paintBase) {
			con_paint.data.color_writes_red[0] = false;
			con_paint.data.color_writes_green[0] = false;
			con_paint.data.color_writes_blue[0] = false;
		}
		if (!UITrait.inst.selectedMaterial.paintNor) {
			con_paint.data.color_writes_red[1] = false;
			con_paint.data.color_writes_green[1] = false;
			con_paint.data.color_writes_blue[1] = false;
		}
		if (!UITrait.inst.selectedMaterial.paintOcc) {
			con_paint.data.color_writes_red[2] = false;
		}
		if (!UITrait.inst.selectedMaterial.paintRough) {
			con_paint.data.color_writes_green[2] = false;
		}
		if (!UITrait.inst.selectedMaterial.paintMet) {
			con_paint.data.color_writes_blue[2] = false;
		}
		if (!UITrait.inst.selectedMaterial.paintHeight) {
			con_paint.data.color_writes_alpha[2] = false;
		}

		// Base color only as mask
		var isMask = UITrait.inst.selectedLayerIsMask;
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

		if (UITrait.inst.selectedTool == ToolBake) {
			// Apply normal channel
			frag.vVec = true;
			frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
			#if kha_direct3d11
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end
			frag.write('n = nortan * 2.0 - 1.0;');
			frag.write('n.y = -n.y;');
			frag.write('n = normalize(mul(n, TBN));');

			frag.write('const vec3 voxelgiHalfExtents = vec3(1.0, 1.0, 1.0);');
			frag.write('vec3 voxpos = wposition / voxelgiHalfExtents;');
			frag.add_uniform('sampler3D voxels');
			frag.add_function(armory.system.CyclesFunctions.str_traceAO);
			frag.n = true;
			var strength = UITrait.inst.bakeStrength;
			var radius = UITrait.inst.bakeRadius;
			var offset = UITrait.inst.bakeOffset;
			frag.write('fragColor[2].r = 1.0 - traceAO(voxpos, n, $radius, $offset) * $strength;');
		}

		Cycles.finalize(con_paint);
		Cycles.triplanar = false;
		con_paint.data.shader_from_source = true;
		con_paint.data.vertex_shader = vert.get();
		con_paint.data.fragment_shader = frag.get();

		return con_paint;
	}

	public static function make_mesh_preview(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
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

		Cycles.parse_height = heightUsed;
		var sout = Cycles.parse(UINodes.inst.canvas, con_mesh, vert, frag, null, null, null, matcon);
		Cycles.parse_height = false;
		var base = sout.out_basecol;
		var rough = sout.out_roughness;
		var met = sout.out_metallic;
		var occ = sout.out_occlusion;
		var opac = sout.out_opacity;
		var nortan = Cycles.out_normaltan;
		frag.write('vec3 basecol = pow($base, vec3(2.2, 2.2, 2.2));');
		frag.write('float roughness = $rough;');
		frag.write('float metallic = $met;');
		frag.write('float occlusion = $occ;');
		frag.write('float opacity = $opac;');
		frag.write('vec3 nortan = $nortan;');

		var decal = UITrait.inst.decalPreview;
		if (decal) {
			if (UITrait.inst.selectedTool == ToolDecal) {
				frag.add_uniform('sampler2D texdecalmask', '_texdecalmask');
				frag.write('opacity *= textureLod(texdecalmask, texCoord, 0.0).r;');
			}
			else if (UITrait.inst.selectedTool == ToolText) {
				frag.add_uniform('sampler2D textexttool', '_textexttool');
				frag.write('opacity *= textureLod(textexttool, texCoord, 0.0).r;');
			}
			var opac = 0.05;
			frag.write('if (opacity < $opac) discard;');
		}

		frag.add_out('vec4 fragColor[3]');
		frag.n = true;

		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);
		frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
		frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

		// Apply normal channel
		if (decal) {
			// TODO
		}
		else {
			frag.vVec = true;
			#if kha_direct3d11
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
		// float matid = 0.0;
		frag.write('fragColor[0] = vec4(n.x, n.y, packFloat(metallic, roughness), 0.0);');
		frag.write('fragColor[1] = vec4(basecol.r, basecol.g, basecol.b, packFloat2(occlusion, 1.0));'); // occ/spec
		frag.write('fragColor[2] = vec4(0.0, 0.0, 0.0, 0.0);'); // veloc

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}

	public static function make_depth(data:CyclesShaderData, matcon:TMaterialContext):CyclesShaderContext {
		var context_id = 'depth';
		var con_depth:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: 'clockwise',
			color_writes_red: [false],
			color_writes_green: [false],
			color_writes_blue: [false],
			color_writes_alpha: [false],
			vertex_elements: [{name: "pos", data: 'short4norm'}]
		});

		var vert = con_depth.make_vert();
		var frag = con_depth.make_frag();

		frag.ins = vert.outs;

		vert.add_uniform('mat4 WVP', '_worldViewProjectionMatrix');
		vert.write('gl_Position = mul(vec4(pos.xyz, 1.0), WVP);');

		// Cycles.finalize(con_depth);
		con_depth.data.shader_from_source = true;
		con_depth.data.vertex_shader = vert.get();
		con_depth.data.fragment_shader = frag.get();

		return con_depth;
	}

	public static function make_particle(data:CyclesShaderData):CyclesShaderContext {
		var context_id = 'mesh';
		var con_part:CyclesShaderContext = data.add_context({
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
		// frag.add_uniform('sampler2D paintdb');
		// frag.write('fragColor *= 1.0 - clamp(distance(textureLod(paintdb, texCoord, 0).r, wvpposition.z), 0.0, 1.0);');

		// Cycles.finalize(con_part);
		con_part.data.shader_from_source = true;
		con_part.data.vertex_shader = vert.get();
		con_part.data.fragment_shader = frag.get();

		return con_part;
	}

	public static function make_mesh(data:CyclesShaderData):CyclesShaderContext {
		var context_id = 'mesh';
		var con_mesh:CyclesShaderContext = data.add_context({
			name: context_id,
			depth_write: true,
			compare_mode: 'less',
			cull_mode: UITrait.inst.culling ? 'clockwise' : 'none',
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
			#if (!kha_direct3d11) // TODO: unable to bind texpaint_pack to both vs and fs in d3d11
			vert.write('float height = textureLod(texpaint_pack, tex, 0.0).a;');
			var displaceStrength = UITrait.inst.displaceStrength * 0.1;
			vert.n = true;
			vert.write('wposition += wnormal * height * $displaceStrength;');
			#end
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
		frag.add_function(armory.system.CyclesFunctions.str_packFloat);
		frag.add_function(armory.system.CyclesFunctions.str_packFloat2);

		if (UITrait.inst.selectedTool == ToolColorId) {
			frag.add_uniform('sampler2D texcolorid', '_texcolorid');
			frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 1.0);'); // met/rough
			frag.write('vec3 idcol = pow(textureLod(texcolorid, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
			frag.write('fragColor[1] = vec4(idcol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
		}
		else {
			frag.add_function(armory.system.CyclesFunctions.str_octahedronWrap);

			frag.write('vec3 basecol;');
			frag.write('float roughness;');
			frag.write('float metallic;');
			frag.write('float occlusion;');
			frag.write('float opacity;');
			frag.write('float specular;');
			frag.write('float matid = 0.0;');

			frag.vVec = true;
			frag.add_function(armory.system.CyclesFunctions.str_cotangentFrame);
			#if kha_direct3d11
			frag.write('mat3 TBN = cotangentFrame(n, vVec, texCoord);');
			#else
			frag.write('mat3 TBN = cotangentFrame(n, -vVec, texCoord);');
			#end

			if (UITrait.inst.layers[0].visible) {

				if (UITrait.inst.selectedLayer.paintBase) {
					frag.add_uniform('sampler2D texpaint');
					frag.write('basecol = pow(textureLod(texpaint, texCoord, 0.0).rgb, vec3(2.2, 2.2, 2.2));');
				}
				else {
					frag.write('basecol = vec3(0.0, 0.0, 0.0);');
				}

				if (UITrait.inst.selectedLayer.paintNor ||
					emisUsed) {
					frag.add_uniform('sampler2D texpaint_nor');
					frag.write('vec4 texpaint_nor_sample = textureLod(texpaint_nor, texCoord, 0.0);');

					if (emisUsed) {
						frag.write('matid = texpaint_nor_sample.a;');
					}

					if (UITrait.inst.selectedLayer.paintNor) {
						frag.write('vec3 ntex = texpaint_nor_sample.rgb;');
						frag.write('n = ntex * 2.0 - 1.0;');
						frag.write('n.y = -n.y;');
						frag.write('n = normalize(mul(n, TBN));');
					}
				}

				if (heightUsed ||
					UITrait.inst.selectedLayer.paintOcc ||
					UITrait.inst.selectedLayer.paintRough ||
					UITrait.inst.selectedLayer.paintMet) {
					frag.add_uniform('sampler2D texpaint_pack');
					frag.write('vec4 pack = textureLod(texpaint_pack, texCoord, 0.0);');
				}

				// Height
				if (heightUsed) {
					frag.write('vec4 vech;');
					frag.write('vech.x = textureOffset(texpaint_pack, texCoord, ivec2(-1, 0)).a;');
					frag.write('vech.y = textureOffset(texpaint_pack, texCoord, ivec2(1, 0)).a;');
					frag.write('vech.z = textureOffset(texpaint_pack, texCoord, ivec2(0, -1)).a;');
					frag.write('vech.w = textureOffset(texpaint_pack, texCoord, ivec2(0, 1)).a;');
					// Displace normal strength
					frag.write('vech *= 15 * 7; float h1 = vech.x - vech.y; float h2 = vech.z - vech.w;');
					frag.write('vec3 va = normalize(vec3(2.0, 0.0, h1));');
					frag.write('vec3 vb = normalize(vec3(0.0, 2.0, h2));');
					frag.write('vec3 vc = normalize(vec3(h1, h2, 2.0));');
					frag.write('n = normalize(mul(n, mat3(va, vb, vc)));');
				}
				//

				if (UITrait.inst.selectedLayer.paintOcc) {
					frag.write('occlusion = pack.r;');
				}
				else {
					frag.write('occlusion = 1.0;');
				}
				if (UITrait.inst.selectedLayer.paintRough) {
					frag.write('roughness = pack.g;');
				}
				else {
					frag.write('roughness = 1.0;');
				}
				if (UITrait.inst.selectedLayer.paintMet) {
					frag.write('metallic = pack.b;');
				}
				else {
					frag.write('metallic = 0.0;');
				}

				var l = UITrait.inst.layers[0];
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

			if (UITrait.inst.layers.length > 1) {
				frag.write('float factor0;');
				frag.write('float factorinv0;');
				frag.write('vec4 col_tex0;');
				frag.write('vec4 col_nor0;');
				frag.write('vec4 col_pack0;');
				frag.write('vec3 n0;');
				var numLayers = 1;
				for (i in 1...UITrait.inst.layers.length) {
					var l = UITrait.inst.layers[i];
					if (!l.visible) continue;
					if (numLayers > 4) break; // 16 samplers limit
					numLayers++;
					var id = l.id;

					if (l.objectMask > 0) {
						var uid = UITrait.inst.paintObjects[l.objectMask - 1].uid;
						frag.add_uniform('int objectId', '_uid');
						frag.write('if ($uid == objectId) {');
					}

					frag.add_uniform('sampler2D texpaint' + id);
					frag.write('col_tex0 = textureLod(texpaint' + id + ', texCoord, 0.0);');
					frag.write('factor0 = col_tex0.a;');

					if (l.texpaint_mask != null) {
						frag.add_uniform('sampler2D texpaint_mask' + id);
						frag.write('factor0 *= textureLod(texpaint_mask' + id + ', texCoord, 0.0).r;');
					}
					if (l.maskOpacity < 1) {
						frag.write('factor0 *= ${l.maskOpacity};');
					}

					frag.write('factorinv0 = 1.0 - factor0;');

					if (UITrait.inst.selectedLayer.paintBase) {
						frag.write('basecol = basecol * factorinv0 + pow(col_tex0.rgb, vec3(2.2, 2.2, 2.2)) * factor0;');
					}

					if (emisUsed ||
						UITrait.inst.selectedLayer.paintNor) {

						frag.add_uniform('sampler2D texpaint_nor' + id);
						frag.write('col_nor0 = textureLod(texpaint_nor' + id + ', texCoord, 0.0);');

						if (emisUsed) {
							frag.write('matid = col_nor0.a;');
						}

						if (UITrait.inst.selectedLayer.paintNor) {
							frag.write('n0 = col_nor0.rgb * 2.0 - 1.0;');
							frag.write('n0.y = -n0.y;');
							frag.write('n0 = normalize(mul(n0, TBN));');
							frag.write('n = normalize(n * factorinv0 + n0 * factor0);');
						}
					}

					if (UITrait.inst.selectedLayer.paintOcc ||
						UITrait.inst.selectedLayer.paintRough ||
						UITrait.inst.selectedLayer.paintMet) {
						frag.add_uniform('sampler2D texpaint_pack' + id);
						frag.write('col_pack0 = textureLod(texpaint_pack' + id + ', texCoord, 0.0);');
					
						if (UITrait.inst.selectedLayer.paintOcc) {
							frag.write('occlusion = occlusion * factorinv0 + col_pack0.r * factor0;');
						}
						if (UITrait.inst.selectedLayer.paintRough) {
							frag.write('roughness = roughness * factorinv0 + col_pack0.g * factor0;');
						}
						if (UITrait.inst.selectedLayer.paintMet) {
							frag.write('metallic = metallic * factorinv0 + col_pack0.b * factor0;');
						}
					}

					if (l.objectMask > 0) {
						frag.write('}');
					}
				}
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
			if (UITrait.inst.viewportMode == 0) { // Render
				if (emisUsed) {
					frag.write('if (matid == 1.0) basecol *= 10.0;'); // Boost for bloom
				}
				frag.write('fragColor[0] = vec4(n.xy, packFloat(metallic, roughness), matid);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(occlusion, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 1) { // Basecol
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(basecol.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 2) { // Normal Map
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(ntex.rgb, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 3) { // Occ
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(occlusion, occlusion, occlusion), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 4) { // Rough
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(roughness, roughness, roughness), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 5) { // Met
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(vec3(metallic, metallic, metallic), packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 6) { // Texcoord
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(texCoord, 0.0, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 7) { // Normal
				frag.nAttr = true;
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(nAttr, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 8) { // MaterialID
				frag.write('float sample_matid = textureLod(texpaint_nor, texCoord, 0.0).a;');
				frag.write('float matid_r = fract(sin(dot(vec2(sample_matid, sample_matid * 2.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_g = fract(sin(dot(vec2(sample_matid * 2.0, sample_matid), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('float matid_b = fract(sin(dot(vec2(sample_matid, sample_matid * 4.0), vec2(12.9898, 78.233))) * 43758.5453);');
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('fragColor[1] = vec4(matid_r, matid_g, matid_b, packFloat2(1.0, 1.0));'); // occ/spec
			}
			else if (UITrait.inst.viewportMode == 9) { // Mask
				frag.write('fragColor[0] = vec4(n.xy, packFloat(0.0, 1.0), 0.0);');
				frag.write('float sample_mask = 1.0;');
				if (UITrait.inst.selectedLayer.texpaint_mask != null) {
					frag.add_uniform('sampler2D texpaint_mask', '_texpaint_mask');
					frag.write('sample_mask = textureLod(texpaint_mask, texCoord, 0.0).r;');
				}
				frag.write('fragColor[1] = vec4(sample_mask, sample_mask, sample_mask, packFloat2(1.0, 1.0));'); // occ/spec
			}
		}

		frag.write('vec2 posa = (wvpposition.xy / wvpposition.w) * 0.5 + 0.5;');
		frag.write('vec2 posb = (prevwvpposition.xy / prevwvpposition.w) * 0.5 + 0.5;');
		frag.write('fragColor[2] = vec4(posa - posb, texCoord.xy);');

		Cycles.finalize(con_mesh);
		con_mesh.data.shader_from_source = true;
		con_mesh.data.vertex_shader = vert.get();
		con_mesh.data.fragment_shader = frag.get();

		return con_mesh;
	}
}
