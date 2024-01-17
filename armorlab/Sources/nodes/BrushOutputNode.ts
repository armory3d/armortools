
// @:keep
class BrushOutputNode extends LogicNode {

	id = 0;
	texpaint: Image = null;
	texpaint_nor: Image = null;
	texpaint_pack: Image = null;
	texpaint_nor_empty: Image = null;
	texpaint_pack_empty: Image = null;

	static inst: BrushOutputNode = null;

	constructor() {
		super();

		if (inst == null) {
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				texpaint = RenderPath.active.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_nor";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				texpaint_nor = RenderPath.active.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_pack";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				texpaint_pack = RenderPath.active.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_nor_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				texpaint_nor_empty = RenderPath.active.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_pack_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				texpaint_pack_empty = RenderPath.active.createRenderTarget(t).image;
			}
		}
		else {
			texpaint = inst.texpaint;
			texpaint_nor = inst.texpaint_nor;
			texpaint_pack = inst.texpaint_pack;
		}

		inst = this;
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		inputs[from].getAsImage(done);
	}
}
