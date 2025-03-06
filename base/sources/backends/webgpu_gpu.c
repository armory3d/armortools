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

void kinc_g5_internal_destroy_window() {}

void kinc_g5_internal_destroy() {}

void kinc_g5_internal_init() {}

void kinc_g5_internal_init_window(int depthBufferBits, bool vsync) {
	newRenderTargetWidth = renderTargetWidth = kinc_width();
	newRenderTargetHeight = renderTargetHeight = kinc_height();

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
	scDesc.width = kinc_width();
	scDesc.height = kinc_height();
	scDesc.presentMode = WGPUPresentMode_Fifo;
	swapChain = wgpuDeviceCreateSwapChain(device, surface, &scDesc);
}

void kinc_g5_begin(kinc_g5_texture_t *renderTarget) {}

void kinc_g5_end() {}

bool kinc_g5_swap_buffers() {
	return true;
}

void kinc_g5_flush() {}

bool kinc_g5_raytrace_supported() {
	return false;
}

extern WGPUDevice device;

kinc_g5_vertex_buffer_t *kinc_g5_internal_current_vertex_buffer = NULL;

void kinc_g5_vertex_buffer_init(kinc_g5_vertex_buffer_t *buffer, int count, kinc_g5_vertex_structure_t *structure, bool gpuMemory) {
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += kinc_g5_vertex_data_size(structure->elements[i].data);
	}
}

void kinc_g5_vertex_buffer_destroy(kinc_g5_vertex_buffer_t *buffer) {

}

float *kinc_g5_vertex_buffer_lock_all(kinc_g5_vertex_buffer_t *buffer) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = buffer->impl.count * buffer->impl.stride * sizeof(float);
	bDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, 0, bDesc.size);
}

float *kinc_g5_vertex_buffer_lock(kinc_g5_vertex_buffer_t *buffer, int start, int count) {
	return NULL;
}

void kinc_g5_vertex_buffer_unlock_all(kinc_g5_vertex_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void kinc_g5_vertex_buffer_unlock(kinc_g5_vertex_buffer_t* buffer, int count) {

}

int kinc_g5_vertex_buffer_count(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.count;
}

int kinc_g5_vertex_buffer_stride(kinc_g5_vertex_buffer_t *buffer) {
	return buffer->impl.stride;
}

bool kinc_g5_transpose_mat = false;

void kinc_g5_constant_buffer_init(kinc_g5_constant_buffer_t *buffer, int size) {

}

void kinc_g5_constant_buffer_destroy(kinc_g5_constant_buffer_t *buffer) {}

void kinc_g5_constant_buffer_lock_all(kinc_g5_constant_buffer_t *buffer) {
	kinc_g5_constant_buffer_lock(buffer, 0, kinc_g5_constant_buffer_size(buffer));
}

void kinc_g5_constant_buffer_lock(kinc_g5_constant_buffer_t *buffer, int start, int count) {}

void kinc_g5_constant_buffer_unlock(kinc_g5_constant_buffer_t *buffer) {

}

int kinc_g5_constant_buffer_size(kinc_g5_constant_buffer_t *buffer) {
	return 0;
}

kinc_g5_index_buffer_t *kinc_g5_internal_current_index_buffer = NULL;

void kinc_g5_index_buffer_init(kinc_g5_index_buffer_t *buffer, int count, bool gpuMemory) {
	buffer->impl.count = count;
}

void kinc_g5_index_buffer_destroy(kinc_g5_index_buffer_t *buffer) {

}

static int kinc_g5_internal_index_buffer_stride(kinc_g5_index_buffer_t *buffer) {
	return 4;
}

void *kinc_g5_index_buffer_lock_all(kinc_g5_index_buffer_t *buffer) {
	kinc_g5_index_buffer_lock(buffer, 0, kinc_g5_index_buffer_count(buffer));
}

void *kinc_g5_index_buffer_lock(kinc_g5_index_buffer_t *buffer, int start, int count) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = count * kinc_g5_internal_index_buffer_stride(buffer);
	bDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, start * kinc_g5_internal_index_buffer_stride(buffer), bDesc.size);
}

