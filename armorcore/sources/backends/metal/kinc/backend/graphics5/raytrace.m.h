#include <kinc/backend/graphics5/raytrace.h>
#include <kinc/graphics5/commandlist.h>
#include <kinc/graphics5/constantbuffer.h>
#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/indexbuffer.h>
#include <kinc/graphics5/raytrace.h>
#include <kinc/graphics5/vertexbuffer.h>

static kinc_raytrace_acceleration_structure_t *accel;
static kinc_raytrace_pipeline_t *pipeline;
static kinc_g5_render_target_t *output = NULL;
static kinc_g5_constant_buffer_t *constant_buf;

id getMetalDevice(void);
id getMetalQueue(void);

id<MTLComputePipelineState> _raytracing_pipeline;
NSMutableArray *_primitive_accels;
id<MTLAccelerationStructure> _instance_accel;
dispatch_semaphore_t _sem;

static kinc_g5_render_target_t *_texpaint0;
static kinc_g5_render_target_t *_texpaint1;
static kinc_g5_render_target_t *_texpaint2;
static kinc_g5_texture_t *_texenv;
static kinc_g5_texture_t *_texsobol;
static kinc_g5_texture_t *_texscramble;
static kinc_g5_texture_t *_texrank;
static kinc_g5_vertex_buffer_t *_vb;
static kinc_g5_index_buffer_t *_ib;

bool kinc_raytrace_supported() {
	id<MTLDevice> device = getMetalDevice();
	return device.supportsRaytracing;
}

void kinc_raytrace_pipeline_init(kinc_raytrace_pipeline_t *pipeline, kinc_g5_command_list_t *command_list, void *ray_shader, int ray_shader_size,
                                 kinc_g5_constant_buffer_t *constant_buffer) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	constant_buf = constant_buffer;

	NSError *error = nil;
	id<MTLLibrary> library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:ray_shader length:ray_shader_size encoding:NSUTF8StringEncoding]
	                                              options:nil
	                                                error:&error];
	if (library == nil) {
		kinc_log(KINC_LOG_LEVEL_ERROR, "%s", error.localizedDescription.UTF8String);
	}

	MTLComputePipelineDescriptor *descriptor = [[MTLComputePipelineDescriptor alloc] init];
	descriptor.computeFunction = [library newFunctionWithName:@"raytracingKernel"];
	descriptor.threadGroupSizeIsMultipleOfThreadExecutionWidth = YES;
	_raytracing_pipeline = [device newComputePipelineStateWithDescriptor:descriptor options:0 reflection:nil error:&error];
	_sem = dispatch_semaphore_create(2);
}

void kinc_raytrace_pipeline_destroy(kinc_raytrace_pipeline_t *pipeline) {}

id<MTLAccelerationStructure> create_acceleration_sctructure(MTLAccelerationStructureDescriptor *descriptor) {
	id<MTLDevice> device = getMetalDevice();
	id<MTLCommandQueue> queue = getMetalQueue();

	MTLAccelerationStructureSizes accel_sizes = [device accelerationStructureSizesWithDescriptor:descriptor];
	id<MTLAccelerationStructure> acceleration_structure = [device newAccelerationStructureWithSize:accel_sizes.accelerationStructureSize];

	id<MTLBuffer> scratch_buffer = [device newBufferWithLength:accel_sizes.buildScratchBufferSize options:MTLResourceStorageModePrivate];
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	id<MTLAccelerationStructureCommandEncoder> command_encoder = [command_buffer accelerationStructureCommandEncoder];
	id<MTLBuffer> compacteds_size_buffer = [device newBufferWithLength:sizeof(uint32_t) options:MTLResourceStorageModeShared];

	[command_encoder buildAccelerationStructure:acceleration_structure descriptor:descriptor scratchBuffer:scratch_buffer scratchBufferOffset:0];

	[command_encoder writeCompactedAccelerationStructureSize:acceleration_structure toBuffer:compacteds_size_buffer offset:0];

	[command_encoder endEncoding];
	[command_buffer commit];
	[command_buffer waitUntilCompleted];

	uint32_t compacted_size = *(uint32_t *)compacteds_size_buffer.contents;
	id<MTLAccelerationStructure> compacted_acceleration_structure = [device newAccelerationStructureWithSize:compacted_size];
	command_buffer = [queue commandBuffer];
	command_encoder = [command_buffer accelerationStructureCommandEncoder];
	[command_encoder copyAndCompactAccelerationStructure:acceleration_structure toAccelerationStructure:compacted_acceleration_structure];
	[command_encoder endEncoding];
	[command_buffer commit];

	return compacted_acceleration_structure;
}

