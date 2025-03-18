#include <string.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <emscripten/html5_webgpu.h>
#include <webgpu/webgpu.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_system.h>
#include <stdlib.h>
#include <assert.h>

int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapChain;

void iron_gpu_internal_destroy_window() {}

void iron_gpu_internal_destroy() {}

void iron_gpu_internal_init() {}

void iron_gpu_internal_init_window(int depthBufferBits, bool vsync) {
	newRenderTargetWidth = renderTargetWidth = iron_width();
	newRenderTargetHeight = renderTargetHeight = iron_height();

	device = emscripten_webgpu_get_device();
	queue = wgpuDeviceGetQueue(device);

	WGPUSurfaceDescriptorFromCanvasHTMLSelector canvasDesc;
	memset(&canvasDesc, 0, sizeof(canvasDesc));
	canvasDesc.selector = "canvas";

	WGPUSurfaceDescriptor surfDesc;
	memset(&surfDesc, 0, sizeof(surfDesc));
	surfDesc.nextInChain = &canvasDesc;
	WGPUInstance instance = 0;
	WGPUSurface surface = wgpuInstanceCreateSurface(instance, &surfDesc);

	WGPUSwapChainDescriptor scDesc;
	memset(&scDesc, 0, sizeof(scDesc));
	scDesc.usage = WGPUTextureUsage_RenderAttachment;
	scDesc.format = WGPUTextureFormat_BGRA8Unorm;
	scDesc.width = iron_width();
	scDesc.height = iron_height();
	scDesc.presentMode = WGPUPresentMode_Fifo;
	swapChain = wgpuDeviceCreateSwapChain(device, surface, &scDesc);
}

void iron_gpu_begin(iron_gpu_texture_t *renderTarget) {}

void iron_gpu_end() {}

bool iron_gpu_swap_buffers() {
	return true;
}

bool iron_gpu_raytrace_supported() {
	return false;
}

extern WGPUDevice device;

iron_gpu_buffer_t *iron_gpu_internal_current_vertex_buffer = NULL;

void iron_gpu_vertex_buffer_init(iron_gpu_buffer_t *buffer, int count, iron_gpu_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += iron_gpu_vertex_data_size(structure->elements[i].data);
	}
}

void iron_gpu_vertex_buffer_destroy(iron_gpu_buffer_t *buffer) {

}

float *iron_gpu_vertex_buffer_lock_all(iron_gpu_buffer_t *buffer) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = buffer->impl.count * buffer->impl.stride * sizeof(float);
	bDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, 0, bDesc.size);
}

float *iron_gpu_vertex_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count) {
	return NULL;
}

void iron_gpu_vertex_buffer_unlock_all(iron_gpu_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void iron_gpu_vertex_buffer_unlock(iron_gpu_buffer_t* buffer, int count) {

}

int iron_gpu_vertex_buffer_count(iron_gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

int iron_gpu_vertex_buffer_stride(iron_gpu_buffer_t *buffer) {
	return buffer->impl.stride;
}

bool iron_gpu_transpose_mat = false;

void iron_gpu_constant_buffer_init(iron_gpu_buffer_t *buffer, int size) {

}

void iron_gpu_constant_buffer_destroy(iron_gpu_buffer_t *buffer) {}

void iron_gpu_constant_buffer_lock_all(iron_gpu_buffer_t *buffer) {
	iron_gpu_constant_buffer_lock(buffer, 0, iron_gpu_constant_buffer_size(buffer));
}

void iron_gpu_constant_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count) {}

void iron_gpu_constant_buffer_unlock(iron_gpu_buffer_t *buffer) {

}

int iron_gpu_constant_buffer_size(iron_gpu_buffer_t *buffer) {
	return 0;
}

iron_gpu_buffer_t *iron_gpu_internal_current_index_buffer = NULL;

void iron_gpu_index_buffer_init(iron_gpu_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.count = count;
}

void iron_gpu_index_buffer_destroy(iron_gpu_buffer_t *buffer) {

}

static int iron_gpu_internal_index_buffer_stride(iron_gpu_buffer_t *buffer) {
	return 4;
}

void *iron_gpu_index_buffer_lock_all(iron_gpu_buffer_t *buffer) {
	iron_gpu_index_buffer_lock(buffer, 0, iron_gpu_index_buffer_count(buffer));
}

void *iron_gpu_index_buffer_lock(iron_gpu_buffer_t *buffer, int start, int count) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = count * iron_gpu_internal_index_buffer_stride(buffer);
	bDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, start * iron_gpu_internal_index_buffer_stride(buffer), bDesc.size);
}

