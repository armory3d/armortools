
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

typedef struct inst {
	kinc_matrix4x4_t m;
	int i;
} inst_t;

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

static kinc_g5_vertex_buffer_t *vb[16];
static kinc_g5_vertex_buffer_t *vb_last[16];
static kinc_g5_index_buffer_t *ib[16];
static int vb_count = 0;
static int vb_count_last = 0;
static inst_t instances[1024];
static int instances_count = 0;

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
	if (descriptorHeap != NULL)
		descriptorHeap->lpVtbl->Release(descriptorHeap);
	device->lpVtbl->CreateDescriptorHeap(device , &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &descriptorHeap);
	descriptorSize = device->lpVtbl->GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	// Device
	if (dxrDevice != NULL)
		dxrDevice->lpVtbl->Release(dxrDevice);
	device->lpVtbl->QueryInterface(device , &IID_ID3D12Device5, &dxrDevice);
	if (dxrCommandList != NULL)
		dxrCommandList->lpVtbl->Release(dxrCommandList);
	command_list->impl._commandList->lpVtbl->QueryInterface(command_list->impl._commandList , &IID_ID3D12GraphicsCommandList4, &dxrCommandList);

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
	if (dxrRootSignature != NULL)
		dxrRootSignature->lpVtbl->Release(dxrRootSignature);
	device->lpVtbl->CreateRootSignature(device, 1, blob->lpVtbl->GetBufferPointer(blob), blob->lpVtbl->GetBufferSize(blob), &IID_ID3D12RootSignature,
	                                    &dxrRootSignature);

	// Pipeline
	D3D12_STATE_OBJECT_DESC raytracingPipeline = {};
	raytracingPipeline.Type = D3D12_STATE_OBJECT_TYPE_RAYTRACING_PIPELINE;

	D3D12_SHADER_BYTECODE shaderBytecode = {0};
	shaderBytecode.pShaderBytecode = ray_shader;
	shaderBytecode.BytecodeLength = ray_shader_size;

	D3D12_DXIL_LIBRARY_DESC dxilLibrary = {0};
	dxilLibrary.DXILLibrary = shaderBytecode;
	D3D12_EXPORT_DESC exports[3] = {0};
	exports[0].Name = raygen_shader_name;
	exports[1].Name = closesthit_shader_name;
	exports[2].Name = miss_shader_name;
	dxilLibrary.pExports = exports;
	dxilLibrary.NumExports = 3;

	D3D12_HIT_GROUP_DESC hitGroup = {0};
	hitGroup.ClosestHitShaderImport = closesthit_shader_name;
	hitGroup.HitGroupExport = hit_group_name;
	hitGroup.Type = D3D12_HIT_GROUP_TYPE_TRIANGLES;

	D3D12_RAYTRACING_SHADER_CONFIG shaderConfig = {0};
	shaderConfig.MaxPayloadSizeInBytes = 10 * sizeof(float); // float4 color
	shaderConfig.MaxAttributeSizeInBytes = 8 * sizeof(float); // float2 barycentrics

	D3D12_RAYTRACING_PIPELINE_CONFIG pipelineConfig = {0};
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

	dxrDevice->lpVtbl->CreateStateObject(dxrDevice , &raytracingPipeline, &IID_ID3D12StateObject, &pipeline->impl.dxr_state);

	// Shader tables
	// Get shader identifiers
	ID3D12StateObjectProperties *stateObjectProps = NULL;
	pipeline->impl.dxr_state->lpVtbl->QueryInterface(pipeline->impl.dxr_state , &IID_ID3D12StateObjectProperties, &stateObjectProps);
	const void *rayGenShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, raygen_shader_name);
	const void *missShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, miss_shader_name);
	const void *hitGroupShaderId = stateObjectProps->lpVtbl->GetShaderIdentifier(stateObjectProps, hit_group_name);
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

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.raygen_shader_table);

		D3D12_RANGE rstRange = {0};
		rstRange.Begin = 0;
		rstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.raygen_shader_table->lpVtbl->Map(pipeline->impl.raygen_shader_table, 0, &rstRange, (void **)(&byteDest));

		D3D12_RANGE cbRange = {0};
		cbRange.Begin = 0;
		cbRange.End = constant_buffer->impl.mySize;
		void *constantBufferData;
		constant_buffer->impl.constant_buffer->lpVtbl->Map(constant_buffer->impl.constant_buffer, 0, &cbRange, (void **)&constantBufferData);
		memcpy(byteDest, rayGenShaderId, size);
		memcpy(byteDest + size, constantBufferData, constant_buffer->impl.mySize);
		pipeline->impl.raygen_shader_table->lpVtbl->Unmap(pipeline->impl.raygen_shader_table, 0, NULL);
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

		device->lpVtbl->CreateCommittedResource(device , &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.miss_shader_table);

		D3D12_RANGE mstRange = {0};
		mstRange.Begin = 0;
		mstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.miss_shader_table->lpVtbl->Map(pipeline->impl.miss_shader_table, 0, &mstRange, (void **)(&byteDest));
		memcpy(byteDest, missShaderId, size);
		pipeline->impl.miss_shader_table->lpVtbl->Unmap(pipeline->impl.miss_shader_table, 0, NULL);
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

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
		                                &IID_ID3D12Resource, &pipeline->impl.hitgroup_shader_table);

		D3D12_RANGE hstRange = {0};
		hstRange.Begin = 0;
		hstRange.End = 0;
		uint8_t *byteDest;
		pipeline->impl.hitgroup_shader_table->lpVtbl->Map(pipeline->impl.hitgroup_shader_table, 0, &hstRange, (void **)(&byteDest));
		memcpy(byteDest, hitGroupShaderId, size);
		pipeline->impl.hitgroup_shader_table->lpVtbl->Unmap(pipeline->impl.hitgroup_shader_table, 0, NULL);
	}

	// Output descriptor
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	outputCpuDescriptor.ptr = handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	int descriptorHeapIndex = descriptorsAllocated++;
	outputDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorHeapIndex) * (UINT64)(descriptorSize);
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {
	pipeline->impl.dxr_state->lpVtbl->Release(pipeline->impl.dxr_state);
	pipeline->impl.raygen_shader_table->lpVtbl->Release(pipeline->impl.raygen_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->Release(pipeline->impl.miss_shader_table);
	pipeline->impl.hitgroup_shader_table->lpVtbl->Release(pipeline->impl.hitgroup_shader_table);
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

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr =
	    handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	device->lpVtbl->CreateShaderResourceView(device, vb->impl.uploadBuffer, &srvDesc, cpuDescriptor);

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	vbgpuDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

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

	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr =
	    handle.ptr + (INT64)(descriptorsAllocated) * (UINT64)(descriptorSize);
	UINT descriptorIndex = descriptorsAllocated++;

	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	device->lpVtbl->CreateShaderResourceView(device, ib->impl.upload_buffer, &srvDesc, cpuDescriptor);
	ibgpuDescriptorHandle.ptr =
	    handle.ptr + (INT64)(descriptorIndex) * (UINT64)(descriptorSize);

	return descriptorIndex;
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel) {
	vb_count = 0;
	instances_count = 0;
}

void kinc_raytrace_acceleration_structure_add(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_vertex_buffer_t *_vb, kinc_g5_index_buffer_t *_ib,
	kinc_matrix4x4_t _transform) {

	int vb_i = -1;
	for (int i = 0; i < vb_count; ++i) {
		if (_vb == vb[i]) {
			vb_i = i;
			break;
		}
	}
	if (vb_i == -1) {
		vb_i = vb_count;
		vb[vb_count] = _vb;
		ib[vb_count] = _ib;
		vb_count++;
	}

	inst_t inst = { .i = vb_i, .m =  _transform };
	instances[instances_count] = inst;
	instances_count++;
}

void _kinc_raytrace_acceleration_structure_destroy_bottom(kinc_raytrace_acceleration_structure_t *accel) {
	for (int i = 0; i < vb_count_last; ++i) {
		accel->impl.bottom_level_accel[i]->lpVtbl->Release(accel->impl.bottom_level_accel[i]);
	}
}

void _kinc_raytrace_acceleration_structure_destroy_top(kinc_raytrace_acceleration_structure_t *accel) {
	accel->impl.top_level_accel->lpVtbl->Release(accel->impl.top_level_accel);
}

void kinc_raytrace_acceleration_structure_build(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list,
	kinc_g5_vertex_buffer_t *_vb_full, kinc_g5_index_buffer_t *_ib_full) {

	bool build_bottom = false;
	for (int i = 0; i < 16; ++i) {
		if (vb_last[i] != vb[i]) {
			build_bottom = true;
		}
		vb_last[i] = vb[i];
	}

	if (vb_count_last > 0) {
		if (build_bottom) {
			_kinc_raytrace_acceleration_structure_destroy_bottom(accel);
		}
		_kinc_raytrace_acceleration_structure_destroy_top(accel);
	}

	vb_count_last = vb_count;

	if (vb_count == 0) {
		return;
	}

	descriptorsAllocated = 1; // 1 descriptor already allocated in kinc_raytrace_pipeline_init

	#ifdef is_forge
	create_srv_ib(_ib_full, _ib_full->impl.count, 0);
	create_srv_vb(_vb_full, _vb_full->impl.myCount, vb[0]->impl.myStride);
	#else
	create_srv_ib(ib[0], ib[0]->impl.count, 0);
	create_srv_vb(vb[0], vb[0]->impl.myCount, vb[0]->impl.myStride);
	#endif

	// Reset the command list for the acceleration structure construction
	command_list->impl._commandList->lpVtbl->Reset(command_list->impl._commandList, command_list->impl._commandAllocator, NULL);

	// Get required sizes for an acceleration structure
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS topLevelInputs = {};
	topLevelInputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
	topLevelInputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
	topLevelInputs.NumDescs = 1;
	topLevelInputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL;

	D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO topLevelPrebuildInfo = {0};
	dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice , &topLevelInputs, &topLevelPrebuildInfo);

	UINT64 scratch_size = topLevelPrebuildInfo.ScratchDataSizeInBytes;

	// Bottom AS
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS bottomLevelInputs[16];
	D3D12_RAYTRACING_GEOMETRY_DESC geometryDescs[16];
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			D3D12_RAYTRACING_GEOMETRY_DESC geometryDesc = {};
			geometryDesc.Type = D3D12_RAYTRACING_GEOMETRY_TYPE_TRIANGLES;
			geometryDesc.Triangles.IndexBuffer = ib[i]->impl.upload_buffer->lpVtbl->GetGPUVirtualAddress(ib[i]->impl.upload_buffer);
			geometryDesc.Triangles.IndexCount = ib[i]->impl.count;
			geometryDesc.Triangles.IndexFormat = DXGI_FORMAT_R32_UINT;
			geometryDesc.Triangles.Transform3x4 = 0;
			geometryDesc.Triangles.VertexFormat = DXGI_FORMAT_R16G16B16A16_SNORM;
			geometryDesc.Triangles.VertexCount = vb[i]->impl.myCount;

			D3D12_RESOURCE_DESC desc;
			vb[i]->impl.uploadBuffer->lpVtbl->GetDesc(vb[i]->impl.uploadBuffer, &desc);

			geometryDesc.Triangles.VertexBuffer.StartAddress = vb[i]->impl.uploadBuffer->lpVtbl->GetGPUVirtualAddress(vb[i]->impl.uploadBuffer);
			geometryDesc.Triangles.VertexBuffer.StrideInBytes =
			    desc.Width / vb[i]->impl.myCount;
			geometryDesc.Flags = D3D12_RAYTRACING_GEOMETRY_FLAG_OPAQUE;
			geometryDescs[i] = geometryDesc;

			D3D12_RAYTRACING_ACCELERATION_STRUCTURE_PREBUILD_INFO bottomLevelPrebuildInfo = {0};
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_INPUTS inputs = {};
			inputs.DescsLayout = D3D12_ELEMENTS_LAYOUT_ARRAY;
			inputs.NumDescs = 1;
			inputs.Type = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL;
			inputs.pGeometryDescs = &geometryDescs[i];
			inputs.Flags = D3D12_RAYTRACING_ACCELERATION_STRUCTURE_BUILD_FLAG_PREFER_FAST_TRACE;
			dxrDevice->lpVtbl->GetRaytracingAccelerationStructurePrebuildInfo(dxrDevice , &inputs, &bottomLevelPrebuildInfo);
			bottomLevelInputs[i] = inputs;

			UINT64 blSize = bottomLevelPrebuildInfo.ScratchDataSizeInBytes;
			if (scratch_size < blSize) {
				scratch_size = blSize;
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

				device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
				                                &IID_ID3D12Resource, &accel->impl.bottom_level_accel[i]);
			}
		}
	}

	// Create scratch memory
	ID3D12Resource *scratchResource;
	{
		D3D12_RESOURCE_DESC bufferDesc = {};
		bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		bufferDesc.Width = scratch_size;
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

		device->lpVtbl->CreateCommittedResource(dxrDevice, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, NULL,
		                                &IID_ID3D12Resource, &scratchResource);
	}

	// Bottom AS
	if (build_bottom) {
		for (int i = 0; i < vb_count; ++i) {
			// Bottom Level Acceleration Structure desc
			D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC bottomLevelBuildDesc = {0};
			bottomLevelBuildDesc.Inputs = bottomLevelInputs[i];
			bottomLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);
			bottomLevelBuildDesc.DestAccelerationStructureData =
			    accel->impl.bottom_level_accel[i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[i]);

			// Build acceleration structure
			dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &bottomLevelBuildDesc, 0, NULL);
		}
	}

	// Top AS
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

		device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_RAYTRACING_ACCELERATION_STRUCTURE, NULL,
		                                &IID_ID3D12Resource, &accel->impl.top_level_accel);
	}

	// Create an instance desc for the bottom-level acceleration structure
	D3D12_RESOURCE_DESC bufferDesc = {};
	bufferDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	bufferDesc.Width = sizeof(D3D12_RAYTRACING_INSTANCE_DESC) * instances_count;
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

	ID3D12Resource *instanceDescs;
	device->lpVtbl->CreateCommittedResource(device, &uploadHeapProperties, D3D12_HEAP_FLAG_NONE, &bufferDesc, D3D12_RESOURCE_STATE_GENERIC_READ, NULL,
	                                &IID_ID3D12Resource, &instanceDescs);
	void *mappedData;
	instanceDescs->lpVtbl->Map(instanceDescs, 0, NULL, &mappedData);

	for (int i = 0; i < instances_count; ++i) {
		D3D12_RAYTRACING_INSTANCE_DESC instanceDesc = {0};

		instanceDesc.Transform[0][0] = instances[i].m.m[0];
		instanceDesc.Transform[0][1] = instances[i].m.m[1];
		instanceDesc.Transform[0][2] = instances[i].m.m[2];
		instanceDesc.Transform[0][3] = instances[i].m.m[3];
		instanceDesc.Transform[1][0] = instances[i].m.m[4];
		instanceDesc.Transform[1][1] = instances[i].m.m[5];
		instanceDesc.Transform[1][2] = instances[i].m.m[6];
		instanceDesc.Transform[1][3] = instances[i].m.m[7];
		instanceDesc.Transform[2][0] = instances[i].m.m[8];
		instanceDesc.Transform[2][1] = instances[i].m.m[9];
		instanceDesc.Transform[2][2] = instances[i].m.m[10];
		instanceDesc.Transform[2][3] = instances[i].m.m[11];

		int ib_off = 0;
		for (int j = 0; j < instances[i].i; ++j) {
			ib_off += ib[j]->impl.count * 4;
		}
		instanceDesc.InstanceID = ib_off;
		instanceDesc.InstanceMask = 1;
		instanceDesc.AccelerationStructure =
		    accel->impl.bottom_level_accel[instances[i].i]->lpVtbl->GetGPUVirtualAddress(accel->impl.bottom_level_accel[instances[i].i]);
		memcpy((uint8_t *)mappedData + i * sizeof(D3D12_RAYTRACING_INSTANCE_DESC), &instanceDesc, sizeof(D3D12_RAYTRACING_INSTANCE_DESC));
	}

	instanceDescs->lpVtbl->Unmap(instanceDescs, 0, NULL);

	// Top Level Acceleration Structure desc
	D3D12_BUILD_RAYTRACING_ACCELERATION_STRUCTURE_DESC topLevelBuildDesc = {0};
	topLevelInputs.InstanceDescs = instanceDescs->lpVtbl->GetGPUVirtualAddress(instanceDescs);
	topLevelBuildDesc.Inputs = topLevelInputs;
	topLevelBuildDesc.DestAccelerationStructureData = accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel);
	topLevelBuildDesc.ScratchAccelerationStructureData = scratchResource->lpVtbl->GetGPUVirtualAddress(scratchResource);

	D3D12_RESOURCE_BARRIER barrier = {};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_UAV;
	barrier.UAV.pResource = accel->impl.bottom_level_accel[0];
	command_list->impl._commandList->lpVtbl->ResourceBarrier(command_list->impl._commandList, 1, &barrier);
	dxrCommandList->lpVtbl->BuildRaytracingAccelerationStructure(dxrCommandList, &topLevelBuildDesc, 0, NULL);

	kinc_g5_command_list_end(command_list);
	kinc_g5_command_list_execute(command_list);
	kinc_g5_command_list_wait_for_execution_to_finish(command_list);
	kinc_g5_command_list_begin(command_list);

	scratchResource->lpVtbl->Release(scratchResource);
	instanceDescs->lpVtbl->Release(instanceDescs);
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {
	// accel->impl.bottom_level_accel->Release();
	// accel->impl.top_level_accel->Release();
}