void kinc_raytrace_acceleration_structure_init(kinc_raytrace_acceleration_structure_t *accel, kinc_g5_command_list_t *command_list, kinc_g5_vertex_buffer_t *vb,
                                               kinc_g5_index_buffer_t *ib, float scale) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
#if !TARGET_OS_IPHONE
	MTLResourceOptions options = MTLResourceStorageModeManaged;
#else
	MTLResourceOptions options = MTLResourceStorageModeShared;
#endif

	_vb = vb;
	_ib = ib;

	MTLAccelerationStructureTriangleGeometryDescriptor *descriptor = [MTLAccelerationStructureTriangleGeometryDescriptor descriptor];
	descriptor.indexType = MTLIndexTypeUInt32;
	descriptor.indexBuffer = (__bridge id<MTLBuffer>)ib->impl.metal_buffer;
	descriptor.vertexBuffer = (__bridge id<MTLBuffer>)vb->impl.mtlBuffer;
	descriptor.vertexStride = vb->impl.myStride;
	descriptor.triangleCount = ib->impl.count / 3;
	descriptor.vertexFormat = MTLAttributeFormatShort4Normalized;

	MTLPrimitiveAccelerationStructureDescriptor *accel_descriptor = [MTLPrimitiveAccelerationStructureDescriptor descriptor];
	accel_descriptor.geometryDescriptors = @[ descriptor ];
	id<MTLAccelerationStructure> acceleration_structure = create_acceleration_sctructure(accel_descriptor);
	_primitive_accels = [[NSMutableArray alloc] init];
	[_primitive_accels addObject:acceleration_structure];

	id<MTLBuffer> instance_buffer = [device newBufferWithLength:sizeof(MTLAccelerationStructureInstanceDescriptor) * 1 options:options];

	MTLAccelerationStructureInstanceDescriptor *instance_descriptors = (MTLAccelerationStructureInstanceDescriptor *)instance_buffer.contents;
	instance_descriptors[0].accelerationStructureIndex = 0;
	instance_descriptors[0].options = MTLAccelerationStructureInstanceOptionOpaque;
	instance_descriptors[0].mask = 1;
	instance_descriptors[0].transformationMatrix.columns[0] = MTLPackedFloat3Make(scale, 0, 0);
	instance_descriptors[0].transformationMatrix.columns[1] = MTLPackedFloat3Make(0, scale, 0);
	instance_descriptors[0].transformationMatrix.columns[2] = MTLPackedFloat3Make(0, 0, scale);
	instance_descriptors[0].transformationMatrix.columns[3] = MTLPackedFloat3Make(0, 0, 0);

#if !TARGET_OS_IPHONE
	[instance_buffer didModifyRange:NSMakeRange(0, instance_buffer.length)];
#endif

	MTLInstanceAccelerationStructureDescriptor *inst_accel_descriptor = [MTLInstanceAccelerationStructureDescriptor descriptor];
	inst_accel_descriptor.instancedAccelerationStructures = _primitive_accels;
	inst_accel_descriptor.instanceCount = 1;
	inst_accel_descriptor.instanceDescriptorBuffer = instance_buffer;
	_instance_accel = create_acceleration_sctructure(inst_accel_descriptor);
}

void kinc_raytrace_acceleration_structure_destroy(kinc_raytrace_acceleration_structure_t *accel) {}