void iron_gpu_index_buffer_unlock_all(iron_gpu_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void iron_gpu_index_buffer_unlock(iron_gpu_buffer_t *buffer, int count) {
	iron_gpu_index_buffer_unlock_all(buffer);
}

void iron_gpu_internal_index_buffer_set(iron_gpu_buffer_t *buffer) {

}

int iron_gpu_index_buffer_count(iron_gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

void iron_gpu_texture_init(iron_gpu_texture_t *texture, int width, int height, iron_image_format_t format) {
	// WGPUExtent3D size = {};
	// size.width = iron_width();
	// size.height = iron_height();
	// size.depth = 1;

	// WGPUTextureDescriptor tDesc = {};
	// tDesc.sampleCount = 1;
	// tDesc.format = WGPUTextureFormat_BGRA8Unorm;
	// tDesc.usage = WGPUTextureUsage_OutputAttachment;
	// tDesc.size = size;
	// tDesc.dimension = WGPUTextureDimension_2D;
	// tDesc.mipLevelCount = 1;
	// tDesc.arrayLayerCount = 1;
	// WGPUTexture texture = wgpuDeviceCreateTexture(device, &tDesc);

	// WGPUTextureViewDescriptor tvDesc = {};
	// tvDesc.format = WGPUTextureFormat_BGRA8Unorm;
	// tvDesc.dimension = WGPUTextureViewDimension_2D;
 	// WGPUTextureView textureView = wgpuTextureCreateView(texture, &tvDesc);

	texture->_uploaded = true;
	texture->format = format;
	texture->data = NULL;
}

void iron_gpu_texture_init_from_bytes(iron_gpu_texture_t *texture, void *data, int width, int height, iron_image_format_t format) {}
void iron_gpu_texture_init_from_encoded_data(iron_gpu_texture_t *texture, void *data, int size, const char *format, bool readable) {}
void iron_gpu_texture_init_from_data(iron_gpu_texture_t *texture, void *data, int width, int height, int format, bool readable) {}
void iron_gpu_texture_destroy(iron_gpu_texture_t *texture) {}

int iron_gpu_texture_stride(iron_gpu_texture_t *texture) {
	return 32;
}

void iron_gpu_texture_generate_mipmaps(iron_gpu_texture_t *texture, int levels) {}

void iron_gpu_texture_set_mipmap(iron_gpu_texture_t *texture, iron_gpu_texture_t *mipmap, int level) {}

void iron_gpu_render_target_init(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits) {
    target->width = target->width = width;
	target->height = target->height = height;
	target->state = IRON_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->data = NULL;
	target->_uploaded = true;
}

void iron_gpu_render_target_init_framebuffer(iron_gpu_texture_t *target, int width, int height, iron_image_format_t format, int depthBufferBits) {}

void iron_gpu_render_target_set_depth_from(iron_gpu_texture_t *renderTarget, iron_gpu_texture_t *source) {}

void iron_gpu_render_target_get_pixels(iron_gpu_texture_t *renderTarget, uint8_t *data) {}

extern WGPUDevice device;

void iron_gpu_pipeline_init(iron_gpu_pipeline_t *pipe) {
	iron_gpu_internal_pipeline_init(pipe);
	iron_gpu_internal_pipeline_set_defaults(pipe);
}

iron_gpu_constant_location_t iron_gpu_pipeline_get_constant_location(iron_gpu_pipeline_t *pipe, const char* name) {
	iron_gpu_constant_location_t location;
	return location;
}

iron_gpu_texture_unit_t iron_gpu_pipeline_get_texture_unit(iron_gpu_pipeline_t *pipe, const char *name) {
	iron_gpu_texture_unit_t unit;
	return unit;
}

void iron_gpu_pipeline_compile(iron_gpu_pipeline_t *pipe) {
	WGPUColorTargetState csDesc;
	memset(&csDesc, 0, sizeof(csDesc));
	csDesc.format = WGPUTextureFormat_BGRA8Unorm;
	csDesc.writeMask = WGPUColorWriteMask_All;
	WGPUBlendState blend;
	memset(&blend, 0, sizeof(blend));
	blend.color.operation = WGPUBlendOperation_Add;
	blend.color.srcFactor = WGPUBlendFactor_One;
	blend.color.dstFactor = WGPUBlendFactor_Zero;
	blend.alpha.operation = WGPUBlendOperation_Add;
	blend.alpha.srcFactor = WGPUBlendFactor_One;
	blend.alpha.dstFactor = WGPUBlendFactor_Zero;
	csDesc.blend = &blend;

	WGPUPipelineLayoutDescriptor plDesc;
	memset(&plDesc, 0, sizeof(plDesc));
	plDesc.bindGroupLayoutCount = 0;
	plDesc.bindGroupLayouts = NULL;

	WGPUVertexAttribute vaDesc[8];
	memset(&vaDesc[0], 0, sizeof(vaDesc[0]) * 8);
	uint64_t offset = 0;
	for (int i = 0; i < pipe->input_layout->size; ++i) {
		vaDesc[i].shaderLocation = i;
		vaDesc[i].offset = offset;
		offset += iron_gpu_vertex_data_size(pipe->input_layout->elements[i].data);
		switch (pipe->input_layout->elements[i].data) {
		case IRON_GPU_VERTEX_DATA_F32_1X:
			vaDesc[i].format = WGPUVertexFormat_Float32;
			break;
		case IRON_GPU_VERTEX_DATA_F32_2X:
			vaDesc[i].format = WGPUVertexFormat_Float32x2;
			break;
		case IRON_GPU_VERTEX_DATA_F32_3X:
			vaDesc[i].format = WGPUVertexFormat_Float32x3;
			break;
		case IRON_GPU_VERTEX_DATA_F32_4X:
			vaDesc[i].format = WGPUVertexFormat_Float32x4;
			break;
		case IRON_GPU_VERTEX_DATA_U8_4X_NORM:
			vaDesc[i].format = WGPUVertexFormat_Unorm8x4;
			break;
		case IRON_GPU_VERTEX_DATA_I16_2X_NORM:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x2;
			break;
		case IRON_GPU_VERTEX_DATA_I16_4X_NORM:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x4;
			break;
		}
	}

	WGPUVertexBufferLayout vbDesc;
	memset(&vbDesc, 0, sizeof(vbDesc));
	vbDesc.arrayStride = offset;
	vbDesc.attributeCount = pipe->input_layout->size;
	vbDesc.attributes = &vaDesc[0];

	WGPUVertexState vsDest;
	memset(&vsDest, 0, sizeof(vsDest));

	vsDest.module = pipe->vertex_shader->impl.module;
	vsDest.entryPoint = "main";

	vsDest.bufferCount = 1;
	vsDest.buffers = &vbDesc;

	WGPUFragmentState fragmentDest;
	memset(&fragmentDest, 0, sizeof(fragmentDest));

	fragmentDest.module = pipe->fragment_shader->impl.module;
	fragmentDest.entryPoint = "main";

	fragmentDest.targetCount = 1;
	fragmentDest.targets = &csDesc;

	WGPUPrimitiveState rsDesc;
	memset(&rsDesc, 0, sizeof(rsDesc));
	rsDesc.topology = WGPUPrimitiveTopology_TriangleList;
	rsDesc.stripIndexFormat = WGPUIndexFormat_Uint32;
	rsDesc.frontFace = WGPUFrontFace_CW;
	rsDesc.cullMode = WGPUCullMode_None;

	WGPUMultisampleState multisample;
	memset(&multisample, 0, sizeof(multisample));
	multisample.count = 1;
	multisample.mask = 0xffffffff;
	multisample.alphaToCoverageEnabled = false;

	WGPURenderPipelineDescriptor rpDesc;
	memset(&rpDesc, 0, sizeof(rpDesc));
	rpDesc.layout = wgpuDeviceCreatePipelineLayout(device, &plDesc);
	rpDesc.fragment = &fragmentDest;
	rpDesc.vertex = vsDest;
	rpDesc.multisample = multisample;
	rpDesc.primitive = rsDesc;
	pipe->impl.pipeline = wgpuDeviceCreateRenderPipeline(device, &rpDesc);
}

void iron_gpu_sampler_init(iron_gpu_sampler_t *sampler, const iron_gpu_sampler_options_t *options) {}

void iron_gpu_sampler_destroy(iron_gpu_sampler_t *sampler) {}

void iron_gpu_shader_init(iron_gpu_shader_t *shader, const void *source, size_t length, iron_gpu_shader_type_t type) {
	WGPUShaderModuleSPIRVDescriptor smSpirvDesc;
	memset(&smSpirvDesc, 0, sizeof(smSpirvDesc));
	smSpirvDesc.chain.sType = WGPUSType_ShaderModuleSPIRVDescriptor;
	smSpirvDesc.codeSize = length / 4;
	smSpirvDesc.code = source;
	WGPUShaderModuleDescriptor smDesc;
	memset(&smDesc, 0, sizeof(smDesc));
	smDesc.nextInChain = &smSpirvDesc;
	shader->impl.module = wgpuDeviceCreateShaderModule(device, &smDesc);
}

void iron_gpu_shader_destroy(iron_gpu_shader_t *shader) {}





#include <string.h>
#include <iron_gpu.h>
#include <iron_system.h>

extern WGPUDevice device;
extern WGPUQueue queue;
extern WGPUSwapChain swapChain;

void iron_gpu_command_list_init(iron_gpu_command_list_t *list) {}

void iron_gpu_command_list_destroy(iron_gpu_command_list_t *list) {}

void iron_gpu_command_list_begin(iron_gpu_command_list_t *list) {
	WGPUCommandEncoderDescriptor ceDesc;
	memset(&ceDesc, 0, sizeof(ceDesc));
	list->impl.encoder = wgpuDeviceCreateCommandEncoder(device, &ceDesc);

	WGPURenderPassColorAttachment attachment;
	memset(&attachment, 0, sizeof(attachment));
	attachment.view = wgpuSwapChainGetCurrentTextureView(swapChain);;
	attachment.loadOp = WGPULoadOp_Clear;
	attachment.storeOp = WGPUStoreOp_Store;
	WGPUColor color = {0, 0, 0, 1};
	attachment.clearValue = color;

	WGPURenderPassDescriptor passDesc;
	memset(&passDesc, 0, sizeof(passDesc));
	passDesc.colorAttachmentCount = 1;
	passDesc.colorAttachments = &attachment;

	list->impl.pass = wgpuCommandEncoderBeginRenderPass(list->impl.encoder, &passDesc);
}

void iron_gpu_command_list_end(iron_gpu_command_list_t *list) {
	wgpuRenderPassEncoderEnd(list->impl.pass);

	WGPUCommandBufferDescriptor cbDesc;
	memset(&cbDesc, 0, sizeof(cbDesc));
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(list->impl.encoder, &cbDesc);
	wgpuQueueSubmit(queue, 1, &commands);
}

void iron_gpu_command_list_clear(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget, unsigned flags, unsigned color, float depth) {

}

void iron_gpu_command_list_render_target_to_framebuffer_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}
void iron_gpu_command_list_framebuffer_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}
void iron_gpu_command_list_texture_to_render_target_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}
void iron_gpu_command_list_render_target_to_texture_barrier(iron_gpu_command_list_t *list, struct iron_gpu_texture *renderTarget) {}

