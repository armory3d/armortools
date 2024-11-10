#ifndef KINC_XBOX_ONE

#include <kinc/backend/graphics5/raytrace.h>

#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/raytrace.h>
#include <kinc/graphics5/vertexbuffer.h>

static const wchar_t *hit_group_name = L"hitgroup";
static const wchar_t *raygen_shader_name = L"raygeneration";
static const wchar_t *closesthit_shader_name = L"closesthit";
static const wchar_t *miss_shader_name = L"miss";

static ID3D12Device5 *dxrDevice = NULL;
static ID3D12GraphicsCommandList4 *dxrCommandList = NULL;
static ID3D12RootSignature *dxrRootSignature = NULL;
static ID3D12DescriptorHeap *descriptorHeap = NULL;
static kinc_raytrace_acceleration_structure_t *accel;
static kinc_raytrace_pipeline_t *pipeline;
static kinc_g5_render_target_t *output = NULL;
static D3D12_CPU_DESCRIPTOR_HANDLE outputCpuDescriptor;
static D3D12_GPU_DESCRIPTOR_HANDLE outputDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE vbgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE ibgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex0gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex1gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE tex2gpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texenvgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texsobolgpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texscramblegpuDescriptorHandle;
static D3D12_GPU_DESCRIPTOR_HANDLE texrankgpuDescriptorHandle;
static int descriptorsAllocated = 0;
static UINT descriptorSize;

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 kinc_g5_constant_buffer_t *constant_buffer) {
	output = NULL;
	descriptorsAllocated = 0;
	pipeline->_constant_buffer = constant_buffer;
	// Descriptor heap
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
	// Allocate a heap for 3 descriptors:
	// 2 - bottom and top level acceleration structure
	// 1 - raytracing output texture SRV
	descriptorHeapDesc.NumDescriptors = 12;
	descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	descriptorHeapDesc.NodeMask = 0;
	if (descriptorHeap != NULL) descriptorHeap->Release();
	device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&descriptorHeap));
	descriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Device
	if (dxrDevice != NULL) dxrDevice->Release();
	device->QueryInterface(IID_GRAPHICS_PPV_ARGS(&dxrDevice));
	if (dxrCommandList != NULL) dxrCommandList->Release();
	command_list->impl._commandList->QueryInterface(IID_GRAPHICS_PPV_ARGS(&dxrCommandList));

	// Root signatures
	// This is a root signature that is shared across all raytracing shaders invoked during a DispatchRays() call.
	D3D12_DESCRIPTOR_RANGE UAVDescriptor = {};
	UAVDescriptor.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_UAV;
	UAVDescriptor.NumDescriptors = 1;
	UAVDescriptor.BaseShaderRegister = 0;
	UAVDescriptor.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorA = {};
	SRVDescriptorA.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorA.NumDescriptors = 1;
	SRVDescriptorA.BaseShaderRegister = 1;
	SRVDescriptorA.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorB = {};
	SRVDescriptorB.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorB.NumDescriptors = 1;
	SRVDescriptorB.BaseShaderRegister = 2;
	SRVDescriptorB.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor0 = {};
	SRVDescriptor0.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor0.NumDescriptors = 1;
	SRVDescriptor0.BaseShaderRegister = 3;
	SRVDescriptor0.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor1 = {};
	SRVDescriptor1.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor1.NumDescriptors = 1;
	SRVDescriptor1.BaseShaderRegister = 4;
	SRVDescriptor1.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptor2 = {};
	SRVDescriptor2.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptor2.NumDescriptors = 1;
	SRVDescriptor2.BaseShaderRegister = 5;
	SRVDescriptor2.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorEnv = {};
	SRVDescriptorEnv.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorEnv.NumDescriptors = 1;
	SRVDescriptorEnv.BaseShaderRegister = 6;
	SRVDescriptorEnv.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorSobol = {};
	SRVDescriptorSobol.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorSobol.NumDescriptors = 1;
	SRVDescriptorSobol.BaseShaderRegister = 7;
	SRVDescriptorSobol.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorScramble = {};
	SRVDescriptorScramble.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorScramble.NumDescriptors = 1;
	SRVDescriptorScramble.BaseShaderRegister = 8;
	SRVDescriptorScramble.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_DESCRIPTOR_RANGE SRVDescriptorRank = {};
	SRVDescriptorRank.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
	SRVDescriptorRank.NumDescriptors = 1;
	SRVDescriptorRank.BaseShaderRegister = 9;
	SRVDescriptorRank.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

	D3D12_ROOT_PARAMETER rootParameters[12] = {};
	// Output view
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[0].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[0].DescriptorTable.pDescriptorRanges = &UAVDescriptor;
	// Acceleration structure
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_SRV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	// Constant buffer
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[2].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[2].DescriptorTable.pDescriptorRanges = &SRVDescriptorA;
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[3].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[3].DescriptorTable.pDescriptorRanges = &SRVDescriptorB;
	rootParameters[4].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[4].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[4].Descriptor.ShaderRegister = 0;
	rootParameters[5].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[5].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[5].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[5].DescriptorTable.pDescriptorRanges = &SRVDescriptor0;
	rootParameters[6].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[6].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[6].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[6].DescriptorTable.pDescriptorRanges = &SRVDescriptor1;
	rootParameters[7].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[7].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[7].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[7].DescriptorTable.pDescriptorRanges = &SRVDescriptor2;
	rootParameters[8].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[8].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[8].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[8].DescriptorTable.pDescriptorRanges = &SRVDescriptorEnv;
	rootParameters[9].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[9].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[9].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[9].DescriptorTable.pDescriptorRanges = &SRVDescriptorSobol;
	rootParameters[10].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[10].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[10].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[10].DescriptorTable.pDescriptorRanges = &SRVDescriptorScramble;
	rootParameters[11].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[11].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;
	rootParameters[11].DescriptorTable.NumDescriptorRanges = 1;
	rootParameters[11].DescriptorTable.pDescriptorRanges = &SRVDescriptorRank;

	D3D12_ROOT_SIGNATURE_DESC dxrRootSignatureDesc = {0};
	dxrRootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
	dxrRootSignatureDesc.pParameters = rootParameters;
	ID3DBlob *blob = NULL;
	ID3DBlob *error = NULL;
	D3D12SerializeRootSignature(&dxrRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &blob, &error);
	if (dxrRootSignature != NULL) dxrRootSignature->Release();
	device->CreateRootSignature(1, blob->GetBufferPointer(), blob->GetBufferSize(), IID_GRAPHICS_PPV_ARGS(&dxrRootSignature));

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {};
	raytracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	D3D12_SHADER_BYTECODE shaderBytecode = {};
	shaderBytecode.pShaderBytecode = ray_shader;
	shaderBytecode.BytecodeLength = ray_shader_size;

	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {};
	dxilLibrary.DXILLibrary = shaderBytecode;
	D3D12_EXPORT_DESC exports[3] = {};
	exports[0].Name = raygen_shader_name;
	exports[1].Name = closesthit_shader_name;
	exports[2].Name = miss_shader_name;
	dxilLibrary.pExports = exports;
	dxilLibrary.NumExports = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {};
	hitGroup.ClosestHitShaderImport = closesthit_shader_name;
	hitGroup.HitGroupExport = hit_group_name;
	hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {};
	shaderConfig.MaxPayloadSizeInBytes = 10 * sizeof(float); // float4 color
	shaderConfig.MaxAttributeSizeInBytes = 8 * sizeof(float); // float2 barycentrics

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {};
	pipelineConfig.MaxTraceRecursionDepth = 1; // ~ primary rays only

	D3D12_STATE_SUBOBJECT subobjects[5] = {};
	subobjects[0].Type = D3D12_STATE_SUBOBJECT_TYPE_DXIL_LIBRARY;
	subobjects[0].pDesc = &dxilLibrary;
	subobjects[1].Type = D3D12_STATE_SUBOBJECT_TYPE_HIT_GROUP;
	subobjects[1].pDesc = &hitGroup;
	subobjects[2].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_SHADER_CONFIG;
	subobjects[2].pDesc = &shaderConfig;
	subobjects[3].Type = D3D12_STATE_SUBOBJECT_TYPE_GLOBAL_ROOT_SIGNATURE;
	subobjects[3].pDesc = &dxrRootSignature;
	subobjects[4].Type = D3D12_STATE_SUBOBJECT_TYPE_RAYTRACING_PIPELINE_CONFIG;
	subobjects[4].pDesc = &pipelineConfig;
	raytracingPipeline.NumSubobjects = 5;
	raytracingPipeline.pSubobjects = subobjects;

	dxrDevice->CreateStateObject(&raytracingPipeline, IID_GRAPHICS_PPV_ARGS(&pipeline->impl.dxr_state));

	// Shader tables
	// Get shader identifiers
	ID3D12StateObjectProperties *stateObjectProps = NULL;
	pipeline->impl.dxr_state->QueryInterface(IID_GRAPHICS_PPV_ARGS(&stateObjectProps));
	const void *rayGenShaderId = stateObjectProps->GetShaderIdentifier(raygen_shader_name);
	const void *missShaderId = stateObjectProps->GetShaderIdentifier(miss_shader_name);
	const void *hitGroupShaderId = stateObjectProps->GetShaderIdentifier(hit_group_name);
	UINT shaderIdSize = D3D12_SHADER_IDENTIFIER_SIZE_IN_BYTES;
	int align = D3D12_RAYTRACING_SHADER_RECORD_BYTE_ALIGNMENT;

	// Ray gen shader table
	{
		UINT size = shaderIdSize + constant_buffer->impl.mySize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&pipeline->impl.raygen_shader_table));

		D3D12_RANGE rstRange = {};
		rstRange.Begin = 0;
		rstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->Map(0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {};
		cbRange.Begin = 0;
		cbRange.End = constant_buffer->impl.mySize;
		void *constantBufferData;
		constant_buffer->impl.constant_buffer->Map(0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->impl.mySize);
		pipeline->impl.raygen_shader_table->Unmap(0, NULL);
	}

	// Miss shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&pipeline->impl.miss_shader_table));

		D3D12_RANGE mstRange = {};
		mstRange.Begin = 0;
		mstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->Map(0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->Unmap(0, NULL);
	}

	// Hit group shader table
	{
		UINT size = shaderIdSize;
		UINT shaderRecordSize = (size + (align - 1)) & ~(align - 1);
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = shaderRecordSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&pipeline->impl.hitgroup_shader_table));

		D3D12_RANGE hstRange = {};
		hstRange.Begin = 0;
		hstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.hitgroup_shader_table->Map(0, &hstRange, (void **)(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->Unmap(0, NULL);
	}

	// Output descriptor
	outputCpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);

	int descriptorHeapIndex = descriptorsAllocated++;
	outputDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorHeapIndex) * (UINT64)(descriptorSize);
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {
	pipeline->impl.dxr_state->Release();
	pipeline->impl.raygen_shader_table->Release();
	pipeline->impl.miss_shader_table->Release();
	pipeline->impl.hitgroup_shader_table->Release();
}

