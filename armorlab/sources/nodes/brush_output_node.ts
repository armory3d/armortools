
type brush_output_node_t = {
	base?: logic_node_t;
	id?: i32;
	texpaint?: image_t;
	texpaint_nor?: image_t;
	texpaint_pack?: image_t;
	texpaint_nor_empty?: image_t;
	texpaint_pack_empty?: image_t;
};

let brush_output_node_inst: brush_output_node_t = null;

function brush_output_node_create(arg: any): brush_output_node_t {
	let n: brush_output_node_t = {};
	n.base = logic_node_create();

	if (brush_output_node_inst == null) {
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_nor";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint_nor = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_pack";
			t.width = config_get_texture_res_x();
			t.height = config_get_texture_res_y();
			t.format = "RGBA32";
			n.texpaint_pack = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_nor_empty";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			n.texpaint_nor_empty = render_path_create_render_target(t)._image;
		}
		{
			let t: render_target_t = render_target_create();
			t.name = "texpaint_pack_empty";
			t.width = 1;
			t.height = 1;
			t.format = "RGBA32";
			n.texpaint_pack_empty = render_path_create_render_target(t)._image;
		}
	}
	else {
		n.texpaint = brush_output_node_inst.texpaint;
		n.texpaint_nor = brush_output_node_inst.texpaint_nor;
		n.texpaint_pack = brush_output_node_inst.texpaint_pack;
	}

	brush_output_node_inst = n;
	return n;
}

function brush_output_node_get_as_image(self: brush_output_node_t, from: i32): image_t {
	return logic_node_input_get_as_image(self.base.inputs[from]);
}
