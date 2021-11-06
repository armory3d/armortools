
let Module = {};
Module["instantiateWasm"] = function(imports, successCallback) {
	let wasmbin = Krom.loadBlob("data/plugins/import_gltf_glb.wasm");
	let module = new WebAssembly.Module(wasmbin);
	let inst = new WebAssembly.Instance(module, imports);
	successCallback(inst);
	return inst.exports;
};
Module.print = console.log;
Module.printErr = console.log;

// emscripten-generated glue
var a;a=a||("undefined"!=typeof Module?Module:{});var g,f={};for(g in a)a.hasOwnProperty(g)&&(f[g]=a[g]);var k,h="./this.program";k=function(n){return"function"==typeof readbuffer?new Uint8Array(readbuffer(n)):("object"==typeof(n=read(n,"binary"))||m("Assertion failed: undefined"),n)},"undefined"!=typeof print&&("undefined"==typeof console&&(console={}),console.log=print,console.warn=console.error="undefined"!=typeof printErr?printErr:print);var q,n=a.print||console.log.bind(console),p=a.printErr||console.warn.bind(console);for(g in f)f.hasOwnProperty(g)&&(a[g]=f[g]);f=null,a.thisProgram&&(h=a.thisProgram),a.wasmBinary&&(q=a.wasmBinary),"object"!=typeof WebAssembly&&p("no native wasm support detected");var r,x,y,z,A,u=new WebAssembly.Table({initial:3,maximum:3,element:"anyfunc"}),v=!1;function w(n){return 0<n%65536&&(n+=65536-n%65536),n}function B(n){x=n,a.HEAP8=y=new Int8Array(n),a.HEAP16=new Int16Array(n),a.HEAP32=A=new Int32Array(n),a.HEAPU8=z=new Uint8Array(n),a.HEAPU16=new Uint16Array(n),a.HEAPU32=new Uint32Array(n),a.HEAPF32=new Float32Array(n),a.HEAPF64=new Float64Array(n)}"undefined"!=typeof TextDecoder&&new TextDecoder("utf8"),"undefined"!=typeof TextDecoder&&new TextDecoder("utf-16le");var C=a.TOTAL_MEMORY||16777216;function D(n){for(;0<n.length;){var e=n.shift();if("function"==typeof e)e();else{var t=e.u;"number"==typeof t?void 0===e.s?a.dynCall_v(t):a.dynCall_vi(t,e.s):t(void 0===e.s?null:e.s)}}}(r=a.wasmMemory?a.wasmMemory:new WebAssembly.Memory({initial:C/65536}))&&(x=r.buffer),C=x.byteLength,B(x),A[752]=5246048;var E=[],F=[],G=[],H=[];function I(){var n=a.preRun.shift();E.unshift(n)}var J=0,K=null,L=null;function m(e){throw a.onAbort&&a.onAbort(e),n(e),p(e),v=!0,new WebAssembly.RuntimeError("abort("+e+"). Build with -s ASSERTIONS=1 for more info.")}function M(){var n=N;return String.prototype.startsWith?n.startsWith("data:application/octet-stream;base64,"):0===n.indexOf("data:application/octet-stream;base64,")}a.preloadedImages={},a.preloadedAudios={};var N="import_gltf_glb.wasm";if(!M()){var O=N;N=a.locateFile?a.locateFile(O,""):""+O}function P(){return new Promise(function(n){n:{try{if(q){var e=new Uint8Array(q);break n}if(k){e=k(N);break n}throw"both async and sync fetching of the wasm failed"}catch(n){m(n)}e=void 0}n(e)})}F.push({u:function(){Q()}});var R={};function S(){if(!T){var n,e={USER:"web_user",LOGNAME:"web_user",PATH:"/",PWD:"/",HOME:"/home/web_user",LANG:("object"==typeof navigator&&navigator.languages&&navigator.languages[0]||"C").replace("-","_")+".UTF-8",_:h};for(n in R)e[n]=R[n];var t=[];for(n in e)t.push(n+"="+e[n]);T=t}return T}var T,U={b:function(n,e,t){z.set(z.subarray(e,e+t),n)},a:function(n){if(2147418112<n)return!1;for(var e=Math.max(y.length,16777216);e<n;)e=e<=536870912?w(2*e):Math.min(w((3*e+2147483648)/4),2147418112);n:{try{r.grow(e-x.byteLength+65535>>16),B(r.buffer);var t=1;break n}catch(n){}t=void 0}return!!t},c:function(a,r){var i=0;return S().forEach(function(n,e){var t=r+i;for(e=A[a+4*e>>2]=t,t=0;t<n.length;++t)y[e++>>0]=n.charCodeAt(t);y[e>>0]=0,i+=n.length+1}),0},d:function(n,e){var t=S();A[n>>2]=t.length;var a=0;return t.forEach(function(n){a+=n.length+1}),A[e>>2]=a,0},memory:r,table:u},V=function(){function e(n){a.asm=n.exports,J--,a.monitorRunDependencies&&a.monitorRunDependencies(J),0==J&&(null!==K&&(clearInterval(K),K=null),L&&(n=L,L=null,n()))}function t(n){e(n.instance)}function r(n){return P().then(function(n){return WebAssembly.instantiate(n,i)}).then(n,function(n){p("failed to asynchronously prepare wasm: "+n),m(n)})}var i={env:U,wasi_unstable:U};if(J++,a.monitorRunDependencies&&a.monitorRunDependencies(J),a.instantiateWasm)try{return a.instantiateWasm(i,e)}catch(n){return p("Module.instantiateWasm callback failed with error: "+n),!1}return function(){if(q||"function"!=typeof WebAssembly.instantiateStreaming||M()||"function"!=typeof fetch)return r(t);fetch(N,{credentials:"same-origin"}).then(function(n){return WebAssembly.instantiateStreaming(n,i).then(t,function(n){p("wasm streaming compile failed: "+n),p("falling back to ArrayBuffer instantiation"),r(t)})})}(),{}}();a.asm=V;var W,Q=a.___wasm_call_ctors=function(){return a.asm.e.apply(null,arguments)};function Y(){function n(){if(!W&&(W=!0,!v)){if(D(F),D(G),a.onRuntimeInitialized&&a.onRuntimeInitialized(),a.postRun)for("function"==typeof a.postRun&&(a.postRun=[a.postRun]);a.postRun.length;){var n=a.postRun.shift();H.unshift(n)}D(H)}}if(!(0<J)){if(a.preRun)for("function"==typeof a.preRun&&(a.preRun=[a.preRun]);a.preRun.length;)I();D(E),0<J||(a.setStatus?(a.setStatus("Running..."),setTimeout(function(){setTimeout(function(){a.setStatus("")},1),n()},1)):n())}}if(a._init=function(){return a.asm.f.apply(null,arguments)},a._parse=function(){return a.asm.g.apply(null,arguments)},a._destroy=function(){return a.asm.h.apply(null,arguments)},a._get_index_count=function(){return a.asm.i.apply(null,arguments)},a._get_vertex_count=function(){return a.asm.j.apply(null,arguments)},a._get_scale_pos=function(){return a.asm.k.apply(null,arguments)},a._get_indices=function(){return a.asm.l.apply(null,arguments)},a._get_positions=function(){return a.asm.m.apply(null,arguments)},a._get_normals=function(){return a.asm.n.apply(null,arguments)},a._get_uvs=function(){return a.asm.o.apply(null,arguments)},a.asm=V,L=function n(){W||Y(),W||(L=n)},a.run=Y,a.preInit)for("function"==typeof a.preInit&&(a.preInit=[a.preInit]);0<a.preInit.length;)a.preInit.pop()();Y();
//

