
onmessage = async ({data : {wasm_module, memory, func_ptr, param_ptr, done_ptr}}) => {
	const                    stubs = new Proxy({}, {get : () => () => 0});
	const {instance}               = await WebAssembly.instantiate(wasm_module, {env : {memory}, imports : stubs});
	instance.exports.wasm_thread_run(func_ptr, param_ptr, done_ptr);
};
