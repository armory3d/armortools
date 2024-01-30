
class ImportEnvmap {

	static pipeline: PipelineState = null;
	static paramsLocation: ConstantLocation;
	static params = new Vec4();
	static n = new Vec4();
	static radianceLocation: TextureUnit;
	static radiance: Image = null;
	static radianceCpu: Image = null;
	static mips: Image[] = null;
	static mipsCpu: Image[] = null;

	static run = (path: string, image: Image) => {

		// Init
		if (ImportEnvmap.pipeline == null) {
			ImportEnvmap.pipeline = new PipelineState();
			ImportEnvmap.pipeline.vertexShader = System.getShader("pass.vert");
			ImportEnvmap.pipeline.fragmentShader = System.getShader("prefilter_envmap.frag");
			let vs = new VertexStructure();
			vs.add("pos", VertexData.F32_2X);
			ImportEnvmap.pipeline.inputLayout = [vs];
			ImportEnvmap.pipeline.colorAttachmentCount = 1;
			ImportEnvmap.pipeline.colorAttachments[0] = TextureFormat.RGBA128;
			ImportEnvmap.pipeline.compile();
			ImportEnvmap.paramsLocation = ImportEnvmap.pipeline.getConstantLocation("params");
			ImportEnvmap.radianceLocation = ImportEnvmap.pipeline.getTextureUnit("radiance");

			ImportEnvmap.radiance = Image.createRenderTarget(1024, 512, TextureFormat.RGBA128);

			ImportEnvmap.mips = [];
			let w = 512;
			for (let i = 0; i < 10; ++i) {
				ImportEnvmap.mips.push(Image.createRenderTarget(w, w > 1 ? Math.floor(w / 2) : 1, TextureFormat.RGBA128));
				w = Math.floor(w / 2);
			}

			if (ConstData.screenAlignedVB == null) ConstData.createScreenAlignedData();
		}

		// Down-scale to 1024x512
		ImportEnvmap.radiance.g2.begin(false);
		ImportEnvmap.radiance.g2.pipeline = Base.pipeCopy128;
		ImportEnvmap.radiance.g2.drawScaledImage(image, 0, 0, 1024, 512);
		ImportEnvmap.radiance.g2.pipeline = null;
		ImportEnvmap.radiance.g2.end();

		let radiancePixels = ImportEnvmap.radiance.getPixels();
		if (ImportEnvmap.radianceCpu != null) {
			let _radianceCpu = ImportEnvmap.radianceCpu;
			Base.notifyOnNextFrame(() => {
				_radianceCpu.unload();
			});
		}
		ImportEnvmap.radianceCpu = Image.fromBytes(radiancePixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height, TextureFormat.RGBA128, Usage.DynamicUsage);

		// Radiance
		if (ImportEnvmap.mipsCpu != null) {
			for (let mip of ImportEnvmap.mipsCpu) {
				let _mip = mip;
				Base.notifyOnNextFrame(() => {
					///if (!krom_direct3d12) // TODO: crashes after 50+ imports
					_mip.unload();
					///end
				});
			}
		}
		ImportEnvmap.mipsCpu = [];
		for (let i = 0; i < ImportEnvmap.mips.length; ++i) {
			ImportEnvmap.getRadianceMip(ImportEnvmap.mips[i], i, ImportEnvmap.radiance);
			ImportEnvmap.mipsCpu.push(Image.fromBytes(ImportEnvmap.mips[i].getPixels(), ImportEnvmap.mips[i].width, ImportEnvmap.mips[i].height, TextureFormat.RGBA128, Usage.DynamicUsage));
		}
		ImportEnvmap.radianceCpu.setMipmaps(ImportEnvmap.mipsCpu);

		// Irradiance
		Scene.world._irradiance = ImportEnvmap.getSphericalHarmonics(radiancePixels, ImportEnvmap.radiance.width, ImportEnvmap.radiance.height);

		// World
		Scene.world.strength = 1.0;
		Scene.world.radiance_mipmaps = ImportEnvmap.mipsCpu.length - 2;
		Scene.world._envmap = image;
		Scene.world.envmap = path;
		Scene.world._radiance = ImportEnvmap.radianceCpu;
		Scene.world._radianceMipmaps = ImportEnvmap.mipsCpu;
		Context.raw.savedEnvmap = image;
		if (Context.raw.showEnvmapBlur) {
			Scene.world._envmap = Scene.world._radianceMipmaps[0];
		}
		Context.raw.ddirty = 2;
		Project.raw.envmap = path;
	}