void kinc_raytrace_set_textures(kinc_g5_render_target_t *texpaint0, kinc_g5_render_target_t *texpaint1, kinc_g5_render_target_t *texpaint2, kinc_g5_texture_t *texenv, kinc_g5_texture_t *texsobol, kinc_g5_texture_t *texscramble, kinc_g5_texture_t *texrank) {
	_texpaint0 = texpaint0;
	_texpaint1 = texpaint1;
	_texpaint2 = texpaint2;
	_texenv = texenv;
	_texsobol = texsobol;
	_texscramble = texscramble;
	_texrank = texrank;
}

void kinc_raytrace_set_acceleration_structure(kinc_raytrace_acceleration_structure_t *_accel) {
	accel = _accel;
}

void kinc_raytrace_set_pipeline(kinc_raytrace_pipeline_t *_pipeline) {
	pipeline = _pipeline;
}

void kinc_raytrace_set_target(kinc_g5_render_target_t *_output) {
	output = _output;
}

void kinc_raytrace_dispatch_rays(kinc_g5_command_list_t *command_list) {
	id<MTLDevice> device = getMetalDevice();
	if (!device.supportsRaytracing) return;
	dispatch_semaphore_wait(_sem, DISPATCH_TIME_FOREVER);

	id<MTLCommandQueue> queue = getMetalQueue();
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	__block dispatch_semaphore_t sem = _sem;
	[command_buffer addCompletedHandler:^(id<MTLCommandBuffer> buffer) {
		dispatch_semaphore_signal(sem);
	}];

	NSUInteger width = output->texWidth;
	NSUInteger height = output->texHeight;
	MTLSize threads_per_threadgroup = MTLSizeMake(8, 8, 1);
	MTLSize threadgroups = MTLSizeMake((width + threads_per_threadgroup.width - 1) / threads_per_threadgroup.width,
	                                   (height + threads_per_threadgroup.height - 1) / threads_per_threadgroup.height, 1);

	id<MTLComputeCommandEncoder> compute_encoder = [command_buffer computeCommandEncoder];
	[compute_encoder setBuffer:(__bridge id<MTLBuffer>)constant_buf->impl._buffer offset:0 atIndex:0];
	[compute_encoder setAccelerationStructure:_instance_accel atBufferIndex:1];
	[compute_encoder setBuffer: (__bridge id<MTLBuffer>)_ib->impl.metal_buffer offset:0 atIndex:2];
	[compute_encoder setBuffer: (__bridge id<MTLBuffer>)_vb->impl.mtlBuffer offset:0 atIndex:3];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)output->impl._tex atIndex:0];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint0->impl._tex atIndex:1];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint1->impl._tex atIndex:2];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texpaint2->impl._tex atIndex:3];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texenv->impl._tex atIndex:4];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texsobol->impl._tex atIndex:5];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texscramble->impl._tex atIndex:6];
	[compute_encoder setTexture:(__bridge id<MTLTexture>)_texrank->impl._tex atIndex:7];

	for (id<MTLAccelerationStructure> primitive_accel in _primitive_accels)
		[compute_encoder useResource:primitive_accel usage:MTLResourceUsageRead];

	[compute_encoder setComputePipelineState:_raytracing_pipeline];
	[compute_encoder dispatchThreadgroups:threadgroups threadsPerThreadgroup:threads_per_threadgroup];
	[compute_encoder endEncoding];
	[command_buffer commit];
}

void kinc_raytrace_copy(kinc_g5_command_list_t *command_list, kinc_g5_render_target_t *target, kinc_g5_texture_t *source) {
	id<MTLCommandQueue> queue = getMetalQueue();
	id<MTLCommandBuffer> command_buffer = [queue commandBuffer];
	id<MTLBlitCommandEncoder> command_encoder = [command_buffer blitCommandEncoder];
	[command_encoder copyFromTexture:(__bridge id<MTLTexture>)source->impl._tex toTexture:(__bridge id<MTLTexture>)target->impl._tex];
#ifndef KINC_APPLE_SOC
	[command_encoder synchronizeResource:(__bridge id<MTLTexture>)target->impl._tex];
#endif
	[command_encoder endEncoding];
	[command_buffer commit];
	[command_buffer waitUntilCompleted];
}
