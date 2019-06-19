package arm.io;

import zui.Nodes;
import arm.ui.UITrait;
import arm.ui.UINodes;
import arm.nodes.NodesMaterial;
import arm.util.Path;
import arm.util.RenderUtil;
import arm.data.MaterialSlot;

class ImportBlend {

	public static function run(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var bl = new iron.format.blend.Blend(b);
			if (bl.dna == null) { Importer.makeMesh(null, path); return; }

			// var obs = bl.get("Object");
			// var ob = obs[0];
			// var name:String = ob.get("id").get("name");
			// name = name.substring(2, name.length);
			// trace(ob.get("type")); // 1

			var m = bl.get("Mesh")[0];
			if (m == null) { Importer.makeMesh(null, path); return; }

			var totpoly = m.get("totpoly");
			var numtri = 0;
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var totloop = poly.get("totloop");
				numtri += totloop == 3 ? 1 : 2;
			}
			var inda = new kha.arrays.Uint32Array(numtri * 3);
			for (i in 0...inda.length) inda[i] = i;

			var posa32 = new kha.arrays.Float32Array(numtri * 3 * 4);
			var posa = new kha.arrays.Int16Array(numtri * 3 * 4);
			var nora = new kha.arrays.Int16Array(numtri * 3 * 2);
			var hasuv = m.get("mloopuv") != null;
			var texa = hasuv ? new kha.arrays.Int16Array(numtri * 3 * 2) : null;
			