UINT create_srv_vb(kinc_g5_vertex_buffer_t *vb, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	if (elementSize == 0) {
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	device->CreateShaderResourceView(vb->impl.uploadBuffer, &srvDesc, cpuDescriptor);
	vbgpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

UINT create_srv_ib(kinc_g5_index_buffer_t *ib, UINT numElements, UINT elementSize) {
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.Buffer.NumElements = numElements;
	if (elementSize == 0) {
		srvDesc.Format = DXGI_FORMAT_R32_TYPELESS;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_RAW;
		srvDesc.Buffer.StructureByteStride = 0;
	}
	else {
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;
		srvDesc.Buffer.StructureByteStride = elementSize;
	}

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	device->CreateShaderResourceView(ib->impl.upload_buffer, &srvDesc, cpuDescriptor);
	ibgpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb,
                                               kinc_g5_index_buffer_t *ib, float scale) {
	create_srv_ib(ib, ib->impl.count, 0);
	create_srv_vb(vb, vb->impl.myCount, 8 * 2);

	// Reset the command list for the acceleration structure construction
	command_list->impl._commandList->Reset(command_list->impl._commandAllocator, NULL);

	D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
	geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
	geometryDesc.Triangles.IndexBuffer = ib->impl.upload_buffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.IndexCount = ib->impl.count;
	geometryDesc.Triangles.IndexFormat = ib->impl.format == KINC_G5_INDEX_BUFFER_FORMAT_16BIT ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
	geometryDesc.Triangles.Transform3x4 = 0;
	geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
	geometryDesc.Triangles.VertexCount = vb->impl.myCount;
	geometryDesc.Triangles.VertexBuffer.StartAddress = vb->impl.uploadBuffer->GetGPUVirtualAddress();
	geometryDesc.Triangles.VertexBuffer.StrideInBytes = vb->impl.uploadBuffer->GetDesc().Width / vb->impl.myCount;
	geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;

	// Get required sizes for an acceleration structure
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&topLevelInputs, &topLevelPrebuildInfo);

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs = topLevelInputs;
	bottomLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
	bottomLevelInputs.pGeometryDescs = &geometryDesc;
	bottomLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	dxrDevice->GetRaytracingAccelerationStructurePrebuildInfo(&bottomLevelInputs, &bottomLevelPrebuildInfo);

	ID3D12Resource *scratchResource;
	{
		UINT64 tlSize = topLevelPrebuildInfo.ScratchDataSizeInBytes;
		UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = tlSize > blSize ? tlSize : blSize;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&scratchResource));
	}

	// Allocate resources for acceleration structures
	// The resources that will contain acceleration structures must be created in the state D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE,
	// and must have resource flag D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS.
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = bottomLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&accel->impl.bottom_level_accel));
	}
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = topLevelPrebuildInfo.ResultDataMaxSizeInBytes;
		bufferDesc.Height = 1;
		bufferDesc.DepthOrArraySize = 1;
		bufferDesc.MipLevels = 1;
		bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
		bufferDesc.SampleDesc.Count = 1;
		bufferDesc.SampleDesc.Quality = 0;
		bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		bufferDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		uploadHeapProperties.CreationNodeMask = 1;
		uploadHeapProperties.VisibleNodeMask = 1;

		device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
		                                IID_GRAPHICS_PPV_ARGS(&accel->impl.top_level_accel));
	}

	// Create an instance desc for the bottom-level acceleration structure
	ID3D12Resource *instanceDescs;
	D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {};
	instanceDesc.Transform[0][0] = instanceDesc.Transform[1][1] = instanceDesc.Transform[2][2] = scale;
	instanceDesc.InstanceMask = 1;
	instanceDesc.AccelerationStructure = accel->impl.bottom_level_accel->GetGPUVirtualAddress();

	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(instanceDesc);
	bufferDesc.Height = 1;
	bufferDesc.DepthOrArraySize = 1;
	bufferDesc.MipLevels = 1;
	bufferDesc.Format = DXGI_FORMAT_UNKNOWN;
	bufferDesc.SampleDesc.Count = 1;
	bufferDesc.SampleDesc.Quality = 0;
	bufferDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	D3D12_HEAP_PROPERTIES uploadHeapProperties = {};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	uploadHeapProperties.CreationNodeMask = 1;
	uploadHeapProperties.VisibleNodeMask = 1;

	device->CreateCommittedResource(&uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                IID_GRAPHICS_PPV_ARGS(&instanceDescs));
	void *mappedData;
	instanceDescs->Map(0, NULL, &mappedData);
	memcpy(mappedData, &instanceDesc, sizeof(instanceDesc));
	instanceDescs->Unmap(0, NULL);

	// Bottom Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {0};
	bottomLevelBuildDesc.Inputs = bottomLevelInputs;
	bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();
	bottomLevelBuildDesc.DestAccelerationStructureData = accel->impl.bottom_level_accel->GetGPUVirtualAddress();

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = bottomLevelBuildDesc;
	topLevelInputs.InstanceDescs = instanceDescs->GetGPUVirtualAddress();
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData = accel->impl.top_level_accel->GetGPUVirtualAddress();
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->GetGPUVirtualAddress();

	// Build acceleration structure
	dxrCommandList->BuildRaytracingAccelerationStructure(&bottomLevelBuildDesc, 0, NULL);
	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = accel->impl.bottom_level_accel;
	command_list->impl._commandList->ResourceBarrier(1, &barrier);
	dxrCommandList->BuildRaytracingAccelerationStructure(&topLevelBuildDesc, 0, NULL);

	kinc_g5_command_list_end(command_list);
	kinc_g5_command_list_execute(command_list);
	kinc_g5_command_list_wait_for_execution_to_finish(command_list);
	kinc_g5_command_list_begin(command_list);

	scratchResource->Release();
	instanceDescs->Release();
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {
	accel->impl.bottom_level_accel->Release();
	accel->impl.top_level_accel->Release();
}