// Register as ArmorPaint plugin
let import_gltf_glb = function(path, done) {
	iron.Data.getBlob(path, function(b) {
		let buf = new Uint8Array(r.buffer, a._init(b.bytes.length), b.bytes.length);
		for (let i = 0; i < b.bytes.length; ++i) buf[i] = b.readU8(i);
		a._parse();
		let vertex_count = a._get_vertex_count();
		let index_count = a._get_index_count();
		let inda = new Uint32Array(r.buffer, a._get_indices(), index_count);
		let posa = new Int16Array(r.buffer, a._get_positions(), vertex_count * 4);
		let nora = new Int16Array(r.buffer, a._get_normals(), vertex_count * 2);
		let texa = new Int16Array(r.buffer, a._get_uvs(), vertex_count * 2);
		let name = path.split("\\").pop().split("/").pop().split(".").shift();
		done({
			name: name,
			posa: posa,
			nora: nora,
			texa: texa,
			inda: inda,
			scale_pos: a._get_scale_pos(),
			scale_tex: 1.0
		});
		a._destroy();
		iron.Data.deleteBlob(path);
	});
}

let plugin = new arm.Plugin();
let formats = arm.Path.meshFormats;
let importers = arm.Path.meshImporters;
formats.push("gltf");
formats.push("glb");
importers.h["gltf"] = import_gltf_glb;
importers.h["glb"] = import_gltf_glb;

plugin.delete = function() {
	formats.splice(formats.indexOf("gltf"), 1);
	formats.splice(formats.indexOf("glb"), 1);
	importers.h["gltf"] = null;
	importers.h["glb"] = null;
};
