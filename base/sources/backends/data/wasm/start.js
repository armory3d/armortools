
let memory       = null;
let heapu8       = null;
let heapu16      = null;
let heapu32      = null;
let heapi32      = null;
let heapf32      = null;
let module       = null;
let instance     = null;
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

function read_string_n(ptr, n) {
	let str = '';
	for (let i = 0; i < n; ++i) {
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
		return "bgra8unorm";
	// return "rgba8unorm";
	if (id === 0x00000030)
		return "depth32float";
	if (id === 0x0000001B)
		return "bgra8unorm";
}

function id_to_vertex_format(id) {
	if (id === 0x0000001C)
		return "float32";
	if (id === 0x0000001D)
		return "float32x2";
	if (id === 0x0000001E)
		return "float32x3";
	if (id === 0x0000001F)
		return "float32x4";
	if (id === 0x00000017)
		return "snorm16x2";
	if (id === 0x00000018)
		return "snorm16x4";
}

async function init() {
	let   wasm_bytes = null;
	await fetch("./start.wasm").then(res => res.arrayBuffer()).then(buffer => wasm_bytes = new Uint8Array(buffer));

	memory  = new WebAssembly.Memory({initial : 20000, maximum : 20000, shared : true}); // * 65536
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
					size : {width : read_u32(pdescriptor + 28), height : read_u32(pdescriptor + 32), depthOrArrayLayers : 1},
					format : id_to_texture_format(read_u32(pdescriptor + 40)),
					mipLevelCount : 1,
					sampleCount : 1
				};
				let texture = device.createTexture(desc);
				return ptr_to_id(texture);
			},
			wgpuTextureCreateView : function(ptexture, pdescriptor) {
				let texture = id_to_ptr(ptexture);
				// WGPUTextureViewDescriptor
				let desc =
					{format : id_to_texture_format(read_u32(pdescriptor + 12)), dimension : "2d", mipLevelCount : 1, arrayLayerCount : 1, aspect : "all"};
				let view = texture.createView(desc);
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
				let u8     = new Uint8Array(ab);
				for (let i = 0; i < u8.length; ++i) {
					heapu8[ptr + i] = u8[i];
				}
				return ptr;
			},
			wgpuBufferUnmap : function(pbuffer) {
				let buffer = id_to_ptr(pbuffer);
				buffer.unmap();
			},
			wgpuBufferUnmap2 : function(pbuffer, pdata, start, count) {
				let buffer = id_to_ptr(pbuffer);
				let ab     = buffer.getMappedRange(start, count);
				let u8     = new Uint8Array(ab);
				for (let i = 0; i < u8.length; ++i) {
					u8[i] = heapu8[pdata + i];
				}
				buffer.unmap();
			},
			wgpuDeviceCreateCommandEncoder : function(pdevice, pdescriptor) {
				let device = id_to_ptr(pdevice);
				// WGPUCommandBufferDescriptor
				let desc    = null;
				let encoder = device.createCommandEncoder(desc);
				return ptr_to_id(encoder);
			},
			wgpuCommandEncoderCopyBufferToTexture : function(pcommand_encoder, psource, pdestination, pcopysize) {
				let encoder = id_to_ptr(pcommand_encoder);
				// WGPUTexelCopyBufferInfo
				let source = {bytesPerRow : read_u32(psource + 8), rowsPerImage : read_u32(psource + 12), buffer : id_to_ptr(read_u32(psource + 16))};
				// WGPUTexelCopyTextureInfo
				let destination = {texture : id_to_ptr(read_u32(pdestination))};
				// WGPUExtent3D
				let copysize = {width : read_u32(pcopysize), height : read_u32(pcopysize + 4), depthOrArrayLayers : read_u32(pcopysize + 8)};
				encoder.copyBufferToTexture(source, destination, copysize);
			},
			wgpuCommandEncoderFinish : function(pcommand_encoder, pdescriptor) {
				let encoder = id_to_ptr(pcommand_encoder);
				// WGPUCommandBufferDescriptor
				let desc           = null;
				let command_buffer = encoder.finish(desc);
				return ptr_to_id(command_buffer);
			},
			wgpuQueueSubmit : function(pqueue, command_buffer_count, pcommand_buffers) {
				let queue           = id_to_ptr(pqueue);
				let command_buffers = [];
				for (let i = 0; i < command_buffer_count; i++) {
					let c = id_to_ptr(read_u32(pcommand_buffers + i * 4));
					command_buffers.push(c);
				}
				queue.submit(command_buffers);
			},
			wgpuBufferRelease : function(pbuffer) {},
			wgpuDeviceCreateSampler : function(pdevice, pdescriptor) {
				let device = id_to_ptr(pdevice);
				// WGPUSamplerDescriptor
				let desc = {
					addressModeU : "repeat",
					addressModeV : "repeat",
					addressModeW : "repeat",
					magFilter : "linear",
					minFilter : "linear",
					mipmapFilter : "linear",
					maxAnisotropy : 1
				};
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
				heapu32[psurface_texture / 4 + 1] = ptr_to_id(texture);
			},
			wgpuCommandEncoderBeginRenderPass : function(pcommand_encoder, pdescriptor) {
				let encoder = id_to_ptr(pcommand_encoder);

				// WGPURenderPassDescriptor
				let desc = {colorAttachmentCount : read_u32(pdescriptor + 12), colorAttachments : []};
				let pcas = read_u32(pdescriptor + 16);
				for (let i = 0; i < desc.colorAttachmentCount; ++i) {
					// WGPURenderPassColorAttachment
					let ca = {view : id_to_ptr(read_u32(pcas + 4 + i * 28)), loadOp : "clear", storeOp : "store", clearValue : [ 0.0, 0.0, 0.0, 0.0 ]};
					desc.colorAttachments.push(ca);
				}

				// WGPURenderPassDepthStencilAttachment
				let pdsa = read_u32(pdescriptor + 20);
				if (pdsa !== 0) {
					desc.depthStencilAttachment = {view : id_to_ptr(read_u32(pdsa + 4)), depthLoadOp : "clear", depthStoreOp : "store", depthClearValue : 1.0};
				}

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
				render_pass.setVertexBuffer(slot, buffer, Number(offset), Number(size));
			},
			wgpuRenderPassEncoderSetIndexBuffer : function(prender_pass_encoder, pbuffer, format, offset, size) {
				let render_pass = id_to_ptr(prender_pass_encoder);
				let buffer      = id_to_ptr(pbuffer);
				render_pass.setIndexBuffer(buffer, 'uint32', Number(offset), Number(size));
			},
			wgpuDeviceCreateBindGroup : function(pdevice, pdescriptor) {
				let device = id_to_ptr(pdevice);
				// WGPUBindGroupDescriptor
				let desc     = {layout : id_to_ptr(read_u32(pdescriptor + 12)), entryCount : read_u32(pdescriptor + 16), entries : []};
				let pentries = read_u32(pdescriptor + 20);
				for (let i = 0; i < desc.entryCount; i++) {
					// WGPUBindGroupEntry
					let e = {
						binding : read_u32(pentries + 4 + i * 40),
					};

					if (read_u32(pentries + 8 + i * 40) !== 0) {
						e.resource = {
							buffer : id_to_ptr(read_u32(pentries + 8 + i * 40)),
							offset : read_u32(pentries + 16 + i * 40),
							size : read_u32(pentries + 24 + i * 40)
						}
					}
					if (read_u32(pentries + 32 + i * 40) !== 0) {
						e.resource = id_to_ptr(read_u32(pentries + 32 + i * 40));
					}
					if (read_u32(pentries + 36 + i * 40) !== 0) {
						e.resource = id_to_ptr(read_u32(pentries + 36 + i * 40));
					}

					desc.entries.push(e);
				}
				let bg = device.createBindGroup(desc);
				return ptr_to_id(bg);
			},
			wgpuRenderPassEncoderSetBindGroup : function(prender_pass_encoder, group_index, pgroup, dynamic_offset_count, pdynamic_offsets) {
				let render_pass     = id_to_ptr(prender_pass_encoder);
				let group           = id_to_ptr(pgroup);
				let dynamic_offsets = [];
				for (let i = 0; i < dynamic_offset_count; i++) {
					let doff = read_u32(pdynamic_offsets + i * 4);
					dynamic_offsets.push(doff);
				}
				render_pass.setBindGroup(group_index, group, dynamic_offsets);
			},
			wgpuBindGroupRelease : function(pbind_group) {},
			wgpuRenderPipelineRelease : function(prender_pipeline) {},
			wgpuPipelineLayoutRelease : function(ppipeline_layout) {},
			wgpuDeviceCreatePipelineLayout : function(pdevice, pdescriptor) {
				let device = id_to_ptr(pdevice);
				// WGPUPipelineLayoutDescriptor
				let desc = {bindGroupLayoutCount : read_u32(pdescriptor + 12), bindGroupLayouts : []};
				let pbgl = read_u32(pdescriptor + 16);
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
				let wgsl  = {chain : {sType : read_u32(pwgsl + 4)}, code : {data : read_u32(pwgsl + 8), length : read_u32(pwgsl + 12)}};
				// WGPUShaderModuleDescriptor
				let desc = {code : read_string_n(wgsl.code.data, wgsl.code.length)};
				let sm   = device.createShaderModule(desc);
				return ptr_to_id(sm);
			},
			wgpuDeviceCreateRenderPipeline : function(pdevice, pdescriptor) {
				let device = id_to_ptr(pdevice);

				// WGPUFragmentState
				let pfrag    = read_u32(pdescriptor + 92);
				let frag     = {module : id_to_ptr(read_u32(pfrag + 4)), entryPoint : "main", targetCount : 1, targets : []};
				let ptragets = read_u32(pfrag + 28);
				for (let i = 0; i < frag.targetCount; ++i) {
					// WGPUColorTargetState
					let t = {
						format : id_to_texture_format(read_u32(ptragets + 4 + i * 24)),
						blend : {
							color : {operation : "add", srcFactor : "one", dstFactor : "zero"},
							alpha : {operation : "add", srcFactor : "one", dstFactor : "zero"}
						},
						writeMask : read_u32(ptragets + 16 + i * 24)
					};
					frag.targets.push(t);
				}

				// WGPUDepthStencilState
				let pds = read_u32(pdescriptor + 72);
				let ds  = {format : id_to_texture_format(read_u32(pds + 4)), depthWriteEnabled : read_u32(pds + 8), depthCompare : read_u32(pds + 12)};
				if (ds.depthCompare === 0x00000002)
					ds.depthCompare = "less";
				if (ds.depthCompare === 0x00000008)
					ds.depthCompare = "always";

				// WGPURenderPipelineDescriptor
				let desc = {
					layout : id_to_ptr(read_u32(pdescriptor + 12)),
					// WGPUVertexState
					vertex : {module : id_to_ptr(read_u32(pdescriptor + 20)), entryPoint : "main", bufferCount : 1, buffers : []},
					primitive : {topology : "triangle-list", frontFace : "ccw", cullMode : "none"},
					depthStencil : ds,
					fragment : frag,
				};

				let pbuffers = read_u32(pdescriptor + 44);
				for (let i = 0; i < desc.vertex.bufferCount; ++i) {
					// WGPUVertexBufferLayout
					let b           = {arrayStride : read_u32(pbuffers + 8 + i * 24), attributeCount : read_u32(pbuffers + 16 + i * 24), attributes : []};
					let pattributes = read_u32(pbuffers + 20 + i * 24);
					for (let i = 0; i < b.attributeCount; ++i) {
						// WGPUVertexAttribute
						let a = {
							format : id_to_vertex_format(read_u32(pattributes + 4 + i * 24)),
							offset : read_u32(pattributes + 8 + i * 24),
							shaderLocation : read_u32(pattributes + 16 + i * 24)
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
				let queue  = id_to_ptr(pqueue);
				let buffer = id_to_ptr(pbuffer);
				let data   = heapu8.subarray(pdata, pdata + size);
				queue.writeBuffer(buffer, Number(buffer_offset), data);
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
				encoder.copyBufferToBuffer(source, Number(source_offset), destination, Number(destination_offset), Number(size));
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

			js_printf : function(format) {
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
			},
			js_canvas_w : function() {
				return canvas.width;
			},
			js_canvas_h : function() {
				return canvas.height;
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
