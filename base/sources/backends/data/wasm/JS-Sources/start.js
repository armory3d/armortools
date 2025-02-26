// Includes snippets from https://surma.dev/things/c-to-webassembly/ and https://developer.mozilla.org/en-US/docs/WebAssembly/Using_the_JavaScript_API -->

let memory = null;
let heapu8 = null;
let heapu16 = null;
let heapu32 = null;
let heapi32 = null;
let heapf32 = null;
let mod = null;
let instance = null;
let audio_thread_started = false;

function create_thread(func) {
	console.log('Creating thread');
	const thread_starter = new Worker('thread_starter.js');
	const arr = new Uint8Array(memory.buffer, func, 256);
	let str = '';
	for (let i = 0; arr[i] != 0; ++i) {
		str += String.fromCharCode(arr[i]);
	}
	thread_starter.postMessage({ mod, memory, func: str });
}

async function start_audio_thread() {
	const audioContext = new AudioContext();
	await audioContext.audioWorklet.addModule('audio-thread.js');
	const audioThreadNode = new AudioWorkletNode(audioContext, 'audio-thread', { processorOptions: { mod, memory }});
	audioThreadNode.port.onmessage = (message) => {
		console.log(message.data);
	};
	audioThreadNode.connect(audioContext.destination);
}

function read_string(ptr) {
	let str = '';
	for (let i = 0; heapu8[ptr + i] != 0; ++i) {
		str += String.fromCharCode(heapu8[ptr + i]);
	}
	return str;
}

function write_string(ptr, str) {
	for (let i = 0; i < str.length; ++i) {
		heapu8[ptr + i] = str.charCodeAt(i);
	}
	heapu8[ptr + str.length] = 0;
}