			var tri = 0;
			var vec0 = new iron.math.Vec4();
			var vec1 = new iron.math.Vec4();
			var vec2 = new iron.math.Vec4();
			var vec3 = new iron.math.Vec4();
			for (i in 0...totpoly) {
				var poly = m.get("mpoly", i);
				var loopstart = poly.get("loopstart");
				var totloop = poly.get("totloop");
				if (totloop >= 3) {
					var v0 = m.get("mvert", m.get("mloop", loopstart    ).get("v"));
					var v1 = m.get("mvert", m.get("mloop", loopstart + 1).get("v"));
					var v2 = m.get("mvert", m.get("mloop", loopstart + 2).get("v"));
					var co0 = v0.get("co");
					var co1 = v1.get("co");
					var co2 = v2.get("co");
					var no0 = v0.get("no");
					var no1 = v1.get("no");
					var no2 = v2.get("no");
					vec0.set(no0[0] / 32767, no0[1] / 32767, no0[2] / 32767).normalize(); // shortmax
					vec1.set(no1[0] / 32767, no1[1] / 32767, no1[2] / 32767).normalize();
					vec2.set(no2[0] / 32767, no2[1] / 32767, no2[2] / 32767).normalize();
					posa32[tri * 9    ] = co0[0];
					posa32[tri * 9 + 1] = co0[1];
					posa32[tri * 9 + 2] = co0[2];
					posa32[tri * 9 + 3] = co1[0];
					posa32[tri * 9 + 4] = co1[1];
					posa32[tri * 9 + 5] = co1[2];
					posa32[tri * 9 + 6] = co2[0];
					posa32[tri * 9 + 7] = co2[1];
					posa32[tri * 9 + 8] = co2[2];
					posa[tri * 12 + 3] = Std.int(vec0.z * 32767);
					posa[tri * 12 + 7] = Std.int(vec1.z * 32767);
					posa[tri * 12 + 11] = Std.int(vec2.z * 32767);
					nora[tri * 6    ] = Std.int(vec0.x * 32767);
					nora[tri * 6 + 1] = Std.int(vec0.y * 32767);
					nora[tri * 6 + 2] = Std.int(vec1.x * 32767);
					nora[tri * 6 + 3] = Std.int(vec1.y * 32767);
					nora[tri * 6 + 4] = Std.int(vec2.x * 32767);
					nora[tri * 6 + 5] = Std.int(vec2.y * 32767);
					
					var uv0:kha.arrays.Float32Array = null;
					var uv1:kha.arrays.Float32Array = null;
					var uv2:kha.arrays.Float32Array = null;
					if (hasuv) {
						uv0 = m.get("mloopuv", loopstart    ).get("uv");
						uv1 = m.get("mloopuv", loopstart + 1).get("uv");
						uv2 = m.get("mloopuv", loopstart + 2).get("uv");
						texa[tri * 6    ] = Std.int(uv0[0] * 32767);
						texa[tri * 6 + 1] = Std.int((1.0 - uv0[1]) * 32767);
						texa[tri * 6 + 2] = Std.int(uv1[0] * 32767);
						texa[tri * 6 + 3] = Std.int((1.0 - uv1[1]) * 32767);
						texa[tri * 6 + 4] = Std.int(uv2[0] * 32767);
						texa[tri * 6 + 5] = Std.int((1.0 - uv2[1]) * 32767);
					}
					tri++;

					if (totloop >= 4) {
						var v3 = m.get("mvert", m.get("mloop", loopstart + 3).get("v"));
						var co3 = v3.get("co");
						var no3 = v3.get("no");
						vec3.set(no3[0] / 32767, no3[1] / 32767, no3[2] / 32767).normalize();
						posa32[tri * 9    ] = co2[0];
						posa32[tri * 9 + 1] = co2[1];
						posa32[tri * 9 + 2] = co2[2];
						posa32[tri * 9 + 3] = co3[0];
						posa32[tri * 9 + 4] = co3[1];
						posa32[tri * 9 + 5] = co3[2];
						posa32[tri * 9 + 6] = co0[0];
						posa32[tri * 9 + 7] = co0[1];
						posa32[tri * 9 + 8] = co0[2];
						posa[tri * 12 + 3] = Std.int(vec2.z * 32767);
						posa[tri * 12 + 7] = Std.int(vec3.z * 32767);
						posa[tri * 12 + 11] = Std.int(vec0.z * 32767);
						nora[tri * 6    ] = Std.int(vec2.x * 32767);
						nora[tri * 6 + 1] = Std.int(vec2.y * 32767);
						nora[tri * 6 + 2] = Std.int(vec3.x * 32767);
						nora[tri * 6 + 3] = Std.int(vec3.y * 32767);
						nora[tri * 6 + 4] = Std.int(vec0.x * 32767);
						nora[tri * 6 + 5] = Std.int(vec0.y * 32767);
						
						if (hasuv) {
							var uv3 = m.get("mloopuv", loopstart + 3).get("uv");
							texa[tri * 6    ] = Std.int(uv2[0] * 32767);
							texa[tri * 6 + 1] = Std.int((1.0 - uv2[1]) * 32767);
							texa[tri * 6 + 2] = Std.int(uv3[0] * 32767);
							texa[tri * 6 + 3] = Std.int((1.0 - uv3[1]) * 32767);
							texa[tri * 6 + 4] = Std.int(uv0[0] * 32767);
							texa[tri * 6 + 5] = Std.int((1.0 - uv0[1]) * 32767);
						}
						tri++;
					}
				}
			}

			// Pack positions to (-1, 1) range
			var hx = 0.0;
			var hy = 0.0;
			var hz = 0.0;
			for (i in 0...Std.int(posa32.length / 3)) {
				var f = Math.abs(posa32[i * 3]);
				if (hx < f) hx = f;
				f = Math.abs(posa32[i * 3 + 1]);
				if (hy < f) hy = f;
				f = Math.abs(posa32[i * 3 + 2]);
				if (hz < f) hz = f;
			}
			var scalePos = Math.max(hx, Math.max(hy, hz));
			var inv = 1 / scalePos;
			for (i in 0...Std.int(posa32.length / 3)) {
				posa[i * 4    ] = Std.int(posa32[i * 3    ] * 32767 * inv);
				posa[i * 4 + 1] = Std.int(posa32[i * 3 + 1] * 32767 * inv);
				posa[i * 4 + 2] = Std.int(posa32[i * 3 + 2] * 32767 * inv);
			}

