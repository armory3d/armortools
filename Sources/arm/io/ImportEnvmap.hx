package arm.io;

import kha.Image;
import kha.Blob;
import kha.graphics4.TextureFormat;
import kha.arrays.Float32Array;
import iron.data.Data;
import iron.Scene;
import arm.ui.UITrait;
using StringTools;

class ImportEnvmap {

	public static function run(path: String, image: Image) {
		var p = Krom.getFilesLocation() + "/" + Data.dataPath;
		#if krom_windows
		var cmft = p + "/cmft.exe";
		#elseif krom_linux
		var cmft = p + "/cmft-linux64";
		#else
		var cmft = p + "/cmft-osx";
		#end

		var tmp = Krom.getFilesLocation() + "/" + Data.dataPath;

		// Irr
		var cmd = cmft;
		cmd += ' --input "' + path + '"';
		cmd += " --filter shcoeffs";
		cmd += " --outputNum 1";
		cmd += ' --output0 "' + tmp + 'tmp_irr"';
		#if krom_windows
		cmd = cmd.replace("/", "\\");
		#end
		Krom.sysCommand(cmd);

		// Rad
		var faceSize = Std.int(image.width / 8);
		cmd = cmft;
		cmd += ' --input "' + path + '"';
		cmd += " --filter radiance";
		cmd += " --dstFaceSize " + faceSize;
		cmd += " --srcFaceSize " + faceSize;
		cmd += " --excludeBase false";
		cmd += " --glossScale 8";
		cmd += " --glossBias 3";
		cmd += " --lightingModel blinnbrdf";
		cmd += " --edgeFixup none";
		cmd += " --numCpuProcessingThreads 4";
		cmd += " --useOpenCL true";
		cmd += " --clVendor anyGpuVendor";
		cmd += " --deviceType gpu";
		cmd += " --deviceIndex 0";
		cmd += " --generateMipChain true";
		cmd += " --inputGammaNumerator 1.0";
		cmd += " --inputGammaDenominator 2.2";
		cmd += " --outputGammaNumerator 1.0";
		cmd += " --outputGammaDenominator 1.0";
		cmd += " --outputNum 1";
		cmd += ' --output0 "' + tmp + 'tmp_rad"';
		cmd += " --output0params hdr,rgbe,latlong";
		#if krom_windows
		cmd = cmd.replace("/", "\\");
		#end
		Krom.sysCommand(cmd);

		// Load irr
		Data.getBlob("tmp_irr.c", function(blob: Blob) {
			var lines = blob.toString().split("\n");
			var band0 = lines[5];
			var band1 = lines[6];
			var band2 = lines[7];
			band0 = band0.substring(band0.indexOf("{"), band0.length);
			band1 = band1.substring(band1.indexOf("{"), band1.length);
			band2 = band2.substring(band2.indexOf("{"), band2.length);
			var band = band0 + band1 + band2;
			band = band.replace("{", "");
			band = band.replace("}", "");
			var ar = band.split(",");
			var buf = new Float32Array(28); // Align to mult of 4 - 27->28
			for (i in 0...27) buf[i] = Std.parseFloat(ar[i]);
			Scene.active.world.probe.irradiance = buf;
			Context.ddirty = 2;
			Data.deleteBlob("tmp_irr.c");
		});

		// World envmap
		Scene.active.world.probe.raw.strength = 1.0;
		Scene.active.world.envmap = image;
		Scene.active.world.raw.envmap = path;
		UITrait.inst.savedEnvmap = image;
		UITrait.inst.showEnvmapHandle.selected = UITrait.inst.showEnvmap = true;

		// Load envmap clone and set mipmaps
		Data.cachedImages.remove(path);
		@:privateAccess Data.loadingImages.remove(path);
		Data.getImage(path, function(image: kha.Image) {

			// Load mips
			var mipsCount = 6 + Std.int(image.width / 1024);
			var mipsLoaded = 0;
			var mips: Array<Image> = [];
			while (mips.length < mipsCount + 2) mips.push(null);
			var mw = Std.int(image.width / 2);
			var mh = Std.int(image.width / 4);
			for (i in 0...mipsCount) {
				Data.getImage("tmp_rad_" + i + "_" + mw + "x" + mh + ".hdr", function(mip: Image) {
					mips[i] = mip;
					mipsLoaded++;
					if (mipsLoaded == mipsCount) {
						// 2x1 and 1x1 mips
						var b2x1 = haxe.io.Bytes.alloc(2 * 1 * 4 * 4); // w * h * channels * format_size
						var b1x1 = haxe.io.Bytes.alloc(1 * 1 * 4 * 4);
						mips[mipsCount] = Image.fromBytes(b2x1, 2, 1, TextureFormat.RGBA128, kha.graphics4.Usage.DynamicUsage);
						mips[mipsCount + 1] = Image.fromBytes(b1x1, 1, 1, TextureFormat.RGBA128, kha.graphics4.Usage.DynamicUsage);
						// Set radiance
						image.setMipmaps(mips);
						Scene.active.world.probe.radiance = image;
						Scene.active.world.probe.radianceMipmaps = mips;
						Context.ddirty = 2;
					}
				}, true); // Readable
				mw = Std.int(mw / 2);
				mh = Std.int(mh / 2);
			}
		});
	}
}
