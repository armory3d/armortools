#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/compute.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/pipeline.h>
#include <kinc/graphics5/vertexbuffer.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/window.h>

void createHeaps(kinc_g5_command_list_t *list);

static int formatSize(DXGI_FORMAT format) {
	switch (format) {
	case DXGI_FORMAT_R32G32B32A32_FLOAT:
		return 16;
	case DXGI_FORMAT_R16G16B16A16_FLOAT:
		return 8;
	case DXGI_FORMAT_R16_FLOAT:
		return 2;
	case DXGI_FORMAT_R8_UNORM:
		return 1;
	default:
		return 4;
	}
}

void kinc_g5_command_list_init(struct kinc_g5_command_list *list) {
#ifndef NDEBUG
	list->impl.open = false;
#endif

	device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, &IID_ID3D12CommandAllocator, &list->impl._commandAllocator);
	device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, list->impl._commandAllocator, NULL, &IID_ID3D12CommandList, &list->impl._commandList);

	list->impl.fence_value = 0;
	list->impl.fence_event = CreateEvent(NULL, FALSE, FALSE, NULL);
	device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, &IID_ID3D12Fence, &list->impl.fence);

	list->impl._indexCount = 0;

	list->impl.current_full_scissor.left = -1;

	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentRenderTargets[i] = NULL;
		list->impl.currentTextures[i] = NULL;
		list->impl.current_samplers[i] = NULL;
	}
	list->impl.heapIndex = 0;
	createHeaps(list);
}

void kinc_g5_command_list_destroy(struct kinc_g5_command_list *list) {}

void kinc_g5_internal_reset_textures(struct kinc_g5_command_list *list);

void kinc_g5_command_list_begin(struct kinc_g5_command_list *list) {
	assert(!list->impl.open);

	compute_pipeline_set = false;

	if (list->impl.fence_value > 0) {
		waitForFence(list->impl.fence, list->impl.fence_value, list->impl.fence_event);
		list->impl._commandAllocator->lpVtbl->Reset(list->impl._commandAllocator);
		list->impl._commandList->lpVtbl->Reset(list->impl._commandList, list->impl._commandAllocator, NULL);
	}

	kinc_g5_internal_reset_textures(list);

#ifndef NDEBUG
	list->impl.open = true;
#endif
}

void kinc_g5_command_list_end(struct kinc_g5_command_list *list) {
	assert(list->impl.open);

	list->impl._commandList->lpVtbl->Close(list->impl._commandList);

#ifndef NDEBUG
	list->impl.open = false;
#endif
}

void kinc_g5_command_list_clear(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget, unsigned flags, unsigned color, float depth) {
	assert(list->impl.open);

	if (flags & KINC_G5_CLEAR_COLOR) {
		float clearColor[] = {((color & 0x00ff0000) >> 16) / 255.0f, ((color & 0x0000ff00) >> 8) / 255.0f, (color & 0x000000ff) / 255.0f,
		                      ((color & 0xff000000) >> 24) / 255.0f};

		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		renderTarget->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(renderTarget->impl.renderTargetDescriptorHeap, &handle);

		list->impl._commandList->lpVtbl->ClearRenderTargetView(
		    list->impl._commandList,
		    handle,
		    clearColor,
		                                                       0,
		                                               NULL);
	}
	if (flags & KINC_G5_CLEAR_DEPTH) {
		D3D12_CLEAR_FLAGS d3dflags = D3D12_CLEAR_FLAG_DEPTH;

		if (renderTarget->impl.depthStencilDescriptorHeap != NULL) {
			D3D12_CPU_DESCRIPTOR_HANDLE handle;
			renderTarget->impl.depthStencilDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(renderTarget->impl.depthStencilDescriptorHeap, &handle);
			list->impl._commandList->lpVtbl->ClearDepthStencilView(
			    list->impl._commandList,
			    handle,
			                                                       d3dflags, depth,
			                                               0, 0, NULL);
		}
	}
}

void kinc_g5_command_list_render_target_to_framebuffer_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_framebuffer_to_render_target_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_texture_to_render_target_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_render_target_to_texture_barrier(struct kinc_g5_command_list *list, kinc_g5_render_target_t *renderTarget) {
	assert(list->impl.open);

	D3D12_RESOURCE_BARRIER barrier;
	barrier.Transition.pResource = renderTarget->impl.renderTarget;
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

	list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
}

void kinc_g5_command_list_set_vertex_constant_buffer(struct kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(list->impl._commandList, 2, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
}

void kinc_g5_command_list_set_fragment_constant_buffer(struct kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->SetGraphicsRootConstantBufferView(list->impl._commandList, 3, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
}

void kinc_g5_command_list_set_compute_constant_buffer(struct kinc_g5_command_list *list, kinc_g5_constant_buffer_t *buffer, int offset, size_t size) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->SetComputeRootConstantBufferView(list->impl._commandList, 3, buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.constant_buffer) + offset);
}

