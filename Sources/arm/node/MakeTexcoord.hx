package arm.node;

import arm.ui.UISidebar;
import arm.node.MaterialShader;
import arm.Enums;

class MakeTexcoord {

	public static function run(vert: MaterialShader, frag: MaterialShader) {

		var fillLayer = Context.layer.material_mask != null;
		var uvType = fillLayer ? Context.layer.uvType : Context.brushPaint;
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;

		// TexCoords - project
		if (uvType == UVProject || decal) {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (fillLayer) { // Decal layer
				frag.write_attrib('uvsp.x *= aspectRatio;');
				frag.write_attrib('uvsp.xy *= 4.0;');
				//frag.write_attrib('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');
			}
			else if (decal) {
				frag.write_attrib('uvsp -= inp.xy;');
				frag.write_attrib('uvsp.x *= aspectRatio;');
				frag.write_attrib('uvsp *= 0.21 / (brushRadius * 0.9);');

				if (Context.brushDirectional) {
					frag.add_uniform('vec3 brushDirection', '_brushDirection');
					frag.write_attrib('if (brushDirection.z == 0.0) discard;');
					frag.write_attrib('uvsp = vec2(uvsp.x * brushDirection.x - uvsp.y * brushDirection.y, uvsp.x * brushDirection.y + uvsp.y * brushDirection.x);');
				}
				var angle = Context.brushAngle + Context.brushNodesAngle;
				var uvAngle = Context.layer.material_mask != null ? Context.layer.angle : angle;
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
			}

			frag.write_attrib('vec2 texCoord = uvsp * brushScale;');
		}
		else if (uvType == UVMap) { // TexCoords - uvmap
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = subtex * brushScale;');

			var angle = Context.brushAngle + Context.brushNodesAngle;
			var uvAngle = Context.layer.material_mask != null ? Context.layer.angle : angle;
			if (uvAngle > 0.0) {
				vert.add_uniform('vec2 brushAngle', '_brushAngle');
				vert.write('texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
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
			frag.write_attrib('vec2 texCoord = wposition.yz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord1 = wposition.xz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord2 = wposition.xy * brushScale * 0.5;');
		}
	}
}