void iron_gpu_command_list_draw_indexed_vertices(iron_gpu_command_list_t *list) {
	wgpuRenderPassEncoderDrawIndexed(list->impl.pass, list->impl.indexCount, 1, 0, 0, 0);
}

void iron_gpu_command_list_draw_indexed_vertices_from_to(iron_gpu_command_list_t *list, int start, int count) {

}

void iron_gpu_command_list_viewport(iron_gpu_command_list_t *list, int x, int y, int width, int height) {

}

void iron_gpu_command_list_scissor(iron_gpu_command_list_t *list, int x, int y, int width, int height) {

}

void iron_gpu_command_list_disable_scissor(iron_gpu_command_list_t *list) {}

void iron_gpu_command_list_set_pipeline(iron_gpu_command_list_t *list, struct iron_gpu_pipeline *pipeline) {
	wgpuRenderPassEncoderSetPipeline(list->impl.pass, pipeline->impl.pipeline);
}

void iron_gpu_command_list_set_pipeline_layout(iron_gpu_command_list_t *list) {}

void iron_gpu_command_list_set_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer) {
	uint64_t size = (iron_gpu_vertex_buffer_count(buffer)) * iron_gpu_vertex_buffer_stride(buffer);
	wgpuRenderPassEncoderSetVertexBuffer(list->impl.pass, 0, buffer->impl.buffer, 0, size);
}