void kinc_raytrace_set_textures(kinc_g5_render_target_t *texpaint0, kinc_g5_render_target_t *texpaint1, kinc_g5_render_target_t *texpaint2, kinc_g5_texture_t *texenv, kinc_g5_texture_t *texsobol, kinc_g5_texture_t *texscramble, kinc_g5_texture_t *texrank) {
	D3D12_CPU_DESCRIPTOR_HANDLE handle;
	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);

	D3D12_CPU_DESCRIPTOR_HANDLE cpuDescriptor = {0};
	cpuDescriptor.ptr = handle.ptr + 5 * (UINT64)(descriptorSize);

	D3D12_CPU_DESCRIPTOR_HANDLE sourceCpu;
	texpaint0->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint0->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

	D3D12_GPU_DESCRIPTOR_HANDLE ghandle;
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex0gpuDescriptorHandle.ptr = ghandle.ptr + 5 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 6 * (UINT64)(descriptorSize);
	texpaint1->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint1->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex1gpuDescriptorHandle.ptr = ghandle.ptr + 6 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 7 * (UINT64)(descriptorSize);
	texpaint2->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texpaint2->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	tex2gpuDescriptorHandle.ptr = ghandle.ptr + 7 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 8 * (UINT64)(descriptorSize);
	texenv->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texenv->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texenvgpuDescriptorHandle.ptr = ghandle.ptr + 8 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 9 * (UINT64)(descriptorSize);
	texsobol->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texsobol->impl.srvDescriptorHeap, &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texsobolgpuDescriptorHandle.ptr = ghandle.ptr + 9 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 10 * (UINT64)(descriptorSize);
	texscramble->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texscramble->impl.srvDescriptorHeap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texscramblegpuDescriptorHandle.ptr = ghandle.ptr + 10 * (UINT64)(descriptorSize);

	descriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(descriptorHeap, &handle);
	cpuDescriptor.ptr = handle.ptr + 11 * (UINT64)(descriptorSize);
	texrank->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(texrank->impl.srvDescriptorHeap , &sourceCpu);
	device->lpVtbl->CopyDescriptorsSimple(device, 1, cpuDescriptor, sourceCpu, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	descriptorHeap->lpVtbl->GetGPUDescriptorHandleForHeapStart(descriptorHeap, &ghandle);
	texrankgpuDescriptorHandle.ptr = ghandle.ptr + 11 * (UINT64)(descriptorSize);
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_render_target_t *_output) {
	if (_output != output) {
		_output->impl.renderTarget->lpVtbl->Release(_output->impl.renderTarget);
		_output->impl.renderTargetDescriptorHeap->lpVtbl->Release(_output->impl.renderTargetDescriptorHeap);
		_output->impl.srvDescriptorHeap->lpVtbl->Release(_output->impl.srvDescriptorHeap);

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

		device->lpVtbl->CreateCommittedResource(device, &heapProperties, D3D12_HEAP_FLAG_NONE, &desc,
		                                D3D12_RESOURCE_STATE_COMMON, &clearValue, &IID_ID3D12Resource, &_output->impl.renderTarget);

		D3D12_RENDER_TARGET_VIEW_DESC view;
		view.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		view.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;
		view.Texture2D.MipSlice = 0;
		view.Texture2D.PlaneSlice = 0;
		D3D12_DESCRIPTOR_HEAP_DESC heapDesc = {};
		heapDesc.NumDescriptors = 1;
		heapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		heapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->lpVtbl->CreateDescriptorHeap(device, &heapDesc, &IID_ID3D12DescriptorHeap, &_output->impl.renderTargetDescriptorHeap);
		D3D12_CPU_DESCRIPTOR_HANDLE handle;
		_output->impl.renderTargetDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.renderTargetDescriptorHeap, &handle);
		device->lpVtbl->CreateRenderTargetView(device, _output->impl.renderTarget, &view,
		    handle);

		D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc = {};
		descriptorHeapDesc.NumDescriptors = 1;
		descriptorHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		descriptorHeapDesc.NodeMask = 0;
		descriptorHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		device->lpVtbl->CreateDescriptorHeap(device, &descriptorHeapDesc, &IID_ID3D12DescriptorHeap, &_output->impl.srvDescriptorHeap);

		D3D12_SHADER_RESOURCE_VIEW_DESC shaderResourceViewDesc = {};
		shaderResourceViewDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		shaderResourceViewDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		shaderResourceViewDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		shaderResourceViewDesc.Texture2D.MipLevels = 1;
		shaderResourceViewDesc.Texture2D.MostDetailedMip = 0;
		shaderResourceViewDesc.Texture2D.ResourceMinLODClamp = 0.0f;
		_output->impl.srvDescriptorHeap->lpVtbl->GetCPUDescriptorHandleForHeapStart(_output->impl.srvDescriptorHeap, &handle);
		device->lpVtbl->CreateShaderResourceView(device, _output->impl.renderTarget, &shaderResourceViewDesc,
		                                         handle);

		D3D12_UNORDERED_ACCESS_VIEW_DESC UAVDesc = {};
		UAVDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
		device->lpVtbl->CreateUnorderedAccessView(device, _output->impl.renderTarget, NULL, &UAVDesc, outputCpuDescriptor);
	}
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	command_list->impl._commandList->lpVtbl->SetComputeRootSignature(command_list->impl._commandList, dxrRootSignature);

	// Bind the heaps, acceleration structure and dispatch rays
	command_list->impl._commandList->lpVtbl->SetDescriptorHeaps(command_list->impl._commandList, 1, &descriptorHeap);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 0, outputDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootShaderResourceView(command_list->impl._commandList, 1, accel->impl.top_level_accel->lpVtbl->GetGPUVirtualAddress(accel->impl.top_level_accel));
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 2, ibgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 3, vbgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootConstantBufferView(command_list->impl._commandList, 4, pipeline->_constant_buffer->impl.constant_buffer->lpVtbl->GetGPUVirtualAddress(pipeline->_constant_buffer->impl.constant_buffer));
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 5, tex0gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 6, tex1gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 7, tex2gpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 8, texenvgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 9, texsobolgpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 10, texscramblegpuDescriptorHandle);
	command_list->impl._commandList->lpVtbl->SetComputeRootDescriptorTable(command_list->impl._commandList, 11, texrankgpuDescriptorHandle);

	// Since each shader table has only one shader record, the stride is same as the size.
	D3D12_DISPATCH_RAYS_DESC dispatchDesc = {0};
	D3D12_RESOURCE_DESC desc;
	pipeline->impl.hitgroup_shader_table->lpVtbl->GetDesc(pipeline->impl.hitgroup_shader_table, &desc);
	dispatchDesc.HitGroupTable.StartAddress = pipeline->impl.hitgroup_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.hitgroup_shader_table);
	dispatchDesc.HitGroupTable.SizeInBytes = desc.Width;
	dispatchDesc.HitGroupTable.StrideInBytes = dispatchDesc.HitGroupTable.SizeInBytes;
	dispatchDesc.MissShaderTable.StartAddress = pipeline->impl.miss_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.miss_shader_table);
	pipeline->impl.miss_shader_table->lpVtbl->GetDesc(pipeline->impl.miss_shader_table, &desc);
	dispatchDesc.MissShaderTable.SizeInBytes = desc.Width;
	dispatchDesc.MissShaderTable.StrideInBytes = dispatchDesc.MissShaderTable.SizeInBytes;
	dispatchDesc.RayGenerationShaderRecord.StartAddress = pipeline->impl.raygen_shader_table->lpVtbl->GetGPUVirtualAddress(pipeline->impl.raygen_shader_table);
	pipeline->impl.raygen_shader_table->lpVtbl->GetDesc(pipeline->impl.raygen_shader_table, &desc);
	dispatchDesc.RayGenerationShaderRecord.SizeInBytes = desc.Width;
	dispatchDesc.Width = output->texWidth;
	dispatchDesc.Height = output->texHeight;
	dispatchDesc.Depth = 1;
	dxrCommandList->lpVtbl->SetPipelineState1(dxrCommandList, pipeline->impl.dxr_state);
	dxrCommandList->lpVtbl->DispatchRays(dxrCommandList, &dispatchDesc);
}
