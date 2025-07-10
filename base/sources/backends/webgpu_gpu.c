#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <webgpu/webgpu.h>
#include <iron_gpu.h>
#include <iron_math.h>
#include <iron_system.h>

bool gpu_transpose_mat = false;
int renderTargetWidth;
int renderTargetHeight;
int newRenderTargetWidth;
int newRenderTargetHeight;

WGPUDevice device;
WGPUQueue queue;
WGPUSwapChain swapChain;
WGPUCommandEncoder encoder;
WGPURenderPassEncoder pass;
int indexCount;
gpu_buffer_t *gpu_internal_current_vertex_buffer = NULL;
gpu_buffer_t *gpu_internal_current_index_buffer = NULL;

void gpu_destroy() {}

void gpu_init_internal(int depth_bits, bool vsync) {
	newRenderTargetWidth = renderTargetWidth = iron_window_width();
	newRenderTargetHeight = renderTargetHeight = iron_window_height();

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
	scDesc.width = iron_window_width();
	scDesc.height = iron_window_height();
	scDesc.presentMode = WGPUPresentMode_Fifo;
	swapChain = wgpuDeviceCreateSwapChain(device, surface, &scDesc);
}

void gpu_begin_internal(struct gpu_texture **targets, int count, gpu_texture_t *depth_buffer, unsigned flags, unsigned color, float depth) {
	WGPUCommandEncoderDescriptor ceDesc;
	memset(&ceDesc, 0, sizeof(ceDesc));
	encoder = wgpuDeviceCreateCommandEncoder(device, &ceDesc);

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

	pass = wgpuCommandEncoderBeginRenderPass(encoder, &passDesc);
}
void gpu_end_internal() {
	wgpuRenderPassEncoderEnd(pass);

	WGPUCommandBufferDescriptor cbDesc;
	memset(&cbDesc, 0, sizeof(cbDesc));
	WGPUCommandBuffer commands = wgpuCommandEncoderFinish(encoder, &cbDesc);
	wgpuQueueSubmit(queue, 1, &commands);
}

void gpu_present() {

}

bool gpu_raytrace_supported() {
	return false;
}

void gpu_vertex_buffer_init(gpu_buffer_t *buffer, int count, gpu_vertex_structure_t *structure) {
	buffer->count = count;
	buffer->impl.count = count;
	buffer->impl.stride = 0;
	for (int i = 0; i < structure->size; ++i) {
		buffer->impl.stride += gpu_vertex_data_size(structure->elements[i].data);
	}
}

float *gpu_vertex_buffer_lock(gpu_buffer_t *buffer) {
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = buffer->impl.count * buffer->impl.stride * sizeof(float);
	bDesc.usage = WGPUBufferUsage_Vertex | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, 0, bDesc.size);
}