void iron_gpu_command_list_set_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer) {
	list->impl.indexCount = iron_gpu_index_buffer_count(buffer);
	uint64_t size = iron_gpu_index_buffer_count(buffer) * sizeof(int);
	wgpuRenderPassEncoderSetIndexBuffer(list->impl.pass, buffer->impl.buffer, WGPUIndexFormat_Uint32, 0, size);
}

void iron_gpu_command_list_set_render_targets(iron_gpu_command_list_t *list, struct iron_gpu_texture **targets, int count) {

}

void iron_gpu_command_list_upload_index_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer) {}
void iron_gpu_command_list_upload_vertex_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer) {}
void iron_gpu_command_list_upload_texture(iron_gpu_command_list_t *list, struct iron_gpu_texture *texture) {}
void iron_gpu_command_list_get_render_target_pixels(iron_gpu_command_list_t *list, iron_gpu_texture_t *render_target, uint8_t *data) {}

void iron_gpu_command_list_execute(iron_gpu_command_list_t *list) {

}

void iron_gpu_command_list_wait_for_execution_to_finish(iron_gpu_command_list_t *list) {

}

void iron_gpu_command_list_set_vertex_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer, int offset, size_t size) {

}

void iron_gpu_command_list_set_fragment_constant_buffer(iron_gpu_command_list_t *list, struct iron_gpu_buffer *buffer, int offset, size_t size) {

}

void iron_gpu_command_list_set_texture(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *texture) {

}

void iron_gpu_command_list_set_sampler(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_sampler_t *sampler) {}

void iron_gpu_command_list_set_texture_from_render_target_depth(iron_gpu_command_list_t *list, iron_gpu_texture_unit_t unit, iron_gpu_texture_t *renderTarget) {}
