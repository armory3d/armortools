
let plugin = new arm.Plugin();

let h1 = new zui.Handle();

plugin.drawUI = function(ui) {
	if (ui.panel(h1, "Converter")) {
		ui.row([1/2, 1/2]);
		if (ui.button(".arm to .json")) {
			arm.UIFiles.show = true;
			arm.UIFiles.isSave = false;
			arm.UIFiles.filters = "arm";
			arm.UIFiles.filesDone = function(path) {
				iron.Data.getBlob(path, function(b) {
					let parsed = iron.ArmPack.decode(b.bytes);
					let out = arm.Bytes.ofString(arm.Json.stringify(parsed, function(key, value) {
						if (arm.StdIs(value, Float32Array)) {
							let ar = Array.from(value);
							ar.unshift(0); // Annotate array type
							return ar;
						}
						else if (arm.StdIs(value, Uint32Array)) {
							let ar = Array.from(value);
							ar.unshift(1);
							return ar;
						}
						else if (arm.StdIs(value, Int16Array)) {
							let ar = Array.from(value);
							ar.unshift(2);
							return ar;
						}
						return value;
					}, "	")).b.bufferValue;
					Krom.fileSaveBytes(path.substr(0, path.length - 3) + "json", out);
				});
			}
		}
		if (ui.button(".json to .arm")) {
			arm.UIFiles.show = true;
			arm.UIFiles.isSave = false;
			arm.UIFiles.filters = "json";
			arm.UIFiles.filesDone = function(path) {
				iron.Data.getBlob(path, function(b) {
					let parsed = arm.Json.parse(b.toString());
					function iterate(d) {
						for (const n of arm.ReflectFields(d)) {
							let v = arm.ReflectField(d, n);
							if (arm.StdIs(v, Array)) {
								if (arm.StdIs(v[0], Number)) {
									let ar = null;
									if (v[0] === 0) ar = new Float32Array(v.length - 1);
									else if (v[0] === 1) ar = new Uint32Array(v.length - 1);
									else if (v[0] === 2) ar = new Int16Array(v.length - 1);
									for (let i = 0; i < v.length - 1; ++i) ar[i] = v[i + 1];
									arm.ReflectSetField(d, n, ar);
								}
								else for (const e of v) if (arm.TypeOf(e) === arm.TObject) iterate(e);
							}
							else if (arm.TypeOf(v) === arm.TObject) iterate(v);
						}
					}
					iterate(parsed);
					let out = iron.ArmPack.encode(parsed).b.bufferValue;
					Krom.fileSaveBytes(path.substr(0, path.length - 4) + "arm", out);
				});
			}
		}
	}
}
