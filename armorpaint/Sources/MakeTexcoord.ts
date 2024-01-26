
class MakeTexcoord {

	static run = (vert: NodeShaderRaw, frag: NodeShaderRaw) => {

		let fillLayer = Context.raw.layer.fill_layer != null;
		let uvType = fillLayer ? Context.raw.layer.uvType : Context.raw.brushPaint;
		let decal = Context.raw.tool == WorkspaceTool.ToolDecal || Context.raw.tool == WorkspaceTool.ToolText;
		let angle = Context.raw.brushAngle + Context.raw.brushNodesAngle;
		let uvAngle = fillLayer ? Context.raw.layer.angle : angle;

		if (uvType == UVType.UVProject || decal) { // TexCoords - project
			NodeShader.add_uniform(frag, 'float brushScale', '_brushScale');
			NodeShader.write_attrib(frag, 'vec2 uvsp = sp.xy;');

			if (fillLayer) { // Decal layer
				NodeShader.write_attrib(frag, 'if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');

				if (uvAngle != 0.0) {
					NodeShader.add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
					NodeShader.write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}

				frag.n = true;
				NodeShader.add_uniform(frag, 'vec3 decalLayerNor', '_decalLayerNor');
				let dotAngle = Context.raw.brushAngleRejectDot;
				NodeShader.write(frag, `if (abs(dot(n, decalLayerNor) - 1.0) > ${dotAngle}) discard;`);

				frag.wposition = true;
				NodeShader.add_uniform(frag, 'vec3 decalLayerLoc', '_decalLayerLoc');
				NodeShader.add_uniform(frag, 'float decalLayerDim', '_decalLayerDim');
				NodeShader.write_attrib(frag, 'if (abs(dot(decalLayerNor, decalLayerLoc - wposition)) > decalLayerDim) discard;');
			}
			else if (decal) {
				NodeShader.add_uniform(frag, 'vec4 decalMask', '_decalMask');
				NodeShader.write_attrib(frag, 'uvsp -= decalMask.xy;');
				NodeShader.write_attrib(frag, 'uvsp.x *= aspectRatio;');
				NodeShader.write_attrib(frag, 'uvsp *= 0.21 / (decalMask.w * 0.9);'); // Decal radius

				if (Context.raw.brushDirectional) {
					NodeShader.add_uniform(frag, 'vec3 brushDirection', '_brushDirection');
					NodeShader.write_attrib(frag, 'if (brushDirection.z == 0.0) discard;');
					NodeShader.write_attrib(frag, 'uvsp = vec2(uvsp.x * brushDirection.x - uvsp.y * brushDirection.y, uvsp.x * brushDirection.y + uvsp.y * brushDirection.x);');
				}

				if (uvAngle != 0.0) {
					NodeShader.add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
					NodeShader.write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}

				NodeShader.add_uniform(frag, 'float brushScaleX', '_brushScaleX');
				NodeShader.write_attrib(frag, 'uvsp.x *= brushScaleX;');

				NodeShader.write_attrib(frag, 'uvsp += vec2(0.5, 0.5);');

				NodeShader.write_attrib(frag, 'if (uvsp.x < 0.0 || uvsp.y < 0.0 || uvsp.x > 1.0 || uvsp.y > 1.0) discard;');
			}
			else {
				NodeShader.write_attrib(frag, 'uvsp.x *= aspectRatio;');

				if (uvAngle != 0.0) {
					NodeShader.add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
					NodeShader.write_attrib(frag, 'uvsp = vec2(uvsp.x * brushAngle.x - uvsp.y * brushAngle.y, uvsp.x * brushAngle.y + uvsp.y * brushAngle.x);');
				}
			}

			NodeShader.write_attrib(frag, 'vec2 texCoord = uvsp * brushScale;');
		}
		else if (uvType == UVType.UVMap) { // TexCoords - uvmap
			NodeShader.add_uniform(vert, 'float brushScale', '_brushScale');
			NodeShader.add_out(vert, 'vec2 texCoord');
			NodeShader.write(vert, 'texCoord = tex * brushScale;');

			if (uvAngle > 0.0) {
				NodeShader.add_uniform(vert, 'vec2 brushAngle', '_brushAngle');
				NodeShader.write(vert, 'texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
			}
		}
		else { // TexCoords - triplanar
			frag.wposition = true;
			frag.n = true;
			NodeShader.add_uniform(frag, 'float brushScale', '_brushScale');
			NodeShader.write_attrib(frag, 'vec3 triWeight = wnormal * wnormal;'); // n * n
			NodeShader.write_attrib(frag, 'float triMax = max(triWeight.x, max(triWeight.y, triWeight.z));');
			NodeShader.write_attrib(frag, 'triWeight = max(triWeight - triMax * 0.75, 0.0);');
			NodeShader.write_attrib(frag, 'vec3 texCoordBlend = triWeight * (1.0 / (triWeight.x + triWeight.y + triWeight.z));');
			NodeShader.write_attrib(frag, 'vec2 texCoord = wposition.yz * brushScale * 0.5;');
			NodeShader.write_attrib(frag, 'vec2 texCoord1 = wposition.xz * brushScale * 0.5;');
			NodeShader.write_attrib(frag, 'vec2 texCoord2 = wposition.xy * brushScale * 0.5;');

			if (uvAngle != 0.0) {
				NodeShader.add_uniform(frag, 'vec2 brushAngle', '_brushAngle');
				NodeShader.write_attrib(frag, 'texCoord = vec2(texCoord.x * brushAngle.x - texCoord.y * brushAngle.y, texCoord.x * brushAngle.y + texCoord.y * brushAngle.x);');
				NodeShader.write_attrib(frag, 'texCoord1 = vec2(texCoord1.x * brushAngle.x - texCoord1.y * brushAngle.y, texCoord1.x * brushAngle.y + texCoord1.y * brushAngle.x);');
				NodeShader.write_attrib(frag, 'texCoord2 = vec2(texCoord2.x * brushAngle.x - texCoord2.y * brushAngle.y, texCoord2.x * brushAngle.y + texCoord2.y * brushAngle.x);');
			}
		}
	}
}