void kinc_g5_command_list_draw_indexed_vertices(struct kinc_g5_command_list *list) {
	kinc_g5_command_list_draw_indexed_vertices_from_to(list, 0, list->impl._indexCount);
}

void kinc_g5_command_list_draw_indexed_vertices_from_to(struct kinc_g5_command_list *list, int start, int count) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->IASetPrimitiveTopology(list->impl._commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	list->impl._commandList->lpVtbl->DrawIndexedInstanced(list->impl._commandList, count, 1, start, 0, 0);
}

void kinc_g5_command_list_execute(kinc_g5_command_list_t *list) {
	assert(!list->impl.open);

	ID3D12CommandList *commandLists[] = {(ID3D12CommandList *)list->impl._commandList};
	commandQueue->lpVtbl->ExecuteCommandLists(commandQueue, 1, commandLists);

	commandQueue->lpVtbl->Signal(commandQueue, list->impl.fence, ++list->impl.fence_value);
}

void kinc_g5_command_list_wait_for_execution_to_finish(kinc_g5_command_list_t *list) {
	waitForFence(list->impl.fence, list->impl.fence_value, list->impl.fence_event);
}

bool kinc_g5_non_pow2_textures_supported(void) {
	return true;
}

void kinc_g5_command_list_viewport(struct kinc_g5_command_list *list, int x, int y, int width, int height) {
	assert(list->impl.open);

	D3D12_VIEWPORT viewport;
	viewport.TopLeftX = (float)x;
	viewport.TopLeftY = (float)y;
	viewport.Width = (float)width;
	viewport.Height = (float)height;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, &viewport);
}

void kinc_g5_command_list_scissor(struct kinc_g5_command_list *list, int x, int y, int width, int height) {
	assert(list->impl.open);

	D3D12_RECT scissor;
	scissor.left = x;
	scissor.top = y;
	scissor.right = x + width;
	scissor.bottom = y + height;
	list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
}

void kinc_g5_command_list_disable_scissor(struct kinc_g5_command_list *list) {
	assert(list->impl.open);

	if (list->impl.current_full_scissor.left >= 0) {
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&list->impl.current_full_scissor);
	}
	else {
		D3D12_RECT scissor;
		scissor.left = 0;
		scissor.top = 0;
		scissor.right = kinc_window_width(0);
		scissor.bottom = kinc_window_height(0);
		list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, &scissor);
	}
}

void kinc_g5_command_list_set_pipeline(struct kinc_g5_command_list *list, kinc_g5_pipeline_t *pipeline) {
	assert(list->impl.open);

	list->impl._currentPipeline = pipeline;
	list->impl._commandList->lpVtbl->SetPipelineState(list->impl._commandList, pipeline->impl.pso);
	compute_pipeline_set = false;

	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentRenderTargets[i] = NULL;
		list->impl.currentTextures[i] = NULL;
	}
	kinc_g5_internal_setConstants(list, list->impl._currentPipeline);
}

void kinc_g5_command_list_set_vertex_buffer(struct kinc_g5_command_list *list, kinc_g5_vertex_buffer_t *buffer) {
	assert(list->impl.open);

	D3D12_VERTEX_BUFFER_VIEW *views = (D3D12_VERTEX_BUFFER_VIEW *)alloca(sizeof(D3D12_VERTEX_BUFFER_VIEW) * 1);
	ZeroMemory(views, sizeof(D3D12_VERTEX_BUFFER_VIEW) * 1);

	views[0].BufferLocation = buffer->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(buffer->impl.uploadBuffer);
	views[0].SizeInBytes = (kinc_g5_vertex_buffer_count(buffer)) * kinc_g5_vertex_buffer_stride(buffer);
	views[0].StrideInBytes = kinc_g5_vertex_buffer_stride(buffer);

	list->impl._commandList->lpVtbl->IASetVertexBuffers(list->impl._commandList, 0, 1, views);
}

void kinc_g5_command_list_set_index_buffer(struct kinc_g5_command_list *list, kinc_g5_index_buffer_t *buffer) {
	assert(list->impl.open);

	list->impl._indexCount = kinc_g5_index_buffer_count(buffer);
	list->impl._commandList->lpVtbl->IASetIndexBuffer(list->impl._commandList, (D3D12_INDEX_BUFFER_VIEW *) & buffer->impl.index_buffer_view);
}

