
let memory  = null;
let heapu8  = null;
let heapu16 = null;
let heapu32 = null;
let heapi32 = null;
let heapf32 = null;
let module   = null;
let instance = null;

let wgpu_objects = [ null ];

function ptr_to_id(ptr) {
	if (ptr === null) {
		return 0;
	}
	wgpu_objects.push(ptr);
	return wgpu_objects.length - 1;
}

function id_to_ptr(id) {
	return wgpu_objects[id];
}

function read_string(ptr) {
	let str = '';
	for (let i = 0; heapu8[ptr + i] != 0; ++i) {
		str += String.fromCharCode(heapu8[ptr + i]);
	}
	return str;
}

function read_u32(ptr) {
	return heapu32[ptr / 4];
}

function write_string(ptr, str) {
	for (let i = 0; i < str.length; ++i) {
		heapu8[ptr + i] = str.charCodeAt(i);
	}
	heapu8[ptr + str.length] = 0;
}

function id_to_texture_format(id) {
	if (id === 0x00000001)
		return "r8unorm";
	if (id === 0x00000016)
		return "rgba8unorm";
}

function id_to_vertex_format(id) {
	return "float32x3";
}

async function init() {
	let   wasm_bytes = null;
	await fetch("./start.wasm").then(res => res.arrayBuffer()).then(buffer => wasm_bytes = new Uint8Array(buffer));

	memory  = new WebAssembly.Memory({initial : 8295, maximum : 8295, shared : true});
	heapu8  = new Uint8Array(memory.buffer);
	heapu16 = new Uint16Array(memory.buffer);
	heapu32 = new Uint32Array(memory.buffer);
	heapi32 = new Int32Array(memory.buffer);
	heapf32 = new Float32Array(memory.buffer);

	if (!navigator.gpu) {
		throw new Error('WebGPU not supported');
	}
	let adapter = await navigator.gpu.requestAdapter();
	let device  = await adapter.requestDevice();

	let canvas  = document.getElementById('iron');
	let context = canvas.getContext('webgpu');
	let format  = navigator.gpu.getPreferredCanvasFormat();
	context.configure({device, format});

	let file_buffer     = null;
	let file_buffer_pos = 0;

	let result = await WebAssembly.instantiate(wasm_bytes, {
		env : {memory},
		imports : {

			wgpuCreateInstance : function(pdescriptor) {
		        let inst = navigator.gpu;
		        return ptr_to_id(inst);
			},
			// wgpuInstanceRequestAdapter : function(pinstance, poptions, pcallback_info) {
			// 	let callback_index = heapu32[(pcallback_info + 12) >> 2];
			// 	instance.exports.__indirect_function_table.get(callback_index)(0, ptr_to_id(adapter), null, null, null);
			// 	return 0n;
			// },
			wgpuInstanceRequestAdapterSync : function() {
		        return ptr_to_id(adapter);
			},
			wgpuAdapterGetInfo : function(padapter, pinfo) {
		        return 0;
			},
			wgpuAdapterInfoFreeMembers : function(padapter_info) {},
			// wgpuAdapterRequestDevice : function(padapter, pdescriptor, pcallback_info) {
			// 	return 0;
			// },
			wgpuAdapterRequestDeviceSync : function() {
		        return ptr_to_id(device);
			},
			wgpuDeviceCreateTexture : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUTextureDescriptor
		        let desc = {
			        usage : read_u32(pdescriptor + 16),
			        dimension : "2d",
			        size : {width : read_u32(pdescriptor + 32), height : read_u32(pdescriptor + 36), depthOrArrayLayers : 1},
			        format : read_u32(pdescriptor + 44),
			        mipLevelCount : 1,
			        sampleCount : 1
		        };
		        desc.format = id_to_texture_format(desc.format);
		        let texture = device.createTexture(desc);
		        return ptr_to_id(texture);
			},
			wgpuTextureCreateView : function(ptexture, pdescriptor) {
		        let texture = id_to_ptr(ptexture);
		        let desc    = {}; // TODO
		        let view    = texture.createView(desc);
		        return ptr_to_id(view);
			},
			wgpuDeviceGetQueue : function(pdevice) {
		        let device = id_to_ptr(pdevice);
		        return ptr_to_id(device.queue);
			},
			wgpuDeviceCreateBindGroupLayout : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUBindGroupLayoutDescriptor
		        let desc     = {entryCount : read_u32(pdescriptor + 12), entries : []};
		        let pentries = read_u32(pdescriptor + 16);

		        // WGPUBindGroupLayoutEntry
		        for (let i = 0; i < desc.entryCount; ++i) {
			        let e = {
				        binding : read_u32(i * 88 + pentries + 4),
				        visibility : read_u32(i * 88 + pentries + 8),
			        };

			        if (read_u32(i * 88 + pentries + 28) != 0x00000000) { // WGPUBufferBindingType_BindingNotUsed
				        e.buffer = {
					        type : read_u32(i * 88 + pentries + 28),
					        hasDynamicOffset : read_u32(i * 88 + pentries + 32),
					        minBindingSize : read_u32(i * 88 + pentries + 36)
				        };
				        if (e.buffer.type === 0x00000002)
					        e.buffer.type = "uniform";
			        }

			        if (read_u32(i * 88 + pentries + 52) != 0x00000000) { // WGPUSamplerBindingType_BindingNotUsed
				        e.sampler = {type : read_u32(i * 88 + pentries + 52)};
				        if (e.sampler.type === 0x00000002)
					        e.sampler.type = "filtering";
			        }

			        if (read_u32(i * 88 + pentries + 60) != 0x00000000) { // WGPUTextureSampleType_BindingNotUsed
				        e.texture = {
					        sampleType : read_u32(i * 88 + pentries + 60),
					        viewDimension : "2d", // read_u32(i * 88 + pentries + 64),
					        multisampled : read_u32(i * 88 + pentries + 68)
				        };
				        if (e.texture.sampleType === 0x00000002)
					        e.texture.sampleType = "float";
			        }

			        desc.entries.push(e);
		        }
		        let bgl = device.createBindGroupLayout(desc);
		        return ptr_to_id(bgl);
			},
			wgpuDeviceCreateBuffer : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUBufferDescriptor
		        let desc   = {usage : read_u32(pdescriptor + 16), size : read_u32(pdescriptor + 24), mappedAtCreation : read_u32(pdescriptor + 32)};
		        let buffer = device.createBuffer(desc);
		        return ptr_to_id(buffer);
			},
			wgpuBufferGetMappedRange : function(pbuffer, offset, size) {
		        let buffer = id_to_ptr(pbuffer);
		        let ptr    = instance.exports.malloc(size);
		        let ab     = buffer.getMappedRange(offset, size);
		        for (let i = 0; i < ab.length; ++i) {
			        heapu8[ptr + i] = ab[i];
		        }
		        return ptr;
			},
			wgpuBufferUnmap : function(pbuffer) {
		        let buffer = id_to_ptr(pbuffer);
		        buffer.unmap();
			},
			wgpuDeviceCreateCommandEncoder : function(pdevice, pdescriptor) {
		        let device  = id_to_ptr(pdevice);
		        let desc    = {}; // TODO
		        let encoder = device.createCommandEncoder(desc);
		        return ptr_to_id(encoder);
			},
			wgpuCommandEncoderCopyBufferToTexture : function(pcommand_encoder, psource, pdestination, pcopysize) {
		        let encoder = id_to_ptr(pcommand_encoder);
		        // WGPUTexelCopyBufferInfo
		        let source = {
			        layout : {bytesPerRow : read_u32(psource + 8), rowsPerImage : read_u32(psource + 12)},
			        buffer : id_to_ptr(read_u32(psource + 16))
		        };
		        // WGPUTexelCopyTextureInfo
		        let destination = {texture : id_to_ptr(read_u32(pdestination))};
		        // WGPUExtent3D
		        let copysize = {width : read_u32(pcopysize), height : read_u32(pcopysize + 4), depthOrArrayLayers : read_u32(pcopysize + 8)};
		        encoder.copyBufferToTexture(source, destination, copysize);
			},
			wgpuCommandEncoderFinish : function(pcommand_encoder, pdescriptor) {
		        let encoder        = id_to_ptr(pcommand_encoder);
		        let desc           = {}; // TODO: read WGPUCommandBufferDescriptor
		        let command_buffer = encoder.finish(desc);
		        return ptr_to_id(command_buffer);
			},
			wgpuQueueSubmit : function(pqueue, command_count, pcommands) {
		        let queue    = id_to_ptr(pqueue);
		        let commands = [];
		        for (let i = 0; i < command_count; i++) {
			        commands.push(id_to_ptr(heapu32[(pcommands >> 2) + i]));
		        }
		        queue.submit(commands);
			},
			wgpuBufferRelease : function(pbuffer) {},
			wgpuDeviceCreateSampler : function(pdevice, pdescriptor) {
		        let device  = id_to_ptr(pdevice);
		        // WGPUSamplerDescriptor
				let desc    = {};
		        let sampler = device.createSampler(desc);
		        return ptr_to_id(sampler);
			},
			wgpuSurfaceGetCapabilities : function(psurface, padapter, pcapabilities) {
		        return 0;
			},
			wgpuSurfaceCapabilitiesFreeMembers : function(pcapabilities) {},
			wgpuBufferDestroy : function(pbuffer) {
		        let buffer = id_to_ptr(pbuffer);
		        buffer.destroy();
			},
			wgpuSurfaceGetCurrentTexture : function(psurface, psurface_texture) {
		        let surface = id_to_ptr(psurface) || context;
		        let texture = surface.getCurrentTexture();
		        // WGPUSurfaceTexture
			},
			wgpuCommandEncoderBeginRenderPass : function(pcommand_encoder, pdescriptor) {
		        let encoder     = id_to_ptr(pcommand_encoder);
		        // WGPURenderPassDescriptor
				let desc        = {};
		        let render_pass = encoder.beginRenderPass(desc);
		        return ptr_to_id(render_pass);
			},
			wgpuRenderPassEncoderSetViewport : function(prender_pass_encoder, x, y, width, height, min_depth, max_depth) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        render_pass.setViewport(x, y, width, height, min_depth, max_depth);
			},
			wgpuRenderPassEncoderSetScissorRect : function(prender_pass_encoder, x, y, width, height) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        render_pass.setScissorRect(x, y, width, height);
			},
			wgpuRenderPassEncoderEnd : function(prender_pass_encoder) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        render_pass.end();
			},
			wgpuRenderPassEncoderRelease : function(prender_pass_encoder) {},
			wgpuCommandBufferRelease : function(commandBufferPtr) {},
			wgpuCommandEncoderRelease : function(pcommand_encoder) {},
			wgpuSurfacePresent : function(psurface) {
		        return 0;
			},
			wgpuRenderPassEncoderDrawIndexed : function(prender_pass_encoder, index_count, instance_count, first_index, base_vertex, first_instance) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        render_pass.drawIndexed(index_count, instance_count, first_index, base_vertex, first_instance);
			},
			wgpuRenderPassEncoderSetPipeline : function(prender_pass_encoder, ppipeline) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        let pipeline    = id_to_ptr(ppipeline);
		        render_pass.setPipeline(pipeline);
			},
			wgpuRenderPassEncoderSetVertexBuffer : function(prender_pass_encoder, slot, pbuffer, offset, size) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        let buffer      = id_to_ptr(pbuffer);
		        render_pass.setVertexBuffer(slot, buffer, offset, size);
			},
			wgpuRenderPassEncoderSetIndexBuffer : function(prender_pass_encoder, pbuffer, format, offset, size) {
		        let render_pass = id_to_ptr(prender_pass_encoder);
		        let buffer      = id_to_ptr(pbuffer);
		        render_pass.setIndexBuffer(buffer, 'uint32', offset, size);
			},
			wgpuDeviceCreateBindGroup : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUBindGroupDescriptor
				let desc   = {};
		        let bg     = device.createBindGroup(desc);
		        return ptr_to_id(bg);
			},
			wgpuRenderPassEncoderSetBindGroup : function(prender_pass_encoder, group_index, pgroup, dynamic_offset_count, pdynamic_offsets) {
		        let render_pass     = id_to_ptr(prender_pass_encoder);
		        let group           = id_to_ptr(pgroup);
		        let dynamic_offsets = [];
		        for (let i = 0; i < dynamic_offset_count; i++) {
			        dynamic_offsets.push(heapu32[(pdynamic_offsets >> 2) + i]);
		        }
		        render_pass.setBindGroup(group_index, group, dynamic_offsets);
			},
			wgpuBindGroupRelease : function(pbind_group) {},
			wgpuRenderPipelineRelease : function(prender_pipeline) {},
			wgpuPipelineLayoutRelease : function(ppipeline_layout) {},
			wgpuDeviceCreatePipelineLayout : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUPipelineLayoutDescriptor
		        let desc = {bindGroupLayoutCount : read_u32(pdescriptor + 24), bindGroupLayouts : []};
		        let pbgl = read_u32(pdescriptor + 28);
		        for (let i = 0; i < desc.bindGroupLayoutCount; ++i) {
			        // WGPUBindGroupLayout
			        let bgl = id_to_ptr(read_u32(pbgl + i * 4));
			        desc.bindGroupLayouts.push(bgl);
		        }
		        let pl = device.createPipelineLayout(desc);
		        return ptr_to_id(pl);
			},
			wgpuDeviceCreateShaderModule : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);
		        // WGPUShaderSourceWGSL
		        let pwgsl = read_u32(pdescriptor);
		        let wgsl  = {chain : {sType : read_u32(pwgsl + 4)}, code : {data : read_u32(pwgsl + 8), length : read_u32(pwgsl + 16)}};
		        // WGPUShaderModuleDescriptor
		        // let desc   = { nextInChain: wgsl };
		        let desc = {code : wgsl.code};
		        let sm   = device.createShaderModule(desc);
		        return ptr_to_id(sm);
			},
			wgpuDeviceCreateRenderPipeline : function(pdevice, pdescriptor) {
		        let device = id_to_ptr(pdevice);

		        // WGPUFragmentState
		        let pfrag    = read_u32(pdescriptor + 128);
		        let frag     = {module : id_to_ptr(read_u32(pfrag + 4)), entryPoint : "main", targetCount : 1, targets : []};
		        let ptragets = read_u32(pdescriptor + 48);
		        for (let i = 0; i < frag.targetCount; ++i) {
			        // WGPUColorTargetState
			        let t = {format : id_to_texture_format(read_u32(ptragets + 4 + i * 24)), blend : null, writeMask : read_u32(ptragets + 12 + i * 24)};
			        frag.targets.push(t);
		        }

		        // WGPURenderPipelineDescriptor
		        let desc = {
			        layout : id_to_ptr(read_u32(pdescriptor + 20)),
			        // WGPUVertexState
			        vertex : {module : id_to_ptr(read_u32(pdescriptor + 28)), entryPoint : "main", bufferCount : 1, buffers : []},
			        primitive : {topology : "trianglelist", frontFace : "ccw", cullMode : "none"},
			        depthStencil : null,
			        multisample : {count : 1, mask : 0},
			        fragment : frag,
		        };

		        let pbuffers = read_u32(pdescriptor + 64);
		        for (let i = 0; i < desc.vertex.bufferCount; ++i) {
			        // WGPUVertexBufferLayout
			        let b           = {arrayStride : read_u32(pbuffers + 8 + i * 28), attributeCount : read_u32(pbuffers + 16 + i * 28), attributes : []};
			        let pattributes = read_u32(pbuffers + 24 + i * 28);
			        for (let i = 0; i < b.attributeCount; ++i) {
				        // WGPUVertexAttribute
				        let a = {
					        format : id_to_vertex_format(read_u32(pattributes + 4 + i * 20)),
					        offset : read_u32(pattributes + 8 + i * 20),
					        shaderLocation : read_u32(pattributes + 16 + i * 20)
				        };
				        b.attributes.push(a);
			        }
			        desc.vertex.buffers.push(b);
		        }

		        let rp = device.createRenderPipeline(desc);
		        return ptr_to_id(rp);
			},
			wgpuShaderModuleRelease : function(pshader_module) {},
			wgpuQueueWriteBuffer : function(pqueue, pbuffer, buffer_offset, pdata, size) {
		        let queue  = id_to_ptr(pqueue) || device.queue;
		        let buffer = id_to_ptr(pbuffer);
		        let data   = heapu8.subarray(pdata, pdata + size);
		        queue.writeBuffer(buffer, buffer_offset, data);
			},
			wgpuTextureDestroy : function(ptexture) {
		        let texture = id_to_ptr(ptexture);
		        texture.destroy();
			},
			wgpuTextureRelease : function(ptexture) {},
			wgpuTextureViewRelease : function(ptexture_view) {},
			wgpuCommandEncoderCopyBufferToBuffer : function(pcommand_encoder, psource, source_offset, pdestination, destination_offset, size) {
		        let encoder     = id_to_ptr(pcommand_encoder);
		        let source      = id_to_ptr(psource);
		        let destination = id_to_ptr(pdestination);
		        encoder.copyBufferToBuffer(source, source_offset, destination, destination_offset, size);
			},
			wgpuSurfaceConfigure : function(psurface, pconfig) {
		        let surface = id_to_ptr(psurface) || context;
		        // WGPUSurfaceConfiguration
		        let config = {
			        device : id_to_ptr(read_u32(pconfig + 4)),
			        format : id_to_texture_format(read_u32(pconfig + 8)),
			        usage : read_u32(pconfig + 16),
			        width : read_u32(pconfig + 24),
			        height : read_u32(pconfig + 28),
			        viewFormatCount : read_u32(pconfig + 32),
			        viewFormats : [],
			        alphaMode : read_u32(pconfig + 40),
			        presentMode : read_u32(pconfig + 44)
		        };
		        if (config.alphaMode === 0x00000001)
			        config.alphaMode = "opaque";
		        if (config.presentMode === 0x00000001)
			        config.presentMode = "fifo";
		        if (config.presentMode === 0x00000004)
			        config.presentMode = "mailbox";
		        let pview_formats = read_u32(pconfig + 36);
		        for (let i = 0; i < config.viewFormatCount; ++i) {
			        let f = id_to_texture_format(read_u32(pview_formats + i * 4));
			        config.viewFormats.push(f);
		        }
		        surface.configure(config);
			},

			js_fprintf : function(format) {
		        console.log(read_string(format));
			},
			js_fopen : function(filename) {
		        let req = new XMLHttpRequest();
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
