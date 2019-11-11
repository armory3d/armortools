package arm.ui;

import haxe.io.Bytes;
import kha.Image;
import zui.Id;
import zui.Ext;
import iron.Scene;
import arm.io.Importer;
import arm.nodes.MaterialParser;
import arm.util.UVUtil;
using StringTools;

class TabViewport {

	public static function draw() {
		var ui = UITrait.inst.ui;
		if (ui.tab(UITrait.inst.htab2, "Viewport")) {
			ui.row([1/4]);
			if (ui.button("Import")) {
				UIFiles.show = true;
				UIFiles.isSave = false;
				UIFiles.filters = "hdr";
				UIFiles.filesDone = function(path:String) {
					if (!path.endsWith(".hdr")) {
						Log.showError("Error: .hdr file expected");
						return;
					}
					Importer.run(path);
				}
			}

			if (Scene.active.world.probe.radianceMipmaps.length > 0) {
				ui.image(Scene.active.world.probe.radianceMipmaps[0]);
			}

			ui.row([1/2, 1/2]);
			var modeHandle = Id.handle({position: 0});
			UITrait.inst.viewportMode = ui.combo(modeHandle, ["Render", "Base Color", "Normal Map", "Occlusion", "Roughness", "Metallic", "TexCoord", "Normal", "MaterialID", "Mask"], "Mode");
			if (modeHandle.changed) {
				MaterialParser.parseMeshMaterial();
			}
			var p = Scene.active.world.probe;
			var envHandle = Id.handle();
			envHandle.value = p.raw.strength;
			p.raw.strength = ui.slider(envHandle, "Environment", 0.0, 8.0, true);
			if (envHandle.changed) Context.ddirty = 2;

			#if (!kha_direct3d12)
			ui.row([1/2, 1/2]);
			if (Scene.active.lights.length > 0) {
				var light = Scene.active.lights[0];

				var sxhandle = Id.handle();
				sxhandle.value = light.data.raw.size;
				light.data.raw.size = ui.slider(sxhandle, "Light Size", 0.0, 4.0, true);
				if (sxhandle.changed) Context.ddirty = 2;
				// var syhandle = Id.handle();
				// syhandle.value = light.data.raw.size_y;
				// light.data.raw.size_y = ui.slider(syhandle, "Size Y", 0.0, 4.0, true);
				// if (syhandle.changed) Context.ddirty = 2;

				var lhandle = Id.handle();
				#if arm_world
				var scale = 1;
				#else
				var scale = 1333;
				#end
				lhandle.value = light.data.raw.strength / scale;
				lhandle.value = Std.int(lhandle.value * 100) / 100;
				light.data.raw.strength = ui.slider(lhandle, "Light", 0.0, 4.0, true) * scale;
				if (lhandle.changed) Context.ddirty = 2;
			}
			#end

			ui.row([1/2, 1/2]);
			var cam = Scene.active.camera.data.raw;
			var near_handle = Id.handle({value: cam.near_plane});
			var far_handle = Id.handle({value: cam.far_plane});
			near_handle.value = Std.int(near_handle.value * 1000) / 1000;
			far_handle.value = Std.int(far_handle.value * 100) / 100;
			ui.changed = false;
			cam.near_plane = ui.slider(near_handle, "Clip Start", 0.001, 1.0, true);
			cam.far_plane = ui.slider(far_handle, "Clip End", 50.0, 100.0, true);
			if (ui.changed) {
				Scene.active.camera.buildProjection();
			}

			var dispHandle = Id.handle({value: UITrait.inst.displaceStrength});
			UITrait.inst.displaceStrength = ui.slider(dispHandle, "Displace", 0.0, 2.0, true);
			if (dispHandle.changed) {
				MaterialParser.parseMeshMaterial();
			}

			ui.row([1/2, 1/2]);
			UITrait.inst.drawWireframe = ui.check(UITrait.inst.wireframeHandle, "Wireframe");
			if (UITrait.inst.wireframeHandle.changed) {
				ui.g.end();
				UVUtil.cacheUVMap();
				ui.g.begin(false);
				MaterialParser.parseMeshMaterial();
			}
			UITrait.inst.drawTexels = ui.check(UITrait.inst.texelsHandle, "Texels");
			if (UITrait.inst.texelsHandle.changed) {
				MaterialParser.parseMeshMaterial();
			}

			ui.row([1/2, 1/2]);
			UITrait.inst.showEnvmap = ui.check(UITrait.inst.showEnvmapHandle, "Envmap");
			if (UITrait.inst.showEnvmapHandle.changed) {
				var world = Scene.active.world;
				world.loadEnvmap(function(_) {});
				if (UITrait.inst.savedEnvmap == null) UITrait.inst.savedEnvmap = world.envmap;
				Context.ddirty = 2;
			}
			var compassHandle = Id.handle({selected: UITrait.inst.showCompass});
			UITrait.inst.showCompass = ui.check(compassHandle, "Compass");
			if (compassHandle.changed) Context.ddirty = 2;

			if (UITrait.inst.showEnvmap) {
				UITrait.inst.showEnvmapBlur = ui.check(UITrait.inst.showEnvmapBlurHandle, "Blurred");
				if (UITrait.inst.showEnvmapBlurHandle.changed) Context.ddirty = 2;
			}
			else {
				if (ui.panel(Id.handle({selected: false}), "Viewport Color")) {
					var hwheel = Id.handle({color: 0xff030303});
					var worldColor:kha.Color = Ext.colorWheel(ui, hwheel);
					if (hwheel.changed) {
						// var b = UITrait.inst.emptyEnvmap.lock(); // No lock for d3d11
						// b.set(0, worldColor.Rb);
						// b.set(1, worldColor.Gb);
						// b.set(2, worldColor.Bb);
						// UITrait.inst.emptyEnvmap.unlock();
						// UITrait.inst.emptyEnvmap.unload(); //
						var b = Bytes.alloc(4);
						b.set(0, worldColor.Rb);
						b.set(1, worldColor.Gb);
						b.set(2, worldColor.Bb);
						b.set(3, 255);
						UITrait.inst.emptyEnvmap = Image.fromBytes(b, 1, 1);
						Context.ddirty = 2;
					}
				}
			}

			if (UITrait.inst.showEnvmap) {
				Scene.active.world.envmap = UITrait.inst.showEnvmapBlur ? Scene.active.world.probe.radianceMipmaps[0] : UITrait.inst.savedEnvmap;
			}
			else {
				Scene.active.world.envmap = UITrait.inst.emptyEnvmap;
			}
		}
	}
}
