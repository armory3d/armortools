
Module = {}
Module["instantiateWasm"] = function(imports, successCallback) {
	let wasmbin = Krom.loadBlob("data/plugins/uv_unwrap.wasm");
	let module = new WebAssembly.Module(wasmbin);
	let inst = new WebAssembly.Instance(module, imports);
	successCallback(inst);
	return inst.exports;
};

// emscripten-generated glue
var b;b=b||("undefined"!=typeof Module?Module:{});var k,h={};for(k in b)b.hasOwnProperty(k)&&(h[k]=b[k]);var m,l="./this.program";m=function(t){return"function"==typeof readbuffer?new Uint8Array(readbuffer(t)):("object"==typeof(t=read(t,"binary"))||n("Assertion failed: undefined"),t)},"undefined"!=typeof print&&("undefined"==typeof console&&(console={}),console.log=print,console.warn=console.error="undefined"!=typeof printErr?printErr:print);var r,p=b.print||console.log.bind(console),q=b.printErr||console.warn.bind(console);for(k in h)h.hasOwnProperty(k)&&(b[k]=h[k]);h=null,b.thisProgram&&(l=b.thisProgram),b.wasmBinary&&(r=b.wasmBinary),"object"!=typeof WebAssembly&&q("no native wasm support detected");var t,A,B,y,C,u=new WebAssembly.Table({initial:9,maximum:9,element:"anyfunc"}),v=!1,w="undefined"!=typeof TextDecoder?new TextDecoder("utf8"):void 0;function x(n){if(n){for(var t=y,e=n+void 0,r=n;t[r]&&!(e<=r);)++r;if(16<r-n&&t.subarray&&w)n=w.decode(t.subarray(n,r));else{for(e="";n<r;){var o=t[n++];if(128&o){var a=63&t[n++];if(192==(224&o))e+=String.fromCharCode((31&o)<<6|a);else{var i=63&t[n++];(o=224==(240&o)?(15&o)<<12|a<<6|i:(7&o)<<18|a<<12|i<<6|63&t[n++])<65536?e+=String.fromCharCode(o):(o-=65536,e+=String.fromCharCode(55296|o>>10,56320|1023&o))}}else e+=String.fromCharCode(o)}n=e}}else n="";return n}function z(n){return 0<n%65536&&(n+=65536-n%65536),n}function D(n){A=n,b.HEAP8=B=new Int8Array(n),b.HEAP16=new Int16Array(n),b.HEAP32=C=new Int32Array(n),b.HEAPU8=y=new Uint8Array(n),b.HEAPU16=new Uint16Array(n),b.HEAPU32=new Uint32Array(n),b.HEAPF32=new Float32Array(n),b.HEAPF64=new Float64Array(n)}"undefined"!=typeof TextDecoder&&new TextDecoder("utf-16le");var E=b.TOTAL_MEMORY||16777216;function F(n){for(;0<n.length;){var t=n.shift();if("function"==typeof t)t();else{var e=t.A;"number"==typeof e?void 0===t.w?b.dynCall_v(e):b.dynCall_vi(e,t.w):e(void 0===t.w?null:t.w)}}}(t=b.wasmMemory?b.wasmMemory:new WebAssembly.Memory({initial:E/65536}))&&(A=t.buffer),E=A.byteLength,D(A),C[2124]=5251536;var G=[],H=[],I=[],K=[];function L(){var n=b.preRun.shift();G.unshift(n)}var M=0,N=null,O=null;function n(n){throw b.onAbort&&b.onAbort(n),p(n),q(n),v=!0,new WebAssembly.RuntimeError("abort("+n+"). Build with -s ASSERTIONS=1 for more info.")}function P(){var n=Q;return String.prototype.startsWith?n.startsWith("data:application/octet-stream;base64,"):0===n.indexOf("data:application/octet-stream;base64,")}b.preloadedImages={},b.preloadedAudios={};var Q="uv_unwrap.wasm";if(!P()){var R=Q;Q=b.locateFile?b.locateFile(R,""):""+R}function S(){return new Promise(function(t){n:{try{if(r){var e=new Uint8Array(r);break n}if(m){e=m(Q);break n}throw"both async and sync fetching of the wasm failed"}catch(t){n(t)}e=void 0}t(e)})}H.push({A:function(){aa()}});var T={};function U(){if(!V){var n,t={USER:"web_user",LOGNAME:"web_user",PATH:"/",PWD:"/",HOME:"/home/web_user",LANG:("object"==typeof navigator&&navigator.languages&&navigator.languages[0]||"C").replace("-","_")+".UTF-8",_:l};for(n in T)t[n]=T[n];var e=[];for(n in t)e.push(n+"="+t[n]);V=e}return V}var V,W={a:function(t,e,r,o){n("Assertion failed: "+x(t)+", at: "+[e?x(e):"unknown filename",r,o?x(o):"unknown function"])},e:function(n,t,e){y.set(y.subarray(t,t+e),n)},d:function(n){if(2147418112<n)return!1;for(var e=Math.max(B.length,16777216);e<n;)e=e<=536870912?z(2*e):Math.min(z((3*e+2147483648)/4),2147418112);n:{try{t.grow(e-A.byteLength+65535>>16),D(t.buffer);var r=1;break n}catch(n){}r=void 0}return!!r},f:function(r,o){var a=0;return U().forEach(function(n,t){var e=o+a;for(t=C[r+4*t>>2]=e,e=0;e<n.length;++e)B[t++>>0]=n.charCodeAt(e);B[t>>0]=0,a+=n.length+1}),0},g:function(n,t){var e=U();C[n>>2]=e.length;var r=0;return e.forEach(function(n){r+=n.length+1}),C[t>>2]=r,0},c:function(){q("missing function: iprintf"),n(-1)},memory:t,b:function(n){var t=(n=x(n)).substr(0);return"\n"===t[t.length-1]&&(t=t.substr(0,t.length-1)),p(t),n.length},table:u},X=function(){function t(n){b.asm=n.exports,M--,b.monitorRunDependencies&&b.monitorRunDependencies(M),0==M&&(null!==N&&(clearInterval(N),N=null),O&&(n=O,O=null,n()))}function e(n){t(n.instance)}function o(t){return S().then(function(n){return WebAssembly.instantiate(n,a)}).then(t,function(t){q("failed to asynchronously prepare wasm: "+t),n(t)})}var a={env:W,wasi_unstable:W};if(M++,b.monitorRunDependencies&&b.monitorRunDependencies(M),b.instantiateWasm)try{return b.instantiateWasm(a,t)}catch(n){return q("Module.instantiateWasm callback failed with error: "+n),!1}return function(){if(r||"function"!=typeof WebAssembly.instantiateStreaming||P()||"function"!=typeof fetch)return o(e);fetch(Q,{credentials:"same-origin"}).then(function(n){return WebAssembly.instantiateStreaming(n,a).then(e,function(n){q("wasm streaming compile failed: "+n),q("falling back to ArrayBuffer instantiation"),o(e)})})}(),{}}();b.asm=X;var Y,aa=b.___wasm_call_ctors=function(){return b.asm.h.apply(null,arguments)};function Z(){function n(){if(!Y&&(Y=!0,!v)){if(F(H),F(I),b.onRuntimeInitialized&&b.onRuntimeInitialized(),b.postRun)for("function"==typeof b.postRun&&(b.postRun=[b.postRun]);b.postRun.length;){var n=b.postRun.shift();K.unshift(n)}F(K)}}if(!(0<M)){if(b.preRun)for("function"==typeof b.preRun&&(b.preRun=[b.preRun]);b.preRun.length;)L();F(G),0<M||(b.setStatus?(b.setStatus("Running..."),setTimeout(function(){setTimeout(function(){b.setStatus("")},1),n()},1)):n())}}if(b._setVertexCount=function(){return b.asm.i.apply(null,arguments)},b._setIndexCount=function(){return b.asm.j.apply(null,arguments)},b._setPositions=function(){return b.asm.k.apply(null,arguments)},b._setNormals=function(){return b.asm.l.apply(null,arguments)},b._setIndices=function(){return b.asm.m.apply(null,arguments)},b._getVertexCount=function(){return b.asm.n.apply(null,arguments)},b._getIndexCount=function(){return b.asm.o.apply(null,arguments)},b._getPositions=function(){return b.asm.p.apply(null,arguments)},b._getNormals=function(){return b.asm.q.apply(null,arguments)},b._getUVs=function(){return b.asm.r.apply(null,arguments)},b._getIndices=function(){return b.asm.s.apply(null,arguments)},b._unwrap=function(){return b.asm.t.apply(null,arguments)},b._destroy=function(){return b.asm.u.apply(null,arguments)},b.dynCall_vi=function(){return b.asm.v.apply(null,arguments)},b.asm=X,O=function n(){Y||Z(),Y||(O=n)},b.run=Z,b.preInit)for("function"==typeof b.preInit&&(b.preInit=[b.preInit]);0<b.preInit.length;)b.preInit.pop()();Z();
//