	static getRadianceMip = (mip: Image, level: i32, radiance: Image) => {
		mip.g4.begin();
		mip.g4.setVertexBuffer(ConstData.screenAlignedVB);
		mip.g4.setIndexBuffer(ConstData.screenAlignedIB);
		mip.g4.setPipeline(ImportEnvmap.pipeline);
		ImportEnvmap.params.x = 0.1 + level / 8;
		mip.g4.setFloat4(ImportEnvmap.paramsLocation, ImportEnvmap.params.x, ImportEnvmap.params.y, ImportEnvmap.params.z, ImportEnvmap.params.w);
		mip.g4.setTexture(ImportEnvmap.radianceLocation, radiance);
		mip.g4.drawIndexedVertices();
		mip.g4.end();
	}

	static reverseEquirect = (x: f32, y: f32): Vec4 => {
		let theta = x * Math.PI * 2 - Math.PI;
		let phi = y * Math.PI;
		// return n.set(Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)), Math.cos(phi));
		return ImportEnvmap.n.set(-Math.cos(phi), Math.sin(phi) * Math.cos(theta), -(Math.sin(phi) * Math.sin(theta)));
	}

	// https://ndotl.wordpress.com/2015/03/07/pbr-cubemap-filtering
	// https://seblagarde.wordpress.com/2012/06/10/amd-cubemapgen-for-physically-based-rendering
	static getSphericalHarmonics = (source: ArrayBuffer, sourceWidth: i32, sourceHeight: i32): Float32Array => {
		let sh = new Float32Array(9 * 3 + 1); // Align to mult of 4 - 27->28
		let accum = 0.0;
		let weight = 1.0;
		let weight1 = weight * 4 / 17;
		let weight2 = weight * 8 / 17;
		let weight3 = weight * 15 / 17;
		let weight4 = weight * 5 / 68;
		let weight5 = weight * 15 / 68;
		let view = new DataView(source);

		for (let x = 0; x < sourceWidth; ++x) {
			for (let y = 0; y < sourceHeight; ++y) {
				ImportEnvmap.n = ImportEnvmap.reverseEquirect(x / sourceWidth, y / sourceHeight);

				for (let i = 0; i < 3; ++i) {
					let value = view.getFloat32(((x + y * sourceWidth) * 16 + i * 4), true);
					value = Math.pow(value, 1.0 / 2.2);

					sh[0 + i] += value * weight1;
					sh[3 + i] += value * weight2 * ImportEnvmap.n.x;
					sh[6 + i] += value * weight2 * ImportEnvmap.n.y;
					sh[9 + i] += value * weight2 * ImportEnvmap.n.z;

					sh[12 + i] += value * weight3 * ImportEnvmap.n.x * ImportEnvmap.n.z;
					sh[15 + i] += value * weight3 * ImportEnvmap.n.z * ImportEnvmap.n.y;
					sh[18 + i] += value * weight3 * ImportEnvmap.n.y * ImportEnvmap.n.x;

					sh[21 + i] += value * weight4 * (3.0 * ImportEnvmap.n.z * ImportEnvmap.n.z - 1.0);
					sh[24 + i] += value * weight5 * (ImportEnvmap.n.x * ImportEnvmap.n.x - ImportEnvmap.n.y * ImportEnvmap.n.y);

					accum += weight;
				}
			}
		}

		for (let i = 0; i < sh.length; ++i) {
			sh[i] /= accum / 16;
		}

		return sh;
	}
}
