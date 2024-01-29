
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

		if (BrushOutputNode.inst == null) {
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				this.texpaint = RenderPath.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_nor";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				this.texpaint_nor = RenderPath.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_pack";
				t.width = Config.getTextureResX();
				t.height = Config.getTextureResY();
				t.format = "RGBA32";
				this.texpaint_pack = RenderPath.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_nor_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				this.texpaint_nor_empty = RenderPath.createRenderTarget(t).image;
			}
			{
				let t = new RenderTargetRaw();
				t.name = "texpaint_pack_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				this.texpaint_pack_empty = RenderPath.createRenderTarget(t).image;
			}
		}
		else {
			this.texpaint = BrushOutputNode.inst.texpaint;
			this.texpaint_nor = BrushOutputNode.inst.texpaint_nor;
			this.texpaint_pack = BrushOutputNode.inst.texpaint_pack;
		}

		BrushOutputNode.inst = this;
	}

	override getAsImage = (from: i32, done: (img: Image)=>void) => {
		this.inputs[from].getAsImage(done);
	}
}
