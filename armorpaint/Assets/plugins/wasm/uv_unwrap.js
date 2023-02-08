
let Module = {};
Module["instantiateWasm"] = function(imports, successCallback) {
	let wasmbin = Krom.loadBlob("data/plugins/uv_unwrap.wasm");
	let module = new WebAssembly.Module(wasmbin);
	let inst = new WebAssembly.Instance(module, imports);
	successCallback(inst);
	return inst.exports;
};
Module.print = console.log;
Module.printErr = console.log;

// emscripten-generated glue
var a;a||(a=typeof Module !== 'undefined' ? Module : {});var h={},k;for(k in a)a.hasOwnProperty(k)&&(h[k]=a[k]);var l;l=function(b){if("function"===typeof readbuffer)return new Uint8Array(readbuffer(b));b=read(b,"binary");"object"===typeof b||m("Assertion failed: undefined");return b};"undefined"!==typeof print&&("undefined"===typeof console&&(console={}),console.log=print,console.warn=console.error="undefined"!==typeof printErr?printErr:print);
var n=a.print||console.log.bind(console),p=a.printErr||console.warn.bind(console);for(k in h)h.hasOwnProperty(k)&&(a[k]=h[k]);h=null;var q;a.wasmBinary&&(q=a.wasmBinary);var noExitRuntime;a.noExitRuntime&&(noExitRuntime=a.noExitRuntime);"object"!==typeof WebAssembly&&m("no native wasm support detected");var r,t=!1,v="undefined"!==typeof TextDecoder?new TextDecoder("utf8"):void 0;
function w(b,c,e){var f=c+e;for(e=c;b[e]&&!(e>=f);)++e;if(16<e-c&&b.subarray&&v)return v.decode(b.subarray(c,e));for(f="";c<e;){var d=b[c++];if(d&128){var g=b[c++]&63;if(192==(d&224))f+=String.fromCharCode((d&31)<<6|g);else{var u=b[c++]&63;d=224==(d&240)?(d&15)<<12|g<<6|u:(d&7)<<18|g<<12|u<<6|b[c++]&63;65536>d?f+=String.fromCharCode(d):(d-=65536,f+=String.fromCharCode(55296|d>>10,56320|d&1023))}}else f+=String.fromCharCode(d)}return f}function x(b){return b?w(y,b,void 0):""}var z,y,A;
function B(){var b=r.buffer;z=b;a.HEAP8=new Int8Array(b);a.HEAP16=new Int16Array(b);a.HEAP32=A=new Int32Array(b);a.HEAPU8=y=new Uint8Array(b);a.HEAPU16=new Uint16Array(b);a.HEAPU32=new Uint32Array(b);a.HEAPF32=new Float32Array(b);a.HEAPF64=new Float64Array(b)}var C,G=[],H=[],I=[],J=[];H.push({v:function(){K()}});function L(){var b=a.preRun.shift();G.unshift(b)}var M=0,N=null,O=null;a.preloadedImages={};a.preloadedAudios={};
function m(b){if(a.onAbort)a.onAbort(b);p(b);t=!0;throw new WebAssembly.RuntimeError("abort("+b+"). Build with -s ASSERTIONS=1 for more info.");}function P(){var b=Q;return String.prototype.startsWith?b.startsWith("data:application/octet-stream;base64,"):0===b.indexOf("data:application/octet-stream;base64,")}var Q="xatlas.wasm";if(!P()){var R=Q;Q=a.locateFile?a.locateFile(R,""):""+R}
function T(){return Promise.resolve().then(function(){a:{var b=Q;try{if(b==Q&&q){var c=new Uint8Array(q);break a}if(l){c=l(b);break a}throw"both async and sync fetching of the wasm failed";}catch(e){m(e)}c=void 0}return c})}function U(b){for(;0<b.length;){var c=b.shift();if("function"==typeof c)c(a);else{var e=c.v;"number"===typeof e?void 0===c.u?C.get(e)():C.get(e)(c.u):e(void 0===c.u?null:c.u)}}}
var V=[null,[],[]],W={a:function(b,c,e,f){m("Assertion failed: "+x(b)+", at: "+[c?x(c):"unknown filename",e,f?x(f):"unknown function"])},d:function(b,c,e){y.copyWithin(b,c,c+e)},b:function(b){b>>>=0;var c=y.length;if(2147483648<b)return!1;for(var e=1;4>=e;e*=2){var f=c*(1+.2/e);f=Math.min(f,b+100663296);f=Math.max(16777216,b,f);0<f%65536&&(f+=65536-f%65536);a:{try{r.grow(Math.min(2147483648,f)-z.byteLength+65535>>>16);B();var d=1;break a}catch(g){}d=void 0}if(d)return!0}return!1},c:function(b,c,e,
f){for(var d=0,g=0;g<e;g++){for(var u=A[c+8*g>>2],S=A[c+(8*g+4)>>2],D=0;D<S;D++){var E=y[u+D],F=V[b];0===E||10===E?((1===b?n:p)(w(F,0)),F.length=0):F.push(E)}d+=S}A[f>>2]=d;return 0}};
(function(){function b(d){a.asm=d.exports;r=a.asm.e;B();C=a.asm.f;M--;a.monitorRunDependencies&&a.monitorRunDependencies(M);0==M&&(null!==N&&(clearInterval(N),N=null),O&&(d=O,O=null,d()))}function c(d){b(d.instance)}function e(d){return T().then(function(g){return WebAssembly.instantiate(g,f)}).then(d,function(g){p("failed to asynchronously prepare wasm: "+g);m(g)})}var f={a:W};M++;a.monitorRunDependencies&&a.monitorRunDependencies(M);if(a.instantiateWasm)try{return a.instantiateWasm(f,b)}catch(d){return p("Module.instantiateWasm callback failed with error: "+
d),!1}(function(){return q||"function"!==typeof WebAssembly.instantiateStreaming||P()||"function"!==typeof fetch?e(c):fetch(Q,{credentials:"same-origin"}).then(function(d){return WebAssembly.instantiateStreaming(d,f).then(c,function(g){p("wasm streaming compile failed: "+g);p("falling back to ArrayBuffer instantiation");return e(c)})})})();return{}})();var K=a.___wasm_call_ctors=function(){return(K=a.___wasm_call_ctors=a.asm.g).apply(null,arguments)};
a._setVertexCount=function(){return(a._setVertexCount=a.asm.h).apply(null,arguments)};a._setIndexCount=function(){return(a._setIndexCount=a.asm.i).apply(null,arguments)};a._setPositions=function(){return(a._setPositions=a.asm.j).apply(null,arguments)};a._setNormals=function(){return(a._setNormals=a.asm.k).apply(null,arguments)};a._setIndices=function(){return(a._setIndices=a.asm.l).apply(null,arguments)};a._getVertexCount=function(){return(a._getVertexCount=a.asm.m).apply(null,arguments)};
a._getIndexCount=function(){return(a._getIndexCount=a.asm.n).apply(null,arguments)};a._getPositions=function(){return(a._getPositions=a.asm.o).apply(null,arguments)};a._getNormals=function(){return(a._getNormals=a.asm.p).apply(null,arguments)};a._getUVs=function(){return(a._getUVs=a.asm.q).apply(null,arguments)};a._getIndices=function(){return(a._getIndices=a.asm.r).apply(null,arguments)};a._unwrap=function(){return(a._unwrap=a.asm.s).apply(null,arguments)};
a._destroy=function(){return(a._destroy=a.asm.t).apply(null,arguments)};var X;O=function Y(){X||Z();X||(O=Y)};
function Z(){function b(){if(!X&&(X=!0,a.calledRun=!0,!t)){U(H);U(I);if(a.onRuntimeInitialized)a.onRuntimeInitialized();if(a.postRun)for("function"==typeof a.postRun&&(a.postRun=[a.postRun]);a.postRun.length;){var c=a.postRun.shift();J.unshift(c)}U(J)}}if(!(0<M)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)L();U(G);0<M||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")},1);b()},1)):b())}}a.run=Z;
if(a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();noExitRuntime=!0;Z();
//

function unwrap_mesh(mesh) {
	let positions = mesh.posa;
	let normals = mesh.nora;
	let indices = mesh.inda;
	let vertexCount = positions.length / 4;
	let indexCount = indices.length;

	a._setVertexCount(vertexCount);
	a._setIndexCount(indexCount);
	let pa = new Float32Array(r.buffer, a._setPositions(), vertexCount * 3);
	let na = new Float32Array(r.buffer, a._setNormals(), vertexCount * 3);
	let ia = new Uint32Array(r.buffer, a._setIndices(), indexCount);

	let inv = 1 / 32767;

	for (let i = 0; i < vertexCount; i++) {
		pa[i * 3    ] = positions[i * 4    ] * inv;
		pa[i * 3 + 1] = positions[i * 4 + 1] * inv;
		pa[i * 3 + 2] = positions[i * 4 + 2] * inv;
		na[i * 3    ] = normals  [i * 2    ] * inv;
		na[i * 3 + 1] = normals  [i * 2 + 1] * inv;
		na[i * 3 + 2] = positions[i * 4 + 3] * inv;
	}
	for (let i = 0; i < indexCount; i++) {
		ia[i] = indices[i];
	}

	a._unwrap();

	vertexCount = a._getVertexCount();
	indexCount = a._getIndexCount();
	pa = new Float32Array(r.buffer, a._getPositions(), vertexCount * 3);
	na = new Float32Array(r.buffer, a._getNormals(), vertexCount * 3);
	let ua = new Float32Array(r.buffer, a._getUVs(), vertexCount * 2);
	ia = new Uint32Array(r.buffer, a._getIndices(), indexCount);

	let pa16 = new Int16Array(vertexCount * 4);
	let na16 = new Int16Array(vertexCount * 2);
	let ua16 = new Int16Array(vertexCount * 2);
	let ia32 = new Uint32Array(indexCount);

	for (let i = 0; i < vertexCount; i++) {
		pa16[i * 4    ] = pa[i * 3    ] / inv;
		pa16[i * 4 + 1] = pa[i * 3 + 1] / inv;
		pa16[i * 4 + 2] = pa[i * 3 + 2] / inv;
		pa16[i * 4 + 3] = na[i * 3 + 2] / inv;
		na16[i * 2    ] = na[i * 3    ] / inv;
		na16[i * 2 + 1] = na[i * 3 + 1] / inv;
		ua16[i * 2    ] = ua[i * 2    ] / inv;
		ua16[i * 2 + 1] = ua[i * 2 + 1] / inv;
	}
	for (let i = 0; i < indexCount; i++) {
		ia32[i] = ia[i];
	}

	mesh.posa = pa16;
	mesh.nora = na16;
	mesh.texa = ua16;
	mesh.inda = ia32;

	a._destroy();
}

let plugin = new arm.Plugin();
let h1 = new zui.Handle();
plugin.drawUI = function(ui) {
	if (ui.panel(h1, "UV Unwrap")) {
		if (ui.button("Unwrap Mesh")) {
			for (const po of arm.Project.paintObjects) {
				let raw = po.data.raw;
				var mesh = {
					posa: raw.vertex_arrays[0].values,
					nora: raw.vertex_arrays[1].values,
					texa: null,
					inda: raw.index_arrays[0].values
				};
				unwrap_mesh(mesh);
				raw.vertex_arrays[0].values = mesh.posa;
				raw.vertex_arrays[1].values = mesh.nora;
				raw.vertex_arrays[2].values = mesh.texa;
				raw.index_arrays[0].values = mesh.inda;
				let geom = po.data.geom;
				geom.indices[0] = mesh.inda;
				geom.ready = false;
				geom.build();
			}
			arm.MeshUtil.mergeMesh();
		}
	}
}

let unwrappers = arm.MeshUtil.unwrappers;
unwrappers.h["uv_unwrap.js"] = unwrap_mesh;
plugin.delete = function() {
	unwrappers.h["uv_unwrap.js"] = null;
};