void kinc_g5_command_list_set_render_targets(struct kinc_g5_command_list *list, kinc_g5_render_target_t **targets, int count) {
	assert(list->impl.open);

	kinc_g5_render_target_t *render_target = targets[0];

	D3D12_CPU_DESCRIPTOR_HANDLE target_descriptors[16];
	for (int i = 0; i < count; ++i) {
		targets[i]->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(targets[i]->impl.renderTargetDescriptorHeap,
		                                                                                        &target_descriptors[i]);
	}

	assert(render_target != NULL);

	if (render_target->impl.depthStencilDescriptorHeap != NULL) {
		D3D12_CPU_DESCRIPTOR_HANDLE heapStart;
		render_target->impl.depthStencilDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(render_target->impl.depthStencilDescriptorHeap, &heapStart);
		list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, count, &target_descriptors[0], false, &heapStart);
	}
	else {
		list->impl._commandList->lpVtbl->OMSetRenderTargets(list->impl._commandList, count, &target_descriptors[0], false, NULL);
	}

	list->impl._commandList->lpVtbl->RSSetViewports(list->impl._commandList, 1, (D3D12_VIEWPORT *)&render_target->impl.viewport);
	list->impl._commandList->lpVtbl->RSSetScissorRects(list->impl._commandList, 1, (D3D12_RECT *)&render_target->impl.scissor);

	list->impl.current_full_scissor = render_target->impl.scissor;
}

void kinc_g5_command_list_upload_vertex_buffer(kinc_g5_command_list_t *list, struct kinc_g5_vertex_buffer *buffer) {}

void kinc_g5_command_list_upload_index_buffer(kinc_g5_command_list_t *list, kinc_g5_index_buffer_t *buffer) {
	assert(list->impl.open);
	kinc_g5_internal_index_buffer_upload(buffer, list->impl._commandList);
}

void kinc_g5_command_list_upload_texture(kinc_g5_command_list_t *list, kinc_g5_texture_t *texture) {
	assert(list->impl.open);

	{
		D3D12_RESOURCE_BARRIER transition = {};
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Transition.pResource = texture->impl.image;
		transition.Transition.StateBefore = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		transition.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
		transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &transition);
	}

	D3D12_RESOURCE_DESC Desc;
	texture->impl.image->lpVtbl->GetDesc(texture->impl.image, &Desc);
	ID3D12Device *device = NULL;
	texture->impl.image->lpVtbl->GetDevice(texture->impl.image, &IID_ID3D12Device, &device);
	D3D12_PLACED_SUBRESOURCE_FOOTPRINT footprint;
	device->lpVtbl->GetCopyableFootprints(device , &Desc, 0, 1, 0, &footprint, NULL, NULL, NULL);
	device->lpVtbl->Release(device);

	D3D12_TEXTURE_COPY_LOCATION source = {0};
	source.pResource = texture->impl.uploadImage;
	source.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	source.PlacedFootprint = footprint;

	D3D12_TEXTURE_COPY_LOCATION destination = {0};
	destination.pResource = texture->impl.image;
	destination.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	destination.SubresourceIndex = 0;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList, &destination, 0, 0, 0, &source, NULL);

	{
		D3D12_RESOURCE_BARRIER transition = {};
		transition.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		transition.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		transition.Transition.pResource = texture->impl.image;
		transition.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		transition.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;
		transition.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;

		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &transition);
	}
}

static int d3d12_textureAlignment() {
	return D3D12_TEXTURE_DATA_PITCH_ALIGNMENT;
}