void kinc_g5_index_buffer_unlock_all(kinc_g5_index_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void kinc_g5_index_buffer_unlock(kinc_g5_index_buffer_t *buffer, int count) {
	kinc_g5_index_buffer_unlock_all(buffer);
}

void kinc_g5_internal_index_buffer_set(kinc_g5_index_buffer_t *buffer) {

}

int kinc_g5_index_buffer_count(kinc_g5_index_buffer_t *buffer) {
	return buffer->impl.count;
}

void kinc_g5_texture_init(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {
	// WGPUExtent3D size = {};
	// size.width = kinc_width();
	// size.height = kinc_height();
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

void kinc_g5_texture_init_from_bytes(kinc_g5_texture_t *texture, void *data, int width, int height, kinc_image_format_t format) {}
void kinc_g5_texture_init_from_encoded_data(kinc_g5_texture_t *texture, void *data, int size, const char *format, bool readable) {}
void kinc_g5_texture_init_from_data(kinc_g5_texture_t *texture, void *data, int width, int height, int format, bool readable) {}
void kinc_g5_texture_init_non_sampled_access(kinc_g5_texture_t *texture, int width, int height, kinc_image_format_t format) {}
void kinc_g5_texture_destroy(kinc_g5_texture_t *texture) {}

int kinc_g5_texture_stride(kinc_g5_texture_t *texture) {
	return 32;
}

void kinc_g5_texture_generate_mipmaps(kinc_g5_texture_t *texture, int levels) {}

void kinc_g5_texture_set_mipmap(kinc_g5_texture_t *texture, kinc_g5_texture_t *mipmap, int level) {}

void kinc_g5_render_target_init(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {
    target->width = target->width = width;
	target->height = target->height = height;
	target->state = KINC_INTERNAL_RENDER_TARGET_STATE_RENDER_TARGET;
	target->data = NULL;
}

void kinc_g5_render_target_init_framebuffer(kinc_g5_texture_t *target, int width, int height, kinc_image_format_t format, int depthBufferBits) {}

void kinc_g5_render_target_set_depth_from(kinc_g5_texture_t *renderTarget, kinc_g5_texture_t *source) {}

void kinc_g5_render_target_get_pixels(kinc_g5_texture_t *renderTarget, uint8_t *data) {}

extern WGPUDevice device;

void kinc_g5_pipeline_init(kinc_g5_pipeline_t *pipe) {
	kinc_g5_internal_pipeline_init(pipe);
	kinc_g5_internal_pipeline_set_defaults(pipe);
}

kinc_g5_constant_location_t kinc_g5_pipeline_get_constant_location(kinc_g5_pipeline_t *pipe, const char* name) {
	kinc_g5_constant_location_t location;
	return location;
}

kinc_g5_texture_unit_t kinc_g5_pipeline_get_texture_unit(kinc_g5_pipeline_t *pipe, const char *name) {
	kinc_g5_texture_unit_t unit;
	return unit;
}

void kinc_g5_pipeline_compile(kinc_g5_pipeline_t *pipe) {
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
		offset += kinc_g5_vertex_data_size(pipe->input_layout->elements[i].data);
		switch (pipe->input_layout->elements[i].data) {
		case KINC_G5_VERTEX_DATA_F32_1X:
			vaDesc[i].format = WGPUVertexFormat_Float32;
			break;
		case KINC_G5_VERTEX_DATA_F32_2X:
			vaDesc[i].format = WGPUVertexFormat_Float32x2;
			break;
		case KINC_G5_VERTEX_DATA_F32_3X:
			vaDesc[i].format = WGPUVertexFormat_Float32x3;
			break;
		case KINC_G5_VERTEX_DATA_F32_4X:
			vaDesc[i].format = WGPUVertexFormat_Float32x4;
			break;
		case KINC_G5_VERTEX_DATA_U8_4X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Unorm8x4;
			break;
		case KINC_G5_VERTEX_DATA_I16_2X_NORMALIZED:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x2;
			break;
		case KINC_G5_VERTEX_DATA_I16_4X_NORMALIZED:
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

void kinc_g5_sampler_init(kinc_g5_sampler_t *sampler, const kinc_g5_sampler_options_t *options) {}

void kinc_g5_sampler_destroy(kinc_g5_sampler_t *sampler) {}

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
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

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {}





#include <string.h>
#include <iron_gpu.h>
#include <iron_system.h>

extern WGPUDevice device;
extern WGPUQueue queue;
extern WGPUSwapChain swapChain;

void kinc_g5_command_list_init(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_destroy(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_begin(kinc_g5_command_list_t *list) {
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

void kinc_g5_command_list_end(kinc_g5_command_list_t *list) {
	wgpuRenderPassEncoderEnd(list->impl.pass);

	WGPUCommandBufferDescriptor cbDesc;
	memset(&cbDesc, 0, sizeof(cbDesc));
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(list->impl.encoder, &cbDesc);
	wgpuQueueSubmit(queue, 1, &commands);
}

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget, unsigned flags, unsigned color, float depth) {

}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_texture *renderTarget) {}

void kinc_g5_command_list_draw_indexed_vertices(kinc_g5_command_list_t *list) {
	wgpuRenderPassEncoderDrawIndexed(list->impl.pass, list->impl.indexCount, 1, 0, 0, 0);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(kinc_g5_command_list_t *list, int start, int count) {

}

void kinc_g5_command_list_viewport(kinc_g5_command_list_t *list, int x, int y, int width, int height) {

}

void kinc_g5_command_list_scissor(kinc_g5_command_list_t *list, int x, int y, int width, int height) {

}

void kinc_g5_command_list_disable_scissor(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_pipeline(kinc_g5_command_list_t *list, struct kinc_g5_pipeline *pipeline) {
	wgpuRenderPassEncoderSetPipeline(list->impl.pass, pipeline->impl.pipeline);
}

void kinc_g5_command_list_set_pipeline_layout(kinc_g5_command_list_t *list) {}

void kinc_g5_command_list_set_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {
	uint64_t size = (kinc_g5_vertex_buffer_count(buffer)) * kinc_g5_vertex_buffer_stride(buffer);
	wgpuRenderPassEncoderSetVertexBuffer(list->impl.pass, 0, buffer->impl.buffer, 0, size);
}

void kinc_g5_command_list_set_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {
	list->impl.indexCount = kinc_g5_index_buffer_count(buffer);
	uint64_t size = kinc_g5_index_buffer_count(buffer) * sizeof(int);
	wgpuRenderPassEncoderSetIndexBuffer(list->impl.pass, buffer->impl.buffer, WGPUIndexFormat_Uint32, 0, size);
}

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_texture **targets, int count) {

}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}
void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_texture_t *render_target, uint8_t *data) {}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {

}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {

}

void kinc_g5_command_list_set_vertex_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {

}

void kinc_g5_command_list_set_fragment_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {

}

void kinc_g5_command_list_set_compute_constant_buffer(kinc_g5_command_list_t *list, struct kinc_g5_constant_buffer *buffer, int offset, size_t size) {

}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {

}

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *renderTarget) {}

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, kinc_g5_compute_shader *shader) {}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {}

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *source, int length) {}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location = {0};
	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_texture_unit_t unit = {0};
	return unit;
}