async function init() {
	let wasm_bytes = null;
	await fetch("./ShaderTest.wasm").then(res => res.arrayBuffer()).then(buffer => wasm_bytes = new Uint8Array(buffer));

	// Read memory size from wasm file
	let memory_size = 0;
	let i = 8;
	while (i < wasm_bytes.length) {
		function read_leb() {
			let result = 0;
			let shift = 0;
			while (true) {
				let byte = wasm_bytes[i++];
				result |= (byte & 0x7f) << shift;
				if ((byte & 0x80) == 0) return result;
				shift += 7;
			}
		}
		let type = read_leb()
		let length = read_leb()
		if (type == 6) {
			read_leb(); // count
			i++; // gtype
			i++; // mutable
			read_leb(); // opcode
			memory_size = read_leb() / 65536 + 1;
			break;
		}
		i += length;
	}

	memory = new WebAssembly.Memory({ initial: memory_size, maximum: memory_size, shared: true });
	heapu8 = new Uint8Array(memory.buffer);
	heapu16 = new Uint16Array(memory.buffer);
	heapu32 = new Uint32Array(memory.buffer);
	heapi32 = new Int32Array(memory.buffer);
	heapf32 = new Float32Array(memory.buffer);

	const kanvas = document.getElementById('kanvas');
	const gl = kanvas.getContext('webgl2', { antialias: false, alpha: false });
	// gl.pixelStorei(gl.UNPACK_PREMULTIPLY_ALPHA_WEBGL, 1);
	gl.getExtension("EXT_color_buffer_float");
	gl.getExtension("OES_texture_float_linear");
	gl.getExtension("OES_texture_half_float_linear");
	gl.getExtension("EXT_texture_filter_anisotropic");

	let file_buffer = null;
	let file_buffer_pos = 0;
	let gl_programs = [null];
	let gl_shaders = [null];
	let gl_buffers = [null];
	let gl_framebuffers = [null];
	let gl_renderbuffers = [null];
	let gl_textures = [null];
	let gl_locations = [null];

	const result = await WebAssembly.instantiate(
		wasm_bytes, {
			env: { memory },
			imports: {
				create_thread,
				glViewport: function(x, y, width, height) {
					gl.viewport(x, y, width, height);
				},
				glScissor: function(x, y, width, height) {
					gl.scissor(x, y, width, height);
				},
				glGetIntegerv: function(pname, data) {
					if (pname == 2) { // GL_MAJOR_VERSION
						heapu32[data / 4] = 3;
					}
					else {
						heapu32[data / 4] = gl.getParameter(pname);
					}
				},
				glGetFloatv: function(pname, data) {
					heapf32[data / 4] = gl.getParameter(pname);
				},
				glGetString: function(name) {
					// return gl.getParameter(name);
				},
				glDrawElements: function(mode, count, type, offset) {
					gl.drawElements(mode, count, type, offset);
				},
				glDrawElementsInstanced: function(mode, count, type, indices, instancecount) {
					gl.drawElementsInstanced(mode, count, type, indices, instancecount);
				},
				glVertexAttribDivisor: function(index, divisor) {
					gl.vertexAttribDivisor(index, divisor);
				},
				glBindFramebuffer: function(target, framebuffer) {
					gl.bindFramebuffer(target, gl_framebuffers[framebuffer]);
				},
				glFramebufferTexture2D: function(target, attachment, textarget, texture, level) {
					gl.framebufferTexture2D(target, attachment, textarget, gl_textures[texture], level);
					if (gl.checkFramebufferStatus(gl.FRAMEBUFFER) != gl.FRAMEBUFFER_COMPLETE) {
						console.log("Incomplete framebuffer");
					}
				},
				glGenFramebuffers: function(n, framebuffers) {
					for (let i = 0; i < n; ++i) {
						gl_framebuffers.push(gl.createFramebuffer());
						heapu32[framebuffers / 4 + i] = gl_framebuffers.length - 1;
					}
				},
				glGenRenderbuffers: function(n, renderbuffers) {
					for (let i = 0; i < n; ++i) {
						gl_renderbuffers.push(gl.createRenderbuffer());
						heapu32[renderbuffers / 4 + i] = gl_renderbuffers.length - 1;
					}
				},
				glBindRenderbuffer: function(target, renderbuffer) {
					gl.bindRenderbuffer(target, gl_renderbuffers[renderbuffer]);
				},
				glRenderbufferStorage: function(target, internalformat, width, height) {
					gl.renderbufferStorage(target, internalformat, width, height)
				},
				glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) {
					gl.framebufferRenderbuffer(target, attachment, renderbuffertarget, gl_renderbuffers[renderbuffer]);
				},
				glReadPixels: function(x, y, width, height, format, type, data) {
					let pixels = type == gl.FLOAT ? heapf32.subarray(data / 4) : heapu8.subarray(data);
					gl.readPixels(x, y, width, height, format, type, pixels);
				},
				glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) {
					gl.texSubImage2D(target, level, xoffset, yoffset, width, height, format, type, heapu8.subarray(pixels));
				},
				glEnable: function(cap) {
					gl.enable(cap);
				},
				glDisable: function(cap) {
					gl.disable(cap);
				},
				glColorMask: function(red, green, blue, alpha) {
					gl.colorMask(red, green, blue, alpha);
				},
				glClearColor: function(red, green, blue, alpha) {
					gl.clearColor(red, green, blue, alpha);
				},
				glDepthMask: function(flag) {
					gl.depthMask(flag);
				},
				glClearDepthf: function(depth) {
					gl.clearDepth(depth);
				},
				glStencilMask: function(mask) {
					gl.stencilMask(mask);
				},
				glClearStencil: function(s) {
					gl.clearStencil(s);
				},
				glClear: function(mask) {
					gl.clear(mask);
				},
				glBindBuffer: function(target, buffer) {
					gl.bindBuffer(target, gl_buffers[buffer]);
				},
				glUseProgram: function(program) {
					gl.useProgram(gl_programs[program]);
				},
				glStencilMaskSeparate: function(face, mask) {
					gl.stencilMaskSeparate(face, mask);
				},
				glStencilOpSeparate: function(face, fail, zfail, zpass) {
					gl.stencilOpSeparate(face, fail, zfail, zpass);
				},
				glStencilFuncSeparate: function(face, func, ref, mask) {
					gl.stencilFuncSeparate(face, func, ref, mask);
				},
				glDepthFunc: function(func) {
					gl.depthFunc(func);
				},
				glCullFace: function(mode) {
					gl.cullFace(mode);
				},
				glBlendFuncSeparate: function(src_rgb, dst_rgb, src_alpha, dst_alpha) {
					gl.blendFuncSeparate(src_rgb, dst_rgb, src_alpha, dst_alpha);
				},
				glBlendEquationSeparate: function(mode_rgb, mode_alpha) {
					gl.blendEquationSeparate(mode_rgb, mode_alpha);
				},
				glGenBuffers: function(n, buffers) {
					for (let i = 0; i < n; ++i) {
						gl_buffers.push(gl.createBuffer());
						heapu32[buffers / 4 + i] = gl_buffers.length - 1;
					}
				},
				glBufferData: function(target, size, data, usage) {
					gl.bufferData(target, heapu8.subarray(data, data + Number(size)), usage);
				},
				glCreateProgram: function() {
					gl_programs.push(gl.createProgram());
					return gl_programs.length - 1;
				},
				glAttachShader: function(program, shader) {
					gl.attachShader(gl_programs[program], gl_shaders[shader]);
				},
				glBindAttribLocation: function(program, index, name) {
					gl.bindAttribLocation(gl_programs[program], index, read_string(name));
				},
				glLinkProgram: function(program) {
					gl.linkProgram(gl_programs[program]);
				},
				glGetProgramiv: function(program, pname, params) {
					heapu32[params / 4] = gl.getProgramParameter(gl_programs[program], pname);
				},
				glGetProgramInfoLog: function(program) {
					console.log(gl.getProgramInfoLog(gl_programs[program]));
				},
				glCreateShader: function(type) {
					gl_shaders.push(gl.createShader(type));
					return gl_shaders.length - 1;
				},
				glShaderSource: function(shader, count, source, length) {
					gl.shaderSource(gl_shaders[shader], read_string(heapu32[source / 4]));
				},
				glCompileShader: function(shader) {
					gl.compileShader(gl_shaders[shader]);
				},
				glGetShaderiv: function(shader, pname, params) {
					heapu32[params / 4] = gl.getShaderParameter(gl_shaders[shader], pname);
				},
				glGetShaderInfoLog: function(shader) {
					console.log(gl.getShaderInfoLog(gl_shaders[shader]));
				},
				glBufferSubData: function(target, offset, size, data) {
					gl.bufferSubData(target, Number(offset), heapu8.subarray(data, data + Number(size)), 0);
				},
				glEnableVertexAttribArray: function(index) {
					gl.enableVertexAttribArray(index);
				},
				glVertexAttribPointer: function(index, size, type, normalized, stride, offset) {
					gl.vertexAttribPointer(index, size, type, normalized, stride, offset);
				},
				glDisableVertexAttribArray: function(index) {
					gl.disableVertexAttribArray(index);
				},
				glGetUniformLocation: function(program, name) {
					gl_locations.push(gl.getUniformLocation(gl_programs[program], read_string(name)));
					return gl_locations.length - 1;
				},
				glUniform1i: function(location, v0) {
					gl.uniform1i(gl_locations[location], v0);
				},
				glUniform2i: function(location, v0, v1) {
					gl.uniform2i(gl_locations[location], v0, v1);
				},
				glUniform3i: function(location, v0, v1, v2) {
					gl.uniform3i(gl_locations[location], v0, v1, v2);
				},
				glUniform4i: function(location, v0, v1, v2, v3) {
					gl.uniform4i(gl_locations[location], v0, v1, v2, v3);
				},
				glUniform1iv: function(location, count, value) {
					gl.uniform1iv(gl_locations[location], count, heapi32.subarray(value / 4));
				},
				glUniform2iv: function(location, count, value) {
					gl.uniform2iv(gl_locations[location], count, heapi32.subarray(value / 4));
				},
				glUniform3iv: function(location, count, value) {
					gl.uniform3iv(gl_locations[location], count, heapi32.subarray(value / 4));
				},
				glUniform4iv: function(location, count, value) {
					gl.uniform4iv(gl_locations[location], count, heapi32.subarray(value / 4));
				},
				glUniform1f: function(location, v0) {
					gl.uniform1f(gl_locations[location], v0);
				},
				glUniform2f: function(location, v0, v1) {
					gl.uniform2f(gl_locations[location], v0, v1);
				},
				glUniform3f: function(location, v0, v1, v2) {
					gl.uniform3f(gl_locations[location], v0, v1, v2);
				},
				glUniform4f: function(location, v0, v1, v2, v3) {
					gl.uniform4f(gl_locations[location], v0, v1, v2, v3);
				},
				glUniform1fv: function(location, count, value) {
					var f32 = new Float32Array(memory.buffer, value, count);
					gl.uniform1fv(gl_locations[location], f32);
				},
				glUniform2fv: function(location, count, value) {
					var f32 = new Float32Array(memory.buffer, value, count * 2);
					gl.uniform2fv(gl_locations[location], f32);
				},
				glUniform3fv: function(location, count, value) {
					var f32 = new Float32Array(memory.buffer, value, count * 3);
					gl.uniform3fv(gl_locations[location], f32);
				},
				glUniform4fv: function(location, count, value) {
					var f32 = new Float32Array(memory.buffer, value, count * 4);
					gl.uniform4fv(gl_locations[location], f32);
				},
				glUniformMatrix3fv: function(location, count, transpose, value) {
					var f32 = new Float32Array(memory.buffer, value, 3 * 3);
					gl.uniformMatrix3fv(gl_locations[location], transpose, f32);
				},
				glUniformMatrix4fv: function(location, count, transpose, value) {
					var f32 = new Float32Array(memory.buffer, value, 4 * 4);
					gl.uniformMatrix4fv(gl_locations[location], transpose, f32);
				},
				glTexParameterf: function(target, pname, param) {
					gl.texParameterf(target, pname, param);
				},
				glActiveTexture: function(texture) {
					gl.activeTexture(texture);
				},
				glBindTexture: function(target, texture) {
					gl.bindTexture(target, gl_textures[texture]);
				},
				glTexParameteri: function(target, pname, param) {
					gl.texParameteri(target, pname, param);
				},
				glGetActiveUniform: function(program, index, bufSize, length, size, type, name) {
					let u = gl.getActiveUniform(gl_programs[program], index);
					heapu32[size / 4] = u.size;
					heapu32[type / 4] = u.type;
					write_string(name, u.name);
				},
				glGenTextures: function(n, textures) {
					for (let i = 0; i < n; ++i) {
						gl_textures.push(gl.createTexture());
						heapu32[textures / 4 + i] = gl_textures.length - 1;
					}
				},
				glTexImage2D: function(target, level, internalformat, width, height, border, format, type, data) {
					let pixels = type == gl.FLOAT ? heapf32.subarray(data / 4) :
								 type == gl.UNSIGNED_INT ? heapu32.subarray(data / 4) :
								 type == gl.UNSIGNED_SHORT ? heapu16.subarray(data / 2) :
								 type == gl.HALF_FLOAT ? heapu16.subarray(data / 2) : heapu8.subarray(data);
					gl.texImage2D(target, level, internalformat, width, height, border, format, type, pixels);
				},
				glPixelStorei: function(pname, param) {
					gl.pixelStorei(pname, param);
				},
				glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data) {
					gl.compressedTexImage2D(target, level, internalformat, width, height, border, imageSize, heapu8.subarray(data));
				},
				glDrawBuffers: function(n, bufs) {
					let ar = [];
					for (let i = 0; i < n; ++i) {
						ar.push(gl.COLOR_ATTACHMENT0 + i);
					}
					gl.drawBuffers(ar);
				},
				glGenerateMipmap: function(target) {
					gl.generateMipmap(target);
				},
				glFlush: function() {
					gl.flush();
				},
				glDeleteBuffers: function(n, buffers) {
					for (let i = 0; i < n; ++i) {
						gl.deleteBuffer(gl_buffers[heapu32[buffers / 4 + i]]);
					}
				},
				glDeleteTextures: function(n, textures) {
					for (let i = 0; i < n; ++i) {
						gl.deleteTexture(gl_textures[heapu32[textures / 4 + i]]);
					}
				},
				glDeleteFramebuffers: function(n, framebuffers) {
					for (let i = 0; i < n; ++i) {
						gl.deleteFramebuffer(gl_framebuffers[heapu32[framebuffers / 4 + i]]);
					}
				},
				glDeleteProgram: function(program) {
					gl.deleteProgram(gl_programs[program]);
				},
				glDeleteShader: function(shader) {
					gl.deleteShader(gl_shaders[shader]);
				},
				js_fprintf: function(format) {
					console.log(read_string(format));
				},
				js_fopen: function(filename) {
					const req = new XMLHttpRequest();
					req.open("GET", read_string(filename), false);
					req.overrideMimeType("text/plain; charset=x-user-defined");
					req.send();
					let str = req.response;
					file_buffer_pos = 0;
					file_buffer = new ArrayBuffer(str.length);
					let buf_view = new Uint8Array(file_buffer);
					for (let i = 0; i < str.length; ++i) {
						buf_view[i] = str.charCodeAt(i);
					}
					return 1;
				},
				js_ftell: function(stream) {
					return file_buffer_pos;
				},
				js_fseek: function(stream, offset, origin) {
					file_buffer_pos = offset;
					if (origin == 1) file_buffer_pos += file_buffer.byteLength; // SEEK_END
					return 0;
				},
				js_fread: function(ptr, size, count, stream) {
					let buf_view = new Uint8Array(file_buffer);
					for (let i = 0; i < count; ++i) {
						heapu8[ptr + i] = buf_view[file_buffer_pos++];
					}
					return count;
				},
				js_time: function() {
					return window.performance.now();
				},
				js_pow: function(x) {
					return Math.pow(x);
				},
				js_floor: function(x) {
					return Math.floor(x);
				},
				js_sin: function(x) {
					return Math.sin(x);
				},
				js_cos: function(x) {
					return Math.cos(x);
				},
				js_tan: function(x) {
					return Math.tan(x);
				},
				js_log: function(base, exponent) {
					return Math.log(base, exponent);
				},
				js_exp: function(x) {
					return Math.exp(x);
				},
				js_sqrt: function(x) {
					return Math.sqrt(x);
				},
				js_eval: function(str) {
					(1, eval)(read_string(str));
				}
			}
		}
	);

	mod = result.module;
	instance = result.instance;
	instance.exports._start();

	function update() {
		instance.exports._update();
		window.requestAnimationFrame(update);
	}
	window.requestAnimationFrame(update);

	kanvas.addEventListener('click', (event) => {
		if (!audio_thread_started) {
			start_audio_thread();
			audio_thread_started = true;
		}
	});

	kanvas.addEventListener('contextmenu', (event) => {
		event.preventDefault();
	});

	kanvas.addEventListener('mousedown', (event) => {
		instance.exports._mousedown(event.button, event.clientX, event.clientY);
	});

	kanvas.addEventListener('mouseup', (event) => {
		instance.exports._mouseup(event.button, event.clientX, event.clientY);
	});

	kanvas.addEventListener('mousemove', (event) => {
		instance.exports._mousemove(event.clientX, event.clientY);
	});

	kanvas.addEventListener('wheel', (event) => {
		instance.exports._wheel(event.deltaY);
	});

	kanvas.addEventListener('keydown', (event) => {
		if (event.repeat) {
			event.preventDefault();
			return;
		}
		instance.exports._keydown(event.keyCode);
	});

	kanvas.addEventListener('keyup', (event) => {
		if (event.repeat) {
			event.preventDefault();
			return;
		}
		instance.exports._keyup(event.keyCode);
	});
}

init();