void kinc_g5_command_list_get_render_target_pixels(kinc_g5_command_list_t *list, kinc_g5_render_target_t *render_target, uint8_t *data) {
	assert(list->impl.open);

	D3D12_RESOURCE_DESC desc;
	render_target->impl.renderTarget->lpVtbl->GetDesc(render_target->impl.renderTarget, &desc);
	DXGI_FORMAT dxgiFormat = desc.Format;
	int formatByteSize = formatSize(dxgiFormat);
	int rowPitch = render_target->texWidth * formatByteSize;
	int align = rowPitch % d3d12_textureAlignment();
	if (align != 0)
		rowPitch = rowPitch + (d3d12_textureAlignment() - align);

	// Create readback buffer
	if (render_target->impl.renderTargetReadback == NULL) {
		D3D12_HEAP_PROPERTIES heapProperties;
		heapProperties.Type = D3D12_HEAP_TYPE_READBACK;
		heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;

		D3D12_RESOURCE_DESC resourceDesc;
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Alignment = 0;
		resourceDesc.Width = rowPitch * render_target->texHeight;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.SampleDesc.Quality = 0;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		device->lpVtbl->CreateCommittedResource(device , &heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc, D3D12_RESOURCE_STATE_COPY_DEST, NULL,
		                                &IID_ID3D12Resource, &render_target->impl.renderTargetReadback);
	}

	// Copy render target to readback buffer
	D3D12_RESOURCE_STATES sourceState = D3D12_RESOURCE_STATE_RENDER_TARGET;

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = sourceState;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	D3D12_TEXTURE_COPY_LOCATION source;
	source.pResource = render_target->impl.renderTarget;
	source.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
	source.SubresourceIndex = 0;

	D3D12_TEXTURE_COPY_LOCATION dest;
	dest.pResource = render_target->impl.renderTargetReadback;
	dest.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
	dest.PlacedFootprint.Offset = 0;
	dest.PlacedFootprint.Footprint.Format = dxgiFormat;
	dest.PlacedFootprint.Footprint.Width = render_target->texWidth;
	dest.PlacedFootprint.Footprint.Height = render_target->texHeight;
	dest.PlacedFootprint.Footprint.Depth = 1;
	dest.PlacedFootprint.Footprint.RowPitch = rowPitch;

	list->impl._commandList->lpVtbl->CopyTextureRegion(list->impl._commandList , &dest, 0, 0, 0, &source, NULL);

	{
		D3D12_RESOURCE_BARRIER barrier;
		barrier.Transition.pResource = render_target->impl.renderTarget;
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
		barrier.Transition.StateAfter = sourceState;
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		list->impl._commandList->lpVtbl->ResourceBarrier(list->impl._commandList, 1, &barrier);
	}

	kinc_g5_command_list_end(list);
	kinc_g5_command_list_execute(list);
	kinc_g5_command_list_wait_for_execution_to_finish(list);
	kinc_g5_command_list_begin(list);

	// Read buffer
	void *p;
	render_target->impl.renderTargetReadback->lpVtbl->Map(render_target->impl.renderTargetReadback, 0, NULL, &p);
	memcpy(data, p, render_target->texWidth * render_target->texHeight * formatByteSize);
	render_target->impl.renderTargetReadback->lpVtbl->Unmap(render_target->impl.renderTargetReadback, 0, NULL);
}

void kinc_g5_internal_set_compute_constants(kinc_g5_command_list_t *commandList);

void kinc_g5_command_list_set_compute_shader(kinc_g5_command_list_t *list, kinc_g5_compute_shader *shader) {
	list->impl._commandList->lpVtbl->SetPipelineState(list->impl._commandList, shader->impl.pso);
	compute_pipeline_set = true;

	for (int i = 0; i < KINC_INTERNAL_G5_TEXTURE_COUNT; ++i) {
		list->impl.currentRenderTargets[i] = NULL;
		list->impl.currentTextures[i] = NULL;
	}
	kinc_g5_internal_set_compute_constants(list);
}

void kinc_g5_command_list_compute(kinc_g5_command_list_t *list, int x, int y, int z) {
	assert(list->impl.open);
	list->impl._commandList->lpVtbl->Dispatch(list->impl._commandList, x, y, z);
}

void kinc_g5_command_list_set_texture(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_texture_t *texture) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		kinc_g5_internal_texture_set(list, texture, unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]);
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		kinc_g5_internal_texture_set(list, texture, unit.stages[KINC_G5_SHADER_TYPE_VERTEX]);
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] >= 0) {
		kinc_g5_internal_texture_set(list, texture, unit.stages[KINC_G5_SHADER_TYPE_COMPUTE]);
	}
	kinc_g5_internal_set_textures(list);
}

void kinc_g5_internal_sampler_set(kinc_g5_command_list_t *list, kinc_g5_sampler_t *sampler, int unit);

void kinc_g5_command_list_set_sampler(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_sampler_t *sampler) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		kinc_g5_internal_sampler_set(list, sampler, unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT]);
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		kinc_g5_internal_sampler_set(list, sampler, unit.stages[KINC_G5_SHADER_TYPE_VERTEX]);
	}
	kinc_g5_internal_set_textures(list);
}

void kinc_g5_command_list_set_texture_from_render_target(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		list->impl.currentRenderTargets[target->impl.stage] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		list->impl.currentRenderTargets[target->impl.stage] = target;
	}
	list->impl.currentTextures[target->impl.stage] = NULL;

	kinc_g5_internal_set_textures(list);
}

void kinc_g5_command_list_set_texture_from_render_target_depth(kinc_g5_command_list_t *list, kinc_g5_texture_unit_t unit, kinc_g5_render_target_t *target) {
	if (unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_FRAGMENT];
		list->impl.currentRenderTargets[target->impl.stage_depth] = target;
	}
	else if (unit.stages[KINC_G5_SHADER_TYPE_VERTEX] >= 0) {
		target->impl.stage_depth = unit.stages[KINC_G5_SHADER_TYPE_VERTEX];
		list->impl.currentRenderTargets[target->impl.stage_depth] = target;
	}
	list->impl.currentTextures[target->impl.stage_depth] = NULL;

	kinc_g5_internal_set_textures(list);
}
