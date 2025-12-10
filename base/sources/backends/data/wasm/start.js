
let memory   = null;
let heapu8   = null;
let heapu16  = null;
let heapu32  = null;
let heapi32  = null;
let heapf32  = null;
let module   = null;
let instance = null;

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

async function init_webgpu(canvas) {
	if (!navigator.gpu) {
		throw new Error('WebGPU not supported');
	}
	const adapter = await navigator.gpu.requestAdapter();
	if (!adapter) {
		throw new Error('No GPU adapter found');
	}
	const device = await adapter.requestDevice();

	const context = canvas.getContext('webgpu');
	const format  = navigator.gpu.getPreferredCanvasFormat();

	context.configure({device, format});
	return {context, device};
}

async function init() {
	let   wasm_bytes = null;
	await fetch("./start.wasm").then(res => res.arrayBuffer()).then(buffer => wasm_bytes = new Uint8Array(buffer));

	// Read memory size from wasm file
	let memory_size = 0;
	let i           = 8;
	while (i < wasm_bytes.length) {
		function read_leb() {
			let result = 0;
			let shift  = 0;
			while (true) {
				let byte = wasm_bytes[i++];
				result |= (byte & 0x7f) << shift;
				if ((byte & 0x80) == 0)
					return result;
				shift += 7;
			}
		}
		let type   = read_leb()
		let length = read_leb()
		if (type == 6) {
			read_leb(); // count
			i++;        // gtype
			i++;        // mutable
			read_leb(); // opcode
			memory_size = read_leb() / 65536 + 1;
			break;
		}
		i += length;
	}

	memory  = new WebAssembly.Memory({initial : memory_size, maximum : memory_size, shared : true});
	heapu8  = new Uint8Array(memory.buffer);
	heapu16 = new Uint16Array(memory.buffer);
	heapu32 = new Uint32Array(memory.buffer);
	heapi32 = new Int32Array(memory.buffer);
	heapf32 = new Float32Array(memory.buffer);

	const canvas = document.getElementById('iron');

	const {context, device} = await init_webgpu(canvas);

	let file_buffer     = null;
	let file_buffer_pos = 0;

	const result = await WebAssembly.instantiate(wasm_bytes, {
		env : {memory},
		imports : {

			wgpuDeviceCreateTexture : function(device, descriptor) {
				return null; // WGPUTexture
			},
			wgpuTextureCreateView : function(texture, descriptor) {
				return null; // WGPUTextureView
			},
			wgpuCreateInstance : function(descriptor) {
				return null; // WGPUInstance
			},
			wgpuInstanceRequestAdapter : function(instance, options, callbackInfo) {
				return null; // WGPUFuture
			},
			wgpuAdapterGetInfo : function(adapter, info) {
				return null; // WGPUStatus
			},
			wgpuAdapterInfoFreeMembers : function(adapterInfo) {
			},
			wgpuAdapterRequestDevice : function(adapter, descriptor, callbackInfo) {
				return null; // WGPUFuture
			},
			wgpuDeviceGetQueue : function(device) {
				return null; // WGPUQueue
			},
			wgpuDeviceCreateBindGroupLayout : function(device, descriptor) {
				return null; // WGPUBindGroupLayout
			},
			wgpuDeviceCreateBuffer : function(device, descriptor) {
				return null; // WGPUBuffer
			},
			wgpuBufferGetMappedRange : function(buffer, offset, size) {
				return null; // void *
			},
			wgpuBufferUnmap : function(buffer) {
			},
			wgpuDeviceCreateCommandEncoder : function(device, descriptor) {
				return null; // WGPUCommandEncoder
			},
			wgpuCommandEncoderCopyBufferToTexture : function(commandEncoder, source, destination, copySize) {
			},
			wgpuCommandEncoderFinish : function(commandEncoder, descriptor) {
				return null; // WGPUCommandBuffer
			},
			wgpuQueueSubmit : function(queue, commandCount, commands) {
			},
			wgpuBufferRelease : function(buffer) {
			},
			wgpuDeviceCreateSampler : function(device, descriptor) {
				return null; // WGPUSampler
			},
			wgpuSurfaceGetCapabilities : function(surface, adapter, capabilities) {
				return null; // WGPUStatus
			},
			wgpuSurfaceCapabilitiesFreeMembers : function(surfaceCapabilities) {
			},
			wgpuBufferDestroy : function(buffer) {
			},
			wgpuSurfaceGetCurrentTexture : function(surface, surfaceTexture) {
			},
			wgpuCommandEncoderBeginRenderPass : function(commandEncoder, descriptor) {
				return null; // WGPURenderPassEncoder
			},
			wgpuRenderPassEncoderSetViewport : function(renderPassEncoder, x, y, width, height, minDepth, maxDepth) {
			},
			wgpuRenderPassEncoderSetScissorRect : function(renderPassEncoder, x, y, width, height) {
			},
			wgpuRenderPassEncoderEnd : function(renderPassEncoder) {
			},
			wgpuRenderPassEncoderRelease : function(renderPassEncoder) {
			},
			wgpuCommandBufferRelease : function(commandBuffer) {
			},
			wgpuCommandEncoderRelease : function(commandEncoder) {
			},
			wgpuSurfacePresent : function(surface) {
				return null; // WGPUStatus
			},
			wgpuRenderPassEncoderDrawIndexed : function(renderPassEncoder, indexCount, instanceCount, firstIndex, baseVertex, firstInstance) {
			},
			wgpuRenderPassEncoderSetPipeline : function(renderPassEncoder, pipeline) {
			},
			wgpuRenderPassEncoderSetVertexBuffer : function(renderPassEncoder, slot, buffer, offset, size) {
			},
			wgpuRenderPassEncoderSetIndexBuffer : function(renderPassEncoder, buffer, format, offset, size) {
			},
			wgpuDeviceCreateBindGroup : function(device, descriptor) {
				return null; // WGPUBindGroup
			},
			wgpuRenderPassEncoderSetBindGroup : function(renderPassEncoder, groupIndex, group, dynamicOffsetCount, dynamicOffsets) {
			},
			wgpuBindGroupRelease : function(bindGroup) {
			},
			wgpuRenderPipelineRelease : function(renderPipeline) {
			},
			wgpuPipelineLayoutRelease : function(pipelineLayout) {
			},
			wgpuDeviceCreatePipelineLayout : function(device, descriptor) {
				return null; // WGPUPipelineLayout
			},
			wgpuDeviceCreateShaderModule : function(device, descriptor) {
				return null; // WGPUShaderModule
			},
			wgpuDeviceCreateRenderPipeline : function(device, descriptor) {
				return null; // WGPURenderPipeline
			},
			wgpuShaderModuleRelease : function(shaderModule) {
			},
			wgpuQueueWriteBuffer : function(queue, buffer, bufferOffset, data, size) {
			},
			wgpuTextureDestroy : function(texture) {
			},
			wgpuTextureRelease : function(texture) {
			},
			wgpuTextureViewRelease : function(textureView) {
			},
			wgpuCommandEncoderCopyBufferToBuffer : function(commandEncoder, source, sourceOffset, destination, destinationOffset, size) {
			},
			wgpuSurfaceConfigure : function(surface, config) {
			},

			js_fprintf : function(format) {
		        console.log(read_string(format));
			},
			js_fopen : function(filename) {
		        const req = new XMLHttpRequest();
		        req.open("GET", read_string(filename), false);
		        req.overrideMimeType("text/plain; charset=x-user-defined");
		        req.send();
		        let str         = req.response;
		        file_buffer_pos = 0;
		        file_buffer     = new ArrayBuffer(str.length);
		        let buf_view    = new Uint8Array(file_buffer);
		        for (let i = 0; i < str.length; ++i) {
			        buf_view[i] = str.charCodeAt(i);
		        }
		        return 1;
			},
			js_ftell : function(stream) {
		        return file_buffer_pos;
			},
			js_fseek : function(stream, offset, origin) {
		        file_buffer_pos = offset;
		        if (origin == 1)
			        file_buffer_pos += file_buffer.byteLength; // SEEK_END
		        return 0;
			},
			js_fread : function(ptr, size, count, stream) {
		        let buf_view = new Uint8Array(file_buffer);
		        for (let i = 0; i < count; ++i) {
			        heapu8[ptr + i] = buf_view[file_buffer_pos++];
		        }
		        return count;
			},
			js_time : function() {
		        return window.performance.now();
			},
			js_pow : function(x) {
		        return Math.pow(x);
			},
			js_floor : function(x) {
		        return Math.floor(x);
			},
			js_sin : function(x) {
		        return Math.sin(x);
			},
			js_cos : function(x) {
		        return Math.cos(x);
			},
			js_tan : function(x) {
		        return Math.tan(x);
			},
			js_log : function(base, exponent) {
		        return Math.log(base, exponent);
			},
			js_exp : function(x) {
		        return Math.exp(x);
			},
			js_sqrt : function(x) {
		        return Math.sqrt(x);
			},
			js_eval : function(str) {
		        (1, eval)(read_string(str));
			}
		}
	});

	module   = result.module;
	instance = result.instance;
	instance.exports.wasm_start();

	function update() {
		instance.exports.wasm_update();
		window.requestAnimationFrame(update);
	}
	window.requestAnimationFrame(update);

	canvas.addEventListener('contextmenu', (event) => { event.preventDefault(); });
	canvas.addEventListener('mousedown', (event) => { instance.exports.wasm_mousedown(event.button, event.clientX, event.clientY); });
	canvas.addEventListener('mouseup', (event) => { instance.exports.wasm_mouseup(event.button, event.clientX, event.clientY); });
	canvas.addEventListener('mousemove', (event) => { instance.exports.wasm_mousemove(event.clientX, event.clientY); });
	canvas.addEventListener('wheel', (event) => { instance.exports.wasm_wheel(event.deltaY); });
	canvas.addEventListener('keydown', (event) => {
		if (event.repeat) {
			event.preventDefault();
			return;
		}
		instance.exports.wasm_keydown(event.keyCode);
	});
	canvas.addEventListener('keyup', (event) => {
		if (event.repeat) {
			event.preventDefault();
			return;
		}
		instance.exports.wasm_keyup(event.keyCode);
	});
}

init();
