
type null_node_t = {
	base?: logic_node_t;
};

function null_node_create(arg: any): null_node_t {
	let n: null_node_t = {};
	n.base = logic_node_create();
	n.base.get = float_node_get;
	return n;
}

function null_node_get(self: null_node_t, from: i32, done: (a: any)=>void) {
	done(null);
}
