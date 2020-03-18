package arm.node;

import arm.ui.UISidebar;
import arm.node.MaterialShader;
import arm.Enums;

class MakeTexcoord {

	public static function run(vert: MaterialShader, frag: MaterialShader) {

		var uvType = Context.layer.material_mask != null ? Context.layer.uvType : UISidebar.inst.brushPaint;
		var decal = Context.tool == ToolDecal || Context.tool == ToolText;

		// TexCoords - project
		if (uvType == UVProject || decal) {
			frag.add_uniform('float brushScale', '_brushScale');
			frag.write_attrib('vec2 uvsp = sp.xy;');

			if (decal) {
				frag.write_attrib('uvsp -= inp.xy;');
				frag.write_attrib('uvsp.x *= aspectRatio;');
				frag.write_attrib('uvsp *= 0.21 / (brushRadius * 0.9);');
				frag.add_uniform('float brushScaleX', '_brushScaleX');
				frag.write_attrib('uvsp.x *= brushScaleX;');
				frag.write_attrib('uvsp += vec2(0.5, 0.5);');
				frag.write_attrib('if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');
			}
			else {
				frag.write_attrib('uvsp.x *= aspectRatio;');
			}

			frag.write_attrib('vec2 texCoord = uvsp * brushScale;');

			// if (UISidebar.inst.brushDirectional) {
			// 	frag.add_uniform('vec2 brushDirection', '_brushDirection');
			// 	frag.write('texCoord = vec2(texCoord.x * brushDirection.x - texCoord.y * brushDirection.y, texCoord.x * brushDirection.y + texCoord.y * brushDirection.x);');
			// }
			if (UISidebar.inst.brushDirectional) {
				frag.write('float maskAngleBrush = atan2(-inp.y + inplast.y, inp.x - inplast.x) - 3.141592 / 2;');
				frag.write('texCoord = vec2(texCoord.x * cos(maskAngleBrush) - texCoord.y * sin(maskAngleBrush), texCoord.x * sin(maskAngleBrush) + texCoord.y * cos(maskAngleBrush));');
			}
			var angle = UISidebar.inst.brushAngle + UISidebar.inst.brushNodesAngle;
			var uvAngle = Context.layer.material_mask != null ? Context.layer.angle : angle;
			if (uvAngle != 0.0) {
				var a = uvAngle * (Math.PI / 180);
				frag.write('texCoord = vec2(texCoord.x * ${Math.cos(a)} - texCoord.y * ${Math.sin(a)}, texCoord.x * ${Math.sin(a)} + texCoord.y * ${Math.cos(a)});');
			}
		}
		else if (uvType == UVMap) { // TexCoords - uvmap
			vert.add_uniform('float brushScale', '_brushScale');
			vert.add_out('vec2 texCoord');
			vert.write('texCoord = subtex * brushScale;');

			var angle = UISidebar.inst.brushAngle + UISidebar.inst.brushNodesAngle;
			var uvAngle = Context.layer.material_mask != null ? Context.layer.angle : angle;
			if (uvAngle > 0.0) {
				var a = uvAngle * (Math.PI / 180);
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
			frag.write_attrib('vec2 texCoord = wposition.yz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord1 = wposition.xz * brushScale * 0.5;');
			frag.write_attrib('vec2 texCoord2 = wposition.xy * brushScale * 0.5;');
		}
	}
}
