package arm.io;

import kha.Blob;
import iron.data.Data;
import zui.Nodes;
import arm.format.BlendParser;
import arm.ui.UIBase;
import arm.ui.UINodes;
import arm.shader.NodesMaterial;
import arm.shader.MakeMaterial;
import arm.sys.Path;
import arm.util.RenderUtil;
import arm.data.MaterialSlot;

class ImportBlendMaterial {

	public static function run(path: String) {
		Data.getBlob(path, function(b: Blob) {
			var bl = new BlendParser(b);
			if (bl.dna == null) {
				Console.error(Strings.error3());
				return;
			}

			var mats = bl.get("Material");
			if (mats.length == 0) {
				Console.error("Error: No materials found");
				return;
			}

			var imported: Array<MaterialSlot> = [];

			for (mat in mats) {
				// Material slot
				Context.raw.material = new MaterialSlot(Project.materials[0].data);
				Project.materials.push(Context.raw.material);
				imported.push(Context.raw.material);
				var nodes = Context.raw.material.nodes;
				var canvas = Context.raw.material.canvas;
				canvas.name = mat.get("id").get("name").substr(2); // MAWood
				var nout: TNode = null;
				for (n in canvas.nodes) {
					if (n.type == "OUTPUT_MATERIAL_PBR") {
						nout = n;
						break;
					}
				}
				for (n in canvas.nodes) {
					if (n.name == "RGB") {
						nodes.removeNode(n, canvas);
						break;
					}
				}

				// Parse nodetree
				var nodetree = mat.get("nodetree"); // bNodeTree
				var blnodes = nodetree.get("nodes"); // ListBase
				var bllinks = nodetree.get("links"); // bNodeLink

				// Look for Principled BSDF node
				var node: Dynamic = blnodes.get("first", 0, "bNode");
				var last = blnodes.get("last", 0, "bNode");
				while (true) {
					if (node.get("idname") == "ShaderNodeBsdfPrincipled") break;
					if (node.get("name") == last.get("name")) break;
					node = node.get("next");
				}
				if (node.get("idname") != "ShaderNodeBsdfPrincipled") {
					Console.error("Error: No Principled BSDF node found");
					continue;
				}

				// Use Principled BSDF as material output
				nout.name = node.get("name");
				nout.x = node.get("locx") + 400;
				nout.y = -node.get("locy") + 400;

				// Place nodes
				var node: Dynamic = blnodes.get("first", 0, "bNode");
				while (true) {
					// Search for node in list
					var search = node.get("idname").substr(10).toLowerCase();
					var base: TNode = null;
					for (list in NodesMaterial.list) {
						var found = false;
						for (n in list) {
							var s = n.type.replace("_", "").toLowerCase();
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
						var sock: Dynamic = inputs.get("first", 0, "bNodeSocket");
						var pos = 0;
						while (true) {
							if (pos >= n.inputs.length) break;
							n.inputs[pos].default_value = readBlendSocket(sock);

							var last = sock;
							sock = sock.get("next");
							if (last.block == sock.block) break;
							pos++;
						}

						// Fill button values
						if (search == "teximage") {
							var img = node.get("id", 0, "Image");
							var file: String = img.get("name").substr(2); // '//desktop\logo.png'
							file = Path.baseDir(path) + file;
							ImportTexture.run(file);
							var ar = file.split(Path.sep);
							var filename = ar[ar.length - 1];
							n.buttons[0].default_value = App.getAssetIndex(filename);
						}
						else if (search == "valtorgb") {
							var ramp: Dynamic = node.get("storage", 0, "ColorBand");
							n.buttons[0].data = ramp.get("ipotype") == 0 ? 0 : 1; // Linear / Constant
							var elems: Array<Array<Float>> = n.buttons[0].default_value;
							for (i in 0...ramp.get("tot")) {
								if (i >= elems.length) elems.push([1.0, 1.0, 1.0, 1.0, 0.0]);
								var cbdata: Dynamic = ramp.get("data", i, "CBData");
								elems[i][0] = Std.int(cbdata.get("r") * 100) / 100;
								elems[i][1] = Std.int(cbdata.get("g") * 100) / 100;
								elems[i][2] = Std.int(cbdata.get("b") * 100) / 100;
								elems[i][3] = Std.int(cbdata.get("a") * 100) / 100;
								elems[i][4] = Std.int(cbdata.get("pos") * 100) / 100;
							}
						}
						else if (search == "mixrgb" || search == "math") {
							n.buttons[0].default_value = node.get("custom1");
							n.buttons[1].default_value = node.get("custom2") & 2;
						}
						else if (search == "mapping") {
							var storage = node.get("storage", 0, "TexMapping");
							n.buttons[0].default_value = storage.get("loc");
							n.buttons[1].default_value = storage.get("rot");
							n.buttons[2].default_value = storage.get("size");
							// var mat = storage.get("mat"); float[4][4]
							// storage.flag & 1 // use_min
							// storage.flag & 2 // use_max
							// storage.min[0]
							// storage.min[1]
							// storage.min[2]
							// storage.max[0]
							// storage.max[1]
							// storage.max[2]
						}

						// Fill output socket values
						var outputs = node.get("outputs");
						var sock: Dynamic = outputs.get("first", 0, "bNodeSocket");
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
				var link: Dynamic = bllinks.get("first", 0, "bNodeLink");
				while (true) {
					var fromnode = link.get("fromnode").get("name");
					var tonode = link.get("tonode").get("name");
					var fromsock = link.get("fromsock");
					var tosock = link.get("tosock");

					var from_id = -1;
					var to_id = -1;
					for (n in canvas.nodes) {
						if (n.name == fromnode) {
							from_id = n.id;
							break;
						}
					}
					for (n in canvas.nodes) {
						if (n.name == tonode) {
							to_id = n.id;
							break;
						}
					}

					if (from_id >= 0 && to_id >= 0) {
						var from_socket = 0;
						var sock: Dynamic = fromsock;
						while (true) {
							var last = sock;
							sock = sock.get("prev");
							if (last.block == sock.block) break;
							from_socket++;
						}

						var to_socket = 0;
						var sock: Dynamic = tosock;
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
							var raw: TNodeLink = {
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
				History.newMaterial();
			}

			function _init() {
				for (m in imported) {
					Context.setMaterial(m);
					MakeMaterial.parsePaintMaterial();
					RenderUtil.makeMaterialPreview();
				}
			}
			iron.App.notifyOnInit(_init);

			UIBase.inst.hwnds[TabSidebar1].redraws = 2;
			Data.deleteBlob(path);
		});
	}

	static function readBlendSocket(sock: Dynamic): Dynamic {
		var idname = sock.get("idname");
		if (idname.startsWith("NodeSocketVector")) {
			var v: Dynamic = sock.get("default_value", 0, "bNodeSocketValueVector").get("value");
			v[0] = Std.int(v[0] * 100) / 100;
			v[1] = Std.int(v[1] * 100) / 100;
			v[2] = Std.int(v[2] * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketColor")) {
			var v: Dynamic = sock.get("default_value", 0, "bNodeSocketValueRGBA").get("value");
			v[0] = Std.int(v[0] * 100) / 100;
			v[1] = Std.int(v[1] * 100) / 100;
			v[2] = Std.int(v[2] * 100) / 100;
			v[3] = Std.int(v[3] * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketFloat")) {
			var v: Dynamic = sock.get("default_value", 0, "bNodeSocketValueFloat").get("value");
			v = Std.int(v * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketInt")) {
			return sock.get("default_value", 0, "bNodeSocketValueInt").get("value");
		}
		else if (idname.startsWith("NodeSocketBoolean")) {
			return sock.get("default_value", 0, "bNodeSocketValueBoolean").get("value");
		}
		else if (idname.startsWith("NodeSocketString")) {
			return sock.get("default_value", 0, "bNodeSocketValueString").get("value");
		}
		return null;
	}
}
