package arm.io;

import kha.Image;
import kha.Blob;
import kha.graphics4.TextureFormat;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.TextureUnit;
import kha.graphics4.ConstantLocation;
import kha.graphics4.TextureAddressing;
import kha.graphics4.TextureFilter;
import kha.graphics4.MipMapFilter;
import kha.arrays.Float32Array;
import iron.data.Data;
import iron.data.ConstData;
import iron.math.Vec4;
import iron.Scene;
import arm.sys.Path;

class ImportEnvmap {

	static var pipeline: PipelineState = null;
	static var paramsLocation: ConstantLocation;
	static var params = new Vec4();
	static var n = new Vec4();
	static var radianceLocation: TextureUnit;
	static var radiance: Image = null;
	static var radianceCpu: Image = null;
	static var mips: Array<Image> = null;
	static var mipsCpu: Array<Image> = null;

	public static function run(path: String, image: Image) {

		// Init
		if (pipeline == null) {
			pipeline = new PipelineState();
			pipeline.vertexShader = kha.Shaders.getVertex("pass.vert");
			pipeline.fragmentShader = kha.Shaders.getFragment("prefilter_envmap.frag");
			var vs = new VertexStructure();
			vs.add("pos", VertexData.Float2);
			pipeline.inputLayout = [vs];
			pipeline.colorAttachmentCount = 1;
			pipeline.colorAttachments[0] = TextureFormat.RGBA128;
			pipeline.compile();
			paramsLocation = pipeline.getConstantLocation("params");
			radianceLocation = pipeline.getTextureUnit("radiance");

			radiance = Image.createRenderTarget(1024, 512, TextureFormat.RGBA128);

			mips = [];
			var w = 512;
			for (i in 0...10) {
				mips.push(Image.createRenderTarget(w, w > 1 ? Std.int(w / 2) : 1, TextureFormat.RGBA128));
				w = Std.int(w / 2);
			}

			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		}

		// Down-scale to 1024x512
		radiance.g2.begin(false);
		radiance.g2.pipeline = Layers.pipeCopy128;
		radiance.g2.drawScaledImage(image, 0, 0, 1024, 512);
		radiance.g2.pipeline = null;
		radiance.g2.end();

		var radiancePixels = radiance.getPixels();
		if (radianceCpu != null) radianceCpu.unload();
		radianceCpu = Image.fromBytes(radiancePixels, radiance.width, radiance.height, TextureFormat.RGBA128, kha.graphics4.Usage.DynamicUsage);

		// Radiance
		if (mipsCpu != null) for (mip in mipsCpu) mip.unload();
		mipsCpu = [];
		for (i in 0...mips.length) {
			getRadianceMip(mips[i], i, radiance);
			mipsCpu.push(Image.fromBytes(mips[i].getPixels(), mips[i].width, mips[i].height, TextureFormat.RGBA128, kha.graphics4.Usage.DynamicUsage));
		}
		radianceCpu.setMipmaps(mipsCpu);

		// Irradiance
		Scene.active.world.probe.irradiance = getSphericalHarmonics(radiancePixels, radiance.width, radiance.height);

		// World
		Scene.active.world.probe.raw.strength = 1.0;
		Scene.active.world.probe.raw.radiance_mipmaps = mipsCpu.length - 2;
		Scene.active.world.envmap = image;
		Scene.active.world.raw.envmap = path;
		Scene.active.world.probe.radiance = radianceCpu;
		Scene.active.world.probe.radianceMipmaps = mipsCpu;
		Context.savedEnvmap = image;
		Context.showEnvmapHandle.selected = Context.showEnvmap = true;
		if (Context.showEnvmapBlur) {
			Scene.active.world.envmap = Scene.active.world.probe.radianceMipmaps[0];
		}
		Context.ddirty = 2;
	}

	static function getRadianceMip(mip: kha.Image, level: Int, radiance: kha.Image) {
		mip.g4.begin();
		mip.g4.setVertexBuffer(ConstData.screenAlignedVB);
		mip.g4.setIndexBuffer(ConstData.screenAlignedIB);
		mip.g4.setPipeline(pipeline);
		params.x = 0.1 + level / 8;
		mip.g4.setFloat4(paramsLocation, params.x, params.y, params.z, params.w);
		mip.g4.setTexture(radianceLocation, radiance);
		mip.g4.drawIndexedVertices();
		mip.g4.end();
	}

	static function reverseEquirect(x: Float, y: Float): Vec4 {
		var theta = x * Math.PI * 2 - Math.PI;
		var phi = y * Math.PI;
		// return n.set(Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)), Math.cos(phi));
		return n.set(-Math.cos(phi), Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)));
	}

	// https://ndotl.wordpress.com/2015/03/07/pbr-cubemap-filtering
	// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering
	static function getSphericalHarmonics(source: haxe.io.Bytes, sourceWidth: Int, sourceHeight: Int): Float32Array {
		var sh = new Float32Array(9 * 3 + 1); // Align to mult of 4 - 27->28
		var accum = 0.0;
		var weight = 1.0;
		var weight1 = weight * 4 / 17;
		var weight2 = weight * 8 / 17;
		var weight3 = weight * 15 / 17;
		var weight4 = weight * 5 / 68;
		var weight5 = weight * 15 / 68;

		for (x in 0...sourceWidth) {
			for (y in 0...sourceHeight) {
				n = reverseEquirect(x / sourceWidth, y / sourceHeight);

				for (i in 0...3) {
					var value = source.getFloat(((x + y * sourceWidth) * 16 + i * 4));
					value = Math.pow(value, 1.0 / 2.2);

					sh[0 + i] += value * weight1;
					sh[3 + i] += value * weight2 * n.x;
					sh[6 + i] += value * weight2 * n.y;
					sh[9 + i] += value * weight2 * n.z;

					sh[12 + i] += value * weight3 * n.x * n.z;
					sh[15 + i] += value * weight3 * n.z * n.y;
					sh[18 + i] += value * weight3 * n.y * n.x;

					sh[21 + i] += value * weight4 * (3.0 * n.z * n.z - 1.0);
					sh[24 + i] += value * weight5 * (n.x * n.x - n.y * n.y);

					accum += weight;
				}
			}
		}

		for (i in 0...sh.length) {
			sh[i] /= accum / 16;
		}

		return sh;
	}
}