let plugin = new arm.Plugin();
let h1 = new zui.Handle();
plugin.drawUI = function(ui) {
	if (ui.panel(h1, "UV Unwrap")) {
		if (ui.button("Unwrap Mesh")) {

			for (const po of arm.Project.paintObjects) {

				let raw = po.data.raw;
				let positions = raw.vertex_arrays[0].values;
				let normals = raw.vertex_arrays[1].values;
				let indices = raw.index_arrays[0].values;
				let vertexCount = positions.length / 4;
				let indexCount = indices.length;

				b._setVertexCount(vertexCount);
				b._setIndexCount(indexCount);
				let pa = new Float32Array(t.buffer, b._setPositions(), vertexCount * 3);
				let na = new Float32Array(t.buffer, b._setNormals(), vertexCount * 3);
				let ia = new Uint32Array(t.buffer, b._setIndices(), indexCount);

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

				b._unwrap();

				vertexCount = b._getVertexCount();
				indexCount = b._getIndexCount();
				pa = new Float32Array(t.buffer, b._getPositions(), vertexCount * 3);
				na = new Float32Array(t.buffer, b._getNormals(), vertexCount * 3);
				let ua = new Float32Array(t.buffer, b._getUVs(), vertexCount * 2);
				ia = new Uint32Array(t.buffer, b._getIndices(), indexCount);

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

				po.data.raw.vertex_arrays[0].values = pa16;
				po.data.raw.vertex_arrays[1].values = na16;
				po.data.raw.vertex_arrays[2].values = ua16;
				po.data.raw.index_arrays[0].values = ia32;

				let geom = po.data.geom;
				geom.positions = pa16;
				geom.normals = na16;
				geom.uvs = ua16;
				geom.indices[0] = ia32;
				geom.ready = false;
				geom.build();

				b._destroy();
			}

			arm.Context.mergedObject = null;
		}
	}
}
