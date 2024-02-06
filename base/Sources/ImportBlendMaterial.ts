
///if (is_paint || is_sculpt)

class ImportBlendMaterial {

	static run = (path: string) => {
		data_get_blob(path, (b: ArrayBuffer) => {
			let bl = ParserBlend.init(b);
			if (bl.dna == null) {
				Console.error(Strings.error3());
				return;
			}

			let mats = ParserBlend.get(bl, "Material");
			if (mats.length == 0) {
				Console.error("Error: No materials found");
				return;
			}

			let imported: SlotMaterialRaw[] = [];

			for (let mat of mats) {
				// Material slot
				Context.raw.material = SlotMaterial.create(Project.materials[0].data);
				Project.materials.push(Context.raw.material);
				imported.push(Context.raw.material);
				let nodes = Context.raw.material.nodes;
				let canvas = Context.raw.material.canvas;
				canvas.name = BlHandle.get(BlHandle.get(mat, "id"), "name").substr(2); // MAWood
				let nout: zui_node_t = null;
				for (let n of canvas.nodes) {
					if (n.type == "OUTPUT_MATERIAL_PBR") {
						nout = n;
						break;
					}
				}
				for (let n of canvas.nodes) {
					if (n.name == "RGB") {
						zui_remove_node(n, canvas);
						break;
					}
				}

				// Parse nodetree
				let nodetree = BlHandle.get(mat, "nodetree"); // bNodeTree
				let blnodes = BlHandle.get(nodetree, "nodes"); // ListBase
				let bllinks = BlHandle.get(nodetree, "links"); // bNodeLink

				// Look for Principled BSDF node
				let node: any = BlHandle.get(blnodes, "first", 0, "bNode");
				let last = BlHandle.get(blnodes, "last", 0, "bNode");
				while (true) {
					if (BlHandle.get(node, "idname") == "ShaderNodeBsdfPrincipled") break;
					if (BlHandle.get(node, "name") == BlHandle.get(last, "name")) break;
					node = BlHandle.get(node, "next");
				}
				if (BlHandle.get(node, "idname") != "ShaderNodeBsdfPrincipled") {
					Console.error("Error: No Principled BSDF node found");
					continue;
				}

				// Use Principled BSDF as material output
				nout.name = BlHandle.get(node, "name");
				nout.x = BlHandle.get(node, "locx") + 400;
				nout.y = -BlHandle.get(node, "locy") + 400;

				// Place nodes
				node = BlHandle.get(blnodes, "first", 0, "bNode");
				while (true) {
					// Search for node in list
					let search = BlHandle.get(node, "idname").substr(10).toLowerCase();
					let base: zui_node_t = null;
					for (let list of NodesMaterial.list) {
						let found = false;
						for (let n of list) {
							let s = n.type.replaceAll("_", "").toLowerCase();
							if (search == s) {
								base = n;
								found = true;
								break;
							}
						}
						if (found) break;
					}

					if (base != null) {
						let n = UINodes.makeNode(base, nodes, canvas);
						n.x = BlHandle.get(node, "locx") + 400;
						n.y = -BlHandle.get(node, "locy") + 400;
						n.name = BlHandle.get(node, "name");

						// Fill input socket values
						let inputs = BlHandle.get(node, "inputs");
						let sock: any = BlHandle.get(inputs, "first", 0, "bNodeSocket");
						let pos = 0;
						while (true) {
							if (pos >= n.inputs.length) break;
							n.inputs[pos].default_value = ImportBlendMaterial.readBlendSocket(sock);

							let last = sock;
							sock = BlHandle.get(sock, "next");
							if (last.block == sock.block) break;
							pos++;
						}

						// Fill button values
						if (search == "teximage") {
							let img = BlHandle.get(node, "id", 0, "Image");
							let file: string = BlHandle.get(img, "name").substr(2); // '//desktop\logo.png'
							file = Path.baseDir(path) + file;
							ImportTexture.run(file);
							let ar = file.split(Path.sep);
							let filename = ar[ar.length - 1];
							n.buttons[0].default_value = Base.getAssetIndex(filename);
						}
						else if (search == "valtorgb") {
							let ramp: any = BlHandle.get(node, "storage", 0, "ColorBand");
							n.buttons[0].data = BlHandle.get(ramp, "ipotype") == 0 ? 0 : 1; // Linear / Constant
							let elems: f32[][] = n.buttons[0].default_value;
							for (let i = 0; i < BlHandle.get(ramp, "tot"); ++i) {
								if (i >= elems.length) elems.push([1.0, 1.0, 1.0, 1.0, 0.0]);
								let cbdata: any = BlHandle.get(ramp, "data", i, "CBData");
								elems[i][0] = Math.floor(BlHandle.get(cbdata, "r") * 100) / 100;
								elems[i][1] = Math.floor(BlHandle.get(cbdata, "g") * 100) / 100;
								elems[i][2] = Math.floor(BlHandle.get(cbdata, "b") * 100) / 100;
								elems[i][3] = Math.floor(BlHandle.get(cbdata, "a") * 100) / 100;
								elems[i][4] = Math.floor(BlHandle.get(cbdata, "pos") * 100) / 100;
							}
						}
						else if (search == "mixrgb" || search == "math") {
							n.buttons[0].default_value = BlHandle.get(node, "custom1");
							n.buttons[1].default_value = BlHandle.get(node, "custom2") & 2;
						}
						else if (search == "mapping") {
							let storage = BlHandle.get(node, "storage", 0, "TexMapping");
							n.buttons[0].default_value = BlHandle.get(storage, "loc");
							n.buttons[1].default_value = BlHandle.get(storage, "rot");
							n.buttons[2].default_value = BlHandle.get(storage, "size");
							// let mat = BlHandle.get(storage, "mat"); float[4][4]
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
						let outputs = BlHandle.get(node, "outputs");
						sock = BlHandle.get(outputs, "first", 0, "bNodeSocket");
						pos = 0;
						while (true) {
							if (pos >= n.outputs.length) break;
							n.outputs[pos].default_value = ImportBlendMaterial.readBlendSocket(sock);

							let last = sock;
							sock = BlHandle.get(sock, "next");
							if (last.block == sock.block) break;
							pos++;
						}

						canvas.nodes.push(n);
					}

					if (BlHandle.get(node, "name") == BlHandle.get(last, "name")) break;
					node = BlHandle.get(node, "next");
				}

				// Place links
				let link: any = BlHandle.get(bllinks, "first", 0, "bNodeLink");
				while (true) {
					let fromnode = BlHandle.get(BlHandle.get(link, "fromnode"), "name");
					let tonode = BlHandle.get(BlHandle.get(link, "tonode"), "name");
					let fromsock = BlHandle.get(link, "fromsock");
					let tosock = BlHandle.get(link, "tosock");

					let from_id = -1;
					let to_id = -1;
					for (let n of canvas.nodes) {
						if (n.name == fromnode) {
							from_id = n.id;
							break;
						}
					}
					for (let n of canvas.nodes) {
						if (n.name == tonode) {
							to_id = n.id;
							break;
						}
					}

					if (from_id >= 0 && to_id >= 0) {
						let from_socket = 0;
						let sock: any = fromsock;
						while (true) {
							let last = sock;
							sock = BlHandle.get(sock, "prev");
							if (last.block == sock.block) break;
							from_socket++;
						}

						let to_socket = 0;
						sock = tosock;
						while (true) {
							let last = sock;
							sock = BlHandle.get(sock, "prev");
							if (last.block == sock.block) break;
							to_socket++;
						}

						let valid = true;

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
							let raw: zui_node_link_t = {
								id: zui_get_link_id(canvas.links),
								from_id: from_id,
								from_socket: from_socket,
								to_id: to_id,
								to_socket: to_socket
							};
							canvas.links.push(raw);
						}
					}

					let last = link;
					link = BlHandle.get(link, "next");
					if (last.block == link.block) break;
				}
				History.newMaterial();
			}

			let _init = () => {
				for (let m of imported) {
					Context.setMaterial(m);
					MakeMaterial.parsePaintMaterial();
					UtilRender.makeMaterialPreview();
				}
			}
			app_notify_on_init(_init);

			UIBase.hwnds[TabArea.TabSidebar1].redraws = 2;
			data_delete_blob(path);
		});
	}

	static readBlendSocket = (sock: any): any => {
		let idname = BlHandle.get(sock, "idname");
		if (idname.startsWith("NodeSocketVector")) {
			let v: any = BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueVector"), "value");
			v[0] = Math.floor(v[0] * 100) / 100;
			v[1] = Math.floor(v[1] * 100) / 100;
			v[2] = Math.floor(v[2] * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketColor")) {
			let v: any = BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueRGBA"), "value");
			v[0] = Math.floor(v[0] * 100) / 100;
			v[1] = Math.floor(v[1] * 100) / 100;
			v[2] = Math.floor(v[2] * 100) / 100;
			v[3] = Math.floor(v[3] * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketFloat")) {
			let v: any = BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueFloat"), "value");
			v = Math.floor(v * 100) / 100;
			return v;
		}
		else if (idname.startsWith("NodeSocketInt")) {
			return BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueInt"), "value");
		}
		else if (idname.startsWith("NodeSocketBoolean")) {
			return BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueBoolean"), "value");
		}
		else if (idname.startsWith("NodeSocketString")) {
			return BlHandle.get(BlHandle.get(sock, "default_value", 0, "bNodeSocketValueString"), "value");
		}
		return null;
	}
}

///end
