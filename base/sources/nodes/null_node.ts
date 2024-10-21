
type null_node_t = {
	base?: logic_node_t;
};

function null_node_create(raw: ui_node_t, args: f32_array_t): null_node_t {
	let n: null_node_t = {};
	n.base = logic_node_create(n);
	n.base.get = float_node_get;
	return n;
}

function null_node_get(self: null_node_t, from: i32): logic_node_value_t {
	return null;
}