			var name:String = m.get("id").get("name");
			name = name.substring(2, name.length);
			var obj = {posa: posa, nora: nora, texa: texa, inda: inda, name: name, scalePos: scalePos, scaleTes: 1.0};
			Importer.makeMesh(obj, path);
			iron.data.Data.deleteBlob(path);
		});
	}

	public static function runMaterial(path:String) {
		iron.data.Data.getBlob(path, function(b:kha.Blob) {
			var bl = new iron.format.blend.Blend(b);
			if (bl.dna == null) {
				UITrait.inst.showError("Error: Compressed blend");
				return;
			}

			var mats = bl.get("Material");
			if (mats.length == 0) {
				UITrait.inst.showError("Error: No materials found");
				return;
			}

			for (mat in mats) {
				// Material slot
				UITrait.inst.selectedMaterial = new MaterialSlot();
				UITrait.inst.materials.push(UITrait.inst.selectedMaterial);
				UINodes.inst.updateCanvasMap();
				var nodes = UINodes.inst.nodes;
				var canvas = UINodes.inst.canvas;
				var nout:TNode = null;
				for (n in canvas.nodes) if (n.type == "OUTPUT_MATERIAL_PBR") { nout = n; break; }
				for (n in canvas.nodes) if (n.name == "RGB") { nodes.removeNode(n, canvas); break; }

				// Parse nodetree
				var nodetree = mat.get("nodetree"); // bNodeTree
				var blnodes = nodetree.get("nodes"); // ListBase
				var bllinks = nodetree.get("links"); // bNodeLink

				// Look for Principled BSDF node
				var node:Dynamic = blnodes.get("first", 0, "bNode");
				var last = blnodes.get("last", 0, "bNode");
				while (true) {
					if (node.get("idname") == "ShaderNodeBsdfPrincipled") break;
					if (node.get("name") == last.get("name")) break;
					node = node.get("next");
				}
				if (node.get("idname") != "ShaderNodeBsdfPrincipled") {
					UITrait.inst.showError("Error: No Principled BSDF node found");
					continue;
				}

				// Use Principled BSDF as material output
				nout.name = node.get("name");
				nout.x = node.get("locx") + 400;
				nout.y = -node.get("locy") + 400;

				// Place nodes
				var node:Dynamic = blnodes.get("first", 0, "bNode");
				while (true) {
					// Search for node in list
					var search = node.get("idname").substr(10).toLowerCase();
					var base:TNode = null;
					for (list in NodesMaterial.list) {
						var found = false;
						for (n in list) {
							var s = StringTools.replace(n.type, "_", "").toLowerCase();
							if (search == s) {
								base = n;
								found = true;
								break;
							}
						}
						if (found) break;
					}

					if (base != null) {
						var n = UINodes.makeNode(base, nodes, canvas);
						n.x = node.get("locx") + 400;
						n.y = -node.get("locy") + 400;
						n.name = node.get("name");

						// Fill input socket values
						var inputs = node.get("inputs");
						var sock:Dynamic = inputs.get("first", 0, "bNodeSocket");
						var pos = 0;
						while (true) {
							if (pos >= n.inputs.length) break;
							n.inputs[pos].default_value = readBlendSocket(sock);

							var last = sock;
							sock = sock.get("next");
							if (last.block == sock.block) break;
							pos++;
						}

						// Fill output socket values
						if (search == "teximage") {
							var img = node.get("id", 0, "Image");
							var file = img.get("name").substr(2); // '//desktop\logo.png'
							file = StringTools.replace(file, "\\", "/");
							file = Path.baseDir(path) + file;
							arm.io.ImportTexture.run(file);
							n.buttons[0].default_value = arm.App.getAssetIndex(file);
							n.buttons[0].data = arm.App.mapEnum(arm.App.getEnumTexts()[n.buttons[0].default_value]);
						}
						var outputs = node.get("outputs");
						var sock:Dynamic = outputs.get("first", 0, "bNodeSocket");
						var pos = 0;
						while (true) {
							if (pos >= n.outputs.length) break;
							n.outputs[pos].default_value = readBlendSocket(sock);

							var last = sock;
							sock = sock.get("next");
							if (last.block == sock.block) break;
							pos++;
						}
						
						canvas.nodes.push(n);
					}

					if (node.get("name") == last.get("name")) break;
					node = node.get("next");
				}

				// Place links
				var link:Dynamic = bllinks.get("first", 0, "bNodeLink");
				while (true) {
					var fromnode = link.get("fromnode").get("name");
					var tonode = link.get("tonode").get("name");
					var fromsock = link.get("fromsock");
					var tosock = link.get("tosock");

					var from_id = -1;
					var to_id = -1;
					for (n in canvas.nodes) if (n.name == fromnode) { from_id = n.id; break; }
					for (n in canvas.nodes) if (n.name == tonode) { to_id = n.id; break; }

					if (from_id >= 0 && to_id >= 0) {
						var from_socket = 0;
						var sock:Dynamic = fromsock;
						while (true) {
							var last = sock;
							sock = sock.get("prev");
							if (last.block == sock.block) break;
							from_socket++;
						}

						var to_socket = 0;
						var sock:Dynamic = tosock;
						while (true) {
							var last = sock;
							sock = sock.get("prev");
							if (last.block == sock.block) break;
							to_socket++;
						}

						var valid = true;

						// Remap principled
						if (tonode == nout.name) {
							if (to_socket == 0) to_socket = 0; // Base
							else if (to_socket == 18) to_socket = 1; // Opac
							else if (to_socket == 7) to_socket = 3; // Rough
							else if (to_socket == 4) to_socket = 4; // Met
							else if (to_socket == 19) to_socket = 5; // TODO: auto-remove normal_map node
							else if (to_socket == 17) to_socket = 6; // Emis
							else if (to_socket == 1) to_socket = 8; // Subs
							else valid = false;
						}

						if (valid) {
							var raw:TNodeLink = {
								id: nodes.getLinkId(canvas.links),
								from_id: from_id,
								from_socket: from_socket,
								to_id: to_id,
								to_socket: to_socket
							};
							canvas.links.push(raw);
						}
					}

					var last = link;
					link = link.get("next");
					if (last.block == link.block) break;
				}

				arm.nodes.MaterialParser.parsePaintMaterial();
				RenderUtil.makeMaterialPreview();
			}

			UITrait.inst.hwnd1.redraws = 2;
			iron.data.Data.deleteBlob(path);
		});
	}

	static function readBlendSocket(sock:Dynamic):Dynamic {
		var idname = sock.get("idname");
		if (StringTools.startsWith(idname, "NodeSocketVector")) {
			return sock.get("default_value", 0, "bNodeSocketValueVector").get("value");
		}
		else if (StringTools.startsWith(idname, "NodeSocketColor")) {
			return sock.get("default_value", 0, "bNodeSocketValueRGBA").get("value");
		}
		else if (StringTools.startsWith(idname, "NodeSocketFloat")) {
			return sock.get("default_value", 0, "bNodeSocketValueFloat").get("value");
		}
		else if (StringTools.startsWith(idname, "NodeSocketInt")) {
			return sock.get("default_value", 0, "bNodeSocketValueInt").get("value");
		}
		else if (StringTools.startsWith(idname, "NodeSocketBoolean")) {
			return sock.get("default_value", 0, "bNodeSocketValueBoolean").get("value");
		}
		else if (StringTools.startsWith(idname, "NodeSocketString")) {
			return sock.get("default_value", 0, "bNodeSocketValueString").get("value");
		}
		return null;
	}
}
