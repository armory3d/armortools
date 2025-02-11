#include <string.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/compute.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/system.h>

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

void kinc_g5_command_list_clear(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget, unsigned flags, unsigned color, float depth) {

}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_framebuffer_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_texture_to_render_target_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}
void kinc_g5_command_list_render_target_to_texture_barrier(kinc_g5_command_list_t *list, struct kinc_g5_render_target *renderTarget) {}

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

void kinc_g5_command_list_set_render_targets(kinc_g5_command_list_t *list, struct kinc_g5_render_target **targets, int count) {

}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, struct kinc_g5_index_buffer *buffer) {}
void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}
void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, struct kinc_g5_texture *texture) {}
void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {}

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

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *renderTarget) {}

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, kinc_g5_compute_shader *shader) {}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {}
