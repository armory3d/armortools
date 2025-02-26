
function brush_output_node_create(raw: ui_node_t, args: f32_array_t): brush_output_node_t {
    context_raw.run_brush = brush_output_node_run;
	context_raw.parse_brush_inputs = brush_output_node_parse_inputs;

    let n: brush_output_node_t = {};
	n.base = logic_node_create(n);
    n.raw = raw;
    brush_output_node_create_ext(n);
    return n;
}
