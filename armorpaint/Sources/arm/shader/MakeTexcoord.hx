package arm.shader;

import arm.shader.NodeShader;

class MakeTexcoord {

	public static function run(vert: NodeShader, frag: NodeShader) {

		var fillLayer = Context.raw.layer.fill_layer != null;
		var uvType = fillLayer ? Context.raw.layer.uvType : Context.raw.brushPaint;
		var decal = Context.raw.tool == ToolDecal || Context.raw.tool == ToolText;
		var angle = Context.raw.brushAngle + Context.raw.brushNodesAngle;
		var uvAngle = fillLayer ? Context.raw.layer.angle : angle;

		if (uvType == UVProject || decal) { // TexCoords - project
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (fillLayer) { // Decal layer
				frag.write_attrib('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');

				if (uvAngle != 0.0) {
					frag.add_uniform('vec2 brushAngle', '_brushAngle');
					frag.write_attrib('uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}

				frag.n = true;
				frag.add_uniform('vec3 decalLayerNor', '_decalLayerNor');
				var dotAngle = Context.raw.brushAngleRejectDot;
				frag.write('if (abs(dot(n, decalLayerNor) - 1.0) > $dotAngle) discard;');

				frag.wposition = true;
				frag.add_uniform('vec3 decalLayerLoc', '_decalLayerLoc');
				frag.add_uniform('float decalLayerDim', '_decalLayerDim');
				frag.write_attrib('if (abs(dot(decalLayerNor, decalLayerLoc - wposition)) > decalLayerDim) discard;');
			}
			else if (decal) {
				frag.add_uniform('vec4 decalMask', '_decalMask');
				frag.write_attrib('uvsp -= decalMask.xy;');
				frag.write_attrib('uvsp.x *= aspectRatio;');
				frag.write_attrib('uvsp *= 0.21 / (decalMask.w * 0.9);'); // Decal radius

				if (Context.raw.brushDirectional) {
					frag.add_uniform('vec3 brushDirection', '_brushDirection');
					frag.write_attrib('if (brushDirection.z == 0.0) discard;');
					frag.write_attrib('uvsp = vec2(uvsp.x * brushDirection.x - uvsp.y * brushDirection.y, uvsp.x * brushDirection.y + uvsp.y * brushDirection.x);');
				}

				if (uvAngle != 0.0) {
					frag.add_uniform('vec2 brushAngle', '_brushAngle');
					frag.write_attrib('uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}

				frag.add_uniform('float brushScaleX', '_brushScaleX');
				frag.write_attrib('uvsp.x *= brushScaleX;');

				frag.write_attrib('uvsp += vec2(0.5, 0.5);');

				frag.write_attrib('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');
			}
			else {
				frag.write_attrib('uvsp.x *= aspectRatio;');

				if (uvAngle != 0.0) {
					frag.add_uniform('vec2 brushAngle', '_brushAngle');
					frag.write_attrib('uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}
			}

			frag.write_attrib('vec2 texCoord = uvsp * brushScale;');
		}
		else if (uvType == UVMap) { // TexCoords - uvmap
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = tex * brushScale;');

			if (uvAngle > 0.0) {
				vert.add_uniform('vec2 brushAngle', '_brushAngle');
				vert.write('texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
			}
		}
		else { // TexCoords - triplanar
			frag.wposition = true;
			frag.n = true;
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec3 triWeight = wnormal * wnormal;'); // n * n
			frag.write_attrib('float triMax = max(triWeight.x, max(triWeight.y, triWeight.z));');
			frag.write_attrib('triWeight = max(triWeight - triMax * 0.75, 0.0);');
			frag.write_attrib('vec3 texCoordBlend = triWeight * (1.0 / (triWeight.x + triWeight.y + triWeight.z));');
			frag.write_attrib('vec2 texCoord = wposition.yz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord1 = wposition.xz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord2 = wposition.xy * brushScale * 0.5;');

			if (uvAngle != 0.0) {
				frag.add_uniform('vec2 brushAngle', '_brushAngle');
				frag.write_attrib('texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
				frag.write_attrib('texCoord1 = vec2(texCoord1.x * brushAngle.x - texCoord1.y * brushAngle.y, texCoord1.x * brushAngle.y + texCoord1.y * brushAngle.x);');
				frag.write_attrib('texCoord2 = vec2(texCoord2.x * brushAngle.x - texCoord2.y * brushAngle.y, texCoord2.x * brushAngle.y + texCoord2.y * brushAngle.x);');
			}
		}
	}
}