void gpu_vertex_buffer_unlock(gpu_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

int gpu_vertex_buffer_count(gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

int gpu_vertex_buffer_stride(gpu_buffer_t *buffer) {
	return buffer->impl.stride;
}

void gpu_constant_buffer_init(gpu_buffer_t *buffer, int size) {}
void gpu_constant_buffer_destroy(gpu_buffer_t *buffer) {}
void gpu_constant_buffer_lock(gpu_buffer_t *buffer, int start, int count) {}
void gpu_constant_buffer_unlock(gpu_buffer_t *buffer) {}

int gpu_constant_buffer_size(gpu_buffer_t *buffer) {
	return 0;
}

void gpu_index_buffer_init(gpu_buffer_t *buffer, int count) {
	buffer->impl.count = count;
}

void gpu_buffer_destroy(gpu_buffer_t *buffer) {}

static int gpu_internal_index_buffer_stride(gpu_buffer_t *buffer) {
	return 4;
}

void *gpu_index_buffer_lock(gpu_buffer_t *buffer) {
	int start = 0;
	int count = gpu_index_buffer_count(buffer);
	WGPUBufferDescriptor bDesc;
	memset(&bDesc, 0, sizeof(bDesc));
	bDesc.size = count * gpu_internal_index_buffer_stride(buffer);
	bDesc.usage = WGPUBufferUsage_Index | WGPUBufferUsage_CopyDst;
	bDesc.mappedAtCreation = true;
	buffer->impl.buffer = wgpuDeviceCreateBuffer(device, &bDesc);
	return wgpuBufferGetMappedRange(buffer->impl.buffer, start * gpu_internal_index_buffer_stride(buffer), bDesc.size);
}

void gpu_index_buffer_unlock(gpu_buffer_t *buffer) {
	wgpuBufferUnmap(buffer->impl.buffer);
}

void gpu_internal_index_buffer_set(gpu_buffer_t *buffer) {}

int gpu_index_buffer_count(gpu_buffer_t *buffer) {
	return buffer->impl.count;
}

void gpu_texture_init_from_bytes(gpu_texture_t *texture, void *data, int width, int height, gpu_texture_format_t format) {}
void gpu_texture_destroy(gpu_texture_t *texture) {}

void gpu_render_target_init(gpu_texture_t *target, int width, int height, gpu_texture_format_t format) {
    target->width = target->width = width;
	target->height = target->height = height;
	target->state = GPU_TEXTURE_STATE_RENDER_TARGET;
	target->data = NULL;
}

void gpu_render_target_init_framebuffer(gpu_texture_t *target, int width, int height, gpu_texture_format_t format) {}

void gpu_pipeline_init(gpu_pipeline_t *pipe) {
	gpu_internal_pipeline_init(pipe);
}

void gpu_pipeline_compile(gpu_pipeline_t *pipe) {
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
		offset += gpu_vertex_data_size(pipe->input_layout->elements[i].data);
		switch (pipe->input_layout->elements[i].data) {
		case GPU_VERTEX_DATA_F32_1X:
			vaDesc[i].format = WGPUVertexFormat_Float32;
			break;
		case GPU_VERTEX_DATA_F32_2X:
			vaDesc[i].format = WGPUVertexFormat_Float32x2;
			break;
		case GPU_VERTEX_DATA_F32_3X:
			vaDesc[i].format = WGPUVertexFormat_Float32x3;
			break;
		case GPU_VERTEX_DATA_F32_4X:
			vaDesc[i].format = WGPUVertexFormat_Float32x4;
			break;
		case GPU_VERTEX_DATA_I16_2X_NORM:
			vaDesc[i].format = WGPUVertexFormat_Snorm16x2;
			break;
		case GPU_VERTEX_DATA_I16_4X_NORM:
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

void gpu_shader_init(gpu_shader_t *shader, const void *source, size_t length, gpu_shader_type_t type) {
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

void gpu_shader_destroy(gpu_shader_t *shader) {}
void gpu_destroy() {}

void gpu_barrier(gpu_texture_t *renderTarget, int state_after) {}

void gpu_draw_internal() {
	wgpuRenderPassEncoderDrawIndexed(pass, indexCount, 1, 0, 0, 0);
}

void gpu_viewport(int x, int y, int width, int height) {}
void gpu_scissor(int x, int y, int width, int height) {}
void gpu_disable_scissor() {}

void gpu_set_pipeline(struct gpu_pipeline *pipeline) {
	wgpuRenderPassEncoderSetPipeline(pass, pipeline->impl.pipeline);
}

void gpu_set_pipeline_layout() {}

void gpu_set_vertex_buffer(struct gpu_buffer *buffer) {
	uint64_t size = (gpu_vertex_buffer_count(buffer)) * gpu_vertex_buffer_stride(buffer);
	wgpuRenderPassEncoderSetVertexBuffer(pass, 0, buffer->impl.buffer, 0, size);
}

void gpu_set_index_buffer(struct gpu_buffer *buffer) {
	indexCount = gpu_index_buffer_count(buffer);
	uint64_t size = gpu_index_buffer_count(buffer) * sizeof(int);
	wgpuRenderPassEncoderSetIndexBuffer(pass, buffer->impl.buffer, WGPUIndexFormat_Uint32, 0, size);
}

void gpu_get_render_target_pixels(gpu_texture_t *render_target, uint8_t *data) {}
void gpu_wait() {}
void gpu_set_constant_buffer(struct gpu_buffer *buffer, int offset, size_t size) {}
void gpu_set_texture(int unit, gpu_texture_t *texture) {}
