
class BrushOutputNode extends LogicNode {

	id = 0;
	texpaint: image_t = null;
	texpaint_nor: image_t = null;
	texpaint_pack: image_t = null;
	texpaint_nor_empty: image_t = null;
	texpaint_pack_empty: image_t = null;

	static inst: BrushOutputNode = null;

	constructor() {
		super();

		if (BrushOutputNode.inst == null) {
			{
				let t = render_target_create();
				t.name = "texpaint";
				t.width = config_get_texture_res_x();
				t.height = config_get_texture_res_y();
				t.format = "RGBA32";
				this.texpaint = render_path_create_render_target(t)._image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_nor";
				t.width = config_get_texture_res_x();
				t.height = config_get_texture_res_y();
				t.format = "RGBA32";
				this.texpaint_nor = render_path_create_render_target(t)._image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_pack";
				t.width = config_get_texture_res_x();
				t.height = config_get_texture_res_y();
				t.format = "RGBA32";
				this.texpaint_pack = render_path_create_render_target(t)._image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_nor_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				this.texpaint_nor_empty = render_path_create_render_target(t)._image;
			}
			{
				let t = render_target_create();
				t.name = "texpaint_pack_empty";
				t.width = 1;
				t.height = 1;
				t.format = "RGBA32";
				this.texpaint_pack_empty = render_path_create_render_target(t)._image;
			}
		}
		else {
			this.texpaint = BrushOutputNode.inst.texpaint;
			this.texpaint_nor = BrushOutputNode.inst.texpaint_nor;
			this.texpaint_pack = BrushOutputNode.inst.texpaint_pack;
		}

		BrushOutputNode.inst = this;
	}

	override get_as_image = (from: i32, done: (img: image_t)=>void) => {
		this.inputs[from].get_as_image(done);
	}
}
