package arm.io;

import kha.Image;
import kha.Blob;
import kha.graphics4.TextureFormat;
import kha.graphics4.PipelineState;
import kha.graphics4.VertexStructure;
import kha.graphics4.VertexData;
import kha.graphics4.TextureUnit;
import kha.graphics4.ConstantLocation;
import kha.arrays.Float32Array;
import iron.data.Data;
import iron.data.ConstData;
import iron.Scene;
import arm.sys.Path;

class ImportEnvmap {

	static var pipeline: PipelineState = null;
	static var paramsLocation: ConstantLocation;
	static var params = new iron.math.Vec4();
	static var radianceLocation: TextureUnit;
	static var radiance: Image = null;
	static var mips: Array<Image> = null;

	public static function run(path: String, image: Image) {

		// Init
		if (pipeline == null) {
			pipeline = new PipelineState();
			pipeline.vertexShader = Reflect.field(kha.Shaders, "pass_vert");
			pipeline.fragmentShader = Reflect.field(kha.Shaders, "prefilter_envmap_frag");
			var vs = new VertexStructure();
			vs.add("pos", VertexData.Float2);
			pipeline.inputLayout = [vs];
			pipeline.compile();
			paramsLocation = pipeline.getConstantLocation("params");
			radianceLocation = pipeline.getTextureUnit("radiance");

			radiance = Image.createRenderTarget(512, 256, TextureFormat.RGBA64);

			mips = [];
			var w = 256;
			for (i in 0...9) {
				mips.push(Image.createRenderTarget(w, w > 1 ? Std.int(w / 2) : 1, TextureFormat.RGBA64));
				w = Std.int(w / 2);
			}

			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		}

		// Down-scale to 512x256
		radiance.g2.begin(false);
		radiance.g2.drawImage(image, 0, 0);
		radiance.g2.end();

		var radianceCpu = Image.fromBytes(radiance.getPixels(), radiance.width, radiance.height, TextureFormat.RGBA64, kha.graphics4.Usage.DynamicUsage);

		// Radiance
		var mipsCpu: Array<Image> = [];
		for (i in 0...mips.length) {
			getRadianceMip(mips[i], i, radiance);
			mipsCpu.push(Image.fromBytes(mips[i].getPixels(), mips[i].width, mips[i].height, TextureFormat.RGBA64, kha.graphics4.Usage.DynamicUsage));
		}
		radianceCpu.setMipmaps(mipsCpu);

		// Irradiance
		Scene.active.world.probe.irradiance = getSphericalHarmonics(radiance);

		// World
		Scene.active.world.probe.raw.strength = 1.0;
		Scene.active.world.envmap = image;
		Scene.active.world.raw.envmap = path;
		Scene.active.world.probe.radiance = radianceCpu;
		Scene.active.world.probe.radianceMipmaps = mips;
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
		params.x = level;
		mip.g4.setFloat4(paramsLocation, params.x, params.y, params.z, params.w);
		mip.g4.setTexture(radianceLocation, radiance);
		mip.g4.drawIndexedVertices();
		mip.g4.end();
	}

	static function getSphericalHarmonics(source: kha.Image) {
		var sh = new Float32Array(9 * 3 + 1); // Align to mult of 4 - 27->28
		return sh;
	}
}