void kinc_raytrace_set_textures(kinc_g5_render_target_t *texpaint0, kinc_g5_render_target_t *texpaint1, kinc_g5_render_target_t *texpaint2, kinc_g5_texture_t *texenv, kinc_g5_texture_t *texsobol, kinc_g5_texture_t *texscramble, kinc_g5_texture_t *texrank) {
	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 5 * (UINT64)(descriptorSize);
	D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu = texpaint0->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	tex0gpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 5 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 6 * (UINT64)(descriptorSize);
	sourceCpu = texpaint1->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	tex1gpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 6 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 7 * (UINT64)(descriptorSize);
	sourceCpu = texpaint2->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	tex2gpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 7 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 8 * (UINT64)(descriptorSize);
	sourceCpu = texenv->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	texenvgpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 8 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 9 * (UINT64)(descriptorSize);
	sourceCpu = texsobol->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	texsobolgpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 9 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 10 * (UINT64)(descriptorSize);
	sourceCpu = texscramble->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	texscramblegpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 10 * (UINT64)(descriptorSize);

	cpuDescriptor.ptr = descriptorHeap->GetCPUDescriptorHandleForHeapStart().ptr + 11 * (UINT64)(descriptorSize);
	sourceCpu = texrank->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	device->CopyDescriptorsSimple(1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	texrankgpuDescriptorHandle.ptr = descriptorHeap->GetGPUDescriptorHandleForHeapStart().ptr + 11 * (UINT64)(descriptorSize);
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_render_target_t *_output) {
	if (_output != output) {
		_output->impl.renderTarget->Release();
		_output->impl.renderTargetDescriptorHeap->Release();
		_output->impl.srvDescriptorHeap->Release();

		D3D12_HEAP_PROPERTIES heapProperties = {};
		heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
		heapProperties.CreationNodeMask = 1;
		heapProperties.VisibleNodeMask = 1;
		D3D12_RESOURCE_DESC desc = {};
		desc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		desc.Width = _output->texWidth;
		desc.Height = _output->texHeight;
		desc.DepthOrArraySize = 1;
		desc.MipLevels = 1;
		desc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		desc.Flags = D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET | D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
		D3D12_CLEAR_VALUE clearValue;
		clearValue.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		clearValue.Color[0] = 0.0f;
		clearValue.Color[1] = 0.0f;
		clearValue.Color[2] = 0.0f;
		clearValue.Color[3] = 1.0f;

		device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
		                                D3D12_RESOURCE_STATE_COMMON, &clearValue, IID_GRAPHICS_PPV_ARGS(&_output->impl.renderTarget));

		D3D12_RENDER_TARGET_VIEW_DESC view;
		view.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;
		view.Texture2D.PlaneSlice = 0;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&heapDesc, IID_GRAPHICS_PPV_ARGS(&_output->impl.renderTargetDescriptorHeap));
		device->CreateRenderTargetView(_output->impl.renderTarget, &view,
		                               _output->impl.renderTargetDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NodeMask = 0;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->CreateDescriptorHeap(&descriptorHeapDesc, IID_GRAPHICS_PPV_ARGS(&_output->impl.srvDescriptorHeap));

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		device->CreateShaderResourceView(_output->impl.renderTarget, &shaderResourceViewDesc,
		                                 _output->impl.srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->CreateUnorderedAccessView(_output->impl.renderTarget, NULL, &UAVDesc, outputCpuDescriptor);
	}
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	command_list->impl._commandList->SetComputeRootSignature(dxrRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->impl._commandList->SetDescriptorHeaps(1, &descriptorHeap);
	command_list->impl._commandList->SetComputeRootDescriptorTable(0, outputDescriptorHandle);
	command_list->impl._commandList->SetComputeRootShaderResourceView(1, accel->impl.top_level_accel->GetGPUVirtualAddress());
	command_list->impl._commandList->SetComputeRootDescriptorTable(2, ibgpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(3, vbgpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootConstantBufferView(4, pipeline->_constant_buffer->impl.constant_buffer->GetGPUVirtualAddress());
	command_list->impl._commandList->SetComputeRootDescriptorTable(5, tex0gpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(6, tex1gpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(7, tex2gpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(8, texenvgpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(9, texsobolgpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(10, texscramblegpuDescriptorHandle);
	command_list->impl._commandList->SetComputeRootDescriptorTable(11, texrankgpuDescriptorHandle);

	// Since each shader table has only one shader record, the stride is same as the size.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {0};
	dispatchDesc.HitGroupTable.StartAddress = pipeline->impl.hitgroup_shader_table->GetGPUVirtualAddress();
	dispatchDesc.HitGroupTable.SizeInBytes = pipeline->impl.hitgroup_shader_table->GetDesc().Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = pipeline->impl.miss_shader_table->GetGPUVirtualAddress();
	dispatchDesc.MissShaderTable.SizeInBytes = pipeline->impl.miss_shader_table->GetDesc().Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->impl.raygen_shader_table->GetGPUVirtualAddress();
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = pipeline->impl.raygen_shader_table->GetDesc().Width;
	dispatchDesc.Width = output->texWidth;
	dispatchDesc.Height = output->texHeight;
	dispatchDesc.Depth = 1;
	dxrCommandList->SetPipelineState1(pipeline->impl.dxr_state);
	dxrCommandList->DispatchRays(&dispatchDesc);
}

void kinc_raytrace_copy(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t *target, kinc_g5_texture_t *source) {
	D3D12_RESOURCE_BARRIER preCopyBarriers[2] = {};
	preCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preCopyBarriers[0].Transition.pResource = target->impl.renderTarget;
	preCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	preCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_DEST;
	preCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	preCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	preCopyBarriers[1].Transition.pResource = source->impl.image;
	preCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	preCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_COPY_SOURCE;
	preCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->impl._commandList->ResourceBarrier(ARRAYSIZE(preCopyBarriers), preCopyBarriers);

	command_list->impl._commandList->CopyResource(target->impl.renderTarget, source->impl.image);

	D3D12_RESOURCE_BARRIER postCopyBarriers[2] = {};
	postCopyBarriers[0].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postCopyBarriers[0].Transition.pResource = target->impl.renderTarget;
	postCopyBarriers[0].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	postCopyBarriers[0].Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	postCopyBarriers[0].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	postCopyBarriers[1].Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	postCopyBarriers[1].Transition.pResource = source->impl.image;
	postCopyBarriers[1].Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_SOURCE;
	postCopyBarriers[1].Transition.StateAfter = D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
	postCopyBarriers[1].Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	command_list->impl._commandList->ResourceBarrier(ARRAYSIZE(postCopyBarriers), postCopyBarriers);
}

#endif
