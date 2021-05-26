
let Module = {};
Module["instantiateWasm"] = function(imports, successCallback) {
	let wasmbin = Krom.loadBlob("data/plugins/import_svg.wasm");
	let module = new WebAssembly.Module(wasmbin);
	let inst = new WebAssembly.Instance(module, imports);
	successCallback(inst);
	return inst.exports;
};
Module.print = console.log;
Module.printErr = console.log;

// emscripten-generated glue
var a;a||(a=typeof Module !== 'undefined' ? Module : {});var g={},h;for(h in a)a.hasOwnProperty(h)&&(g[h]=a[h]);var k;k=function(b){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(b));b=read(b,"binary");"object"===typeof b||m("Assertion failed: undefined");return b};"undefined"!==typeof print&&("undefined"===typeof console&&(console={}),console.log=print,console.warn=console.error="undefined"!==typeof printErr?printErr:print);
var n=a.print||console.log.bind(console),p=a.printErr||console.warn.bind(console);for(h in g)g.hasOwnProperty(h)&&(a[h]=g[h]);g=null;var q;a.wasmBinary&&(q=a.wasmBinary);var noExitRuntime;a.noExitRuntime&&(noExitRuntime=a.noExitRuntime);"object"!==typeof WebAssembly&&p("no native wasm support detected");var r,t=new WebAssembly.Table({initial:2,maximum:2,element:"anyfunc"}),u=!1,v,w,x;
function y(b){v=b;a.HEAP8=new Int8Array(b);a.HEAP16=new Int16Array(b);a.HEAP32=x=new Int32Array(b);a.HEAPU8=w=new Uint8Array(b);a.HEAPU16=new Uint16Array(b);a.HEAPU32=new Uint32Array(b);a.HEAPF32=new Float32Array(b);a.HEAPF64=new Float64Array(b)}var z=a.INITIAL_MEMORY||16777216;a.wasmMemory?r=a.wasmMemory:r=new WebAssembly.Memory({initial:z/65536,maximum:32768});r&&(v=r.buffer);z=v.byteLength;y(v);x[1492]=5249008;
function A(b){for(;0<b.length;){var c=b.shift();if("function"==typeof c)c(a);else{var d=c.l;"number"===typeof d?void 0===c.j?a.dynCall_v(d):a.dynCall_vi(d,c.j):d(void 0===c.j?null:c.j)}}}var B=[],C=[],D=[],E=[];function F(){var b=a.preRun.shift();B.unshift(b)}var G=0,H=null,I=null;a.preloadedImages={};a.preloadedAudios={};function m(b){if(a.onAbort)a.onAbort(b);n(b);p(b);u=!0;throw new WebAssembly.RuntimeError("abort("+b+"). Build with -s ASSERTIONS=1 for more info.");}
function J(){var b=K;return String.prototype.startsWith?b.startsWith("data:application/octet-stream;base64,"):0===b.indexOf("data:application/octet-stream;base64,")}var K="import_svg.wasm";if(!J()){var L=K;K=a.locateFile?a.locateFile(L,""):""+L}function M(){return new Promise(function(b){a:{try{if(q){var c=new Uint8Array(q);break a}if(k){c=k(K);break a}throw"both async and sync fetching of the wasm failed";}catch(d){m(d)}c=void 0}b(c)})}C.push({l:function(){N()}});
var O={b:function(b,c,d){w.copyWithin(b,c,c+d)},a:function(b){b>>>=0;var c=w.length;if(2147483648<b)return!1;for(var d=1;4>=d;d*=2){var f=c*(1+.2/d);f=Math.min(f,b+100663296);f=Math.max(16777216,b,f);0<f%65536&&(f+=65536-f%65536);a:{try{r.grow(Math.min(2147483648,f)-v.byteLength+65535>>>16);y(r.buffer);var e=1;break a}catch(l){}e=void 0}if(e)return!0}return!1},memory:r,table:t},P=function(){function b(e){a.asm=e.exports;G--;a.monitorRunDependencies&&a.monitorRunDependencies(G);0==G&&(null!==H&&(clearInterval(H),
H=null),I&&(e=I,I=null,e()))}function c(e){b(e.instance)}function d(e){return M().then(function(l){return WebAssembly.instantiate(l,f)}).then(e,function(l){p("failed to asynchronously prepare wasm: "+l);m(l)})}var f={a:O};G++;a.monitorRunDependencies&&a.monitorRunDependencies(G);if(a.instantiateWasm)try{return a.instantiateWasm(f,b)}catch(e){return p("Module.instantiateWasm callback failed with error: "+e),!1}(function(){if(q||"function"!==typeof WebAssembly.instantiateStreaming||J()||"function"!==
typeof fetch)return d(c);fetch(K,{credentials:"same-origin"}).then(function(e){return WebAssembly.instantiateStreaming(e,f).then(c,function(l){p("wasm streaming compile failed: "+l);p("falling back to ArrayBuffer instantiation");d(c)})})})();return{}}();a.asm=P;var N=a.___wasm_call_ctors=function(){return(N=a.___wasm_call_ctors=a.asm.c).apply(null,arguments)};a._init=function(){return(a._init=a.asm.d).apply(null,arguments)};a._parse=function(){return(a._parse=a.asm.e).apply(null,arguments)};
a._get_pixels=function(){return(a._get_pixels=a.asm.f).apply(null,arguments)};a._get_pixels_w=function(){return(a._get_pixels_w=a.asm.g).apply(null,arguments)};a._get_pixels_h=function(){return(a._get_pixels_h=a.asm.h).apply(null,arguments)};a._destroy=function(){return(a._destroy=a.asm.i).apply(null,arguments)};a.asm=P;var Q;I=function R(){Q||S();Q||(I=R)};
function S(){function b(){if(!Q&&(Q=!0,a.calledRun=!0,!u)){A(C);A(D);if(a.onRuntimeInitialized)a.onRuntimeInitialized();if(a.postRun)for("function"==typeof a.postRun&&(a.postRun=[a.postRun]);a.postRun.length;){var c=a.postRun.shift();E.unshift(c)}A(E)}}if(!(0<G)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)F();A(B);0<G||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")},1);b()},1)):b())}}a.run=S;
if(a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();noExitRuntime=!0;S();
//

// Register as ArmorPaint plugin
let import_svg = function(path, done) {
	iron.Data.getBlob(path, function(b) {
		let buf = new Uint8Array(r.buffer, a._init(b.bytes.length + 1), b.bytes.length + 1);
		for (let i = 0; i < b.bytes.length; ++i) buf[i] = b.readU8(i);
		buf[b.bytes.length] = 0;

		a._parse();
		let w = a._get_pixels_w();
		let h = a._get_pixels_h();
		let pixels = r.buffer.slice(a._get_pixels(), a._get_pixels() + w * h * 4);
		let image = core.Image.fromBytes(core.Bytes.ofData(pixels), w, h);
		done(image);

		a._destroy();
		iron.Data.deleteBlob(path);
	});
}

let plugin = new arm.Plugin();
let formats = arm.Path.textureFormats;
let importers = arm.Path.textureImporters;
formats.push("svg");
importers.h["svg"] = import_svg;

plugin.delete = function() {
	formats.splice(formats.indexOf("svg"), 1);
	importers.h["svg"] = null;
};

