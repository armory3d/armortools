#include <kinc/graphics5/compute.h>
#include <kinc/graphics4/texture.h>
#include <kinc/math/core.h>
#include <Metal/Metal.h>

id getMetalDevice(void);
id getMetalLibrary(void);

void kinc_g5_compute_shader_init(kinc_g5_compute_shader *shader, void *_data, int length) {
	shader->impl.name[0] = 0;

	{
		uint8_t *data = (uint8_t *)_data;
		if (length > 1 && data[0] == '>') {
			memcpy(shader->impl.name, data + 1, length - 1);
			shader->impl.name[length - 1] = 0;
		}
		else {
			for (int i = 3; i < length; ++i) {
				if (data[i] == '\n') {
					shader->impl.name[i - 3] = 0;
					break;
				}
				else {
					shader->impl.name[i - 3] = data[i];
				}
			}
		}
	}

	char *data = (char *)_data;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:nil];
	}
	id<MTLFunction> function = [library newFunctionWithName:[NSString stringWithCString:shader->impl.name encoding:NSUTF8StringEncoding]];
	assert(function != nil);
	shader->impl._function = (__bridge_retained void *)function;

	id<MTLDevice> device = getMetalDevice();
	MTLComputePipelineReflection *reflection = nil;
	NSError *error = nil;
	shader->impl._pipeline = (__bridge_retained void *)[device newComputePipelineStateWithFunction:function
	                                                                                       options:MTLPipelineOptionBufferTypeInfo
	                                                                                    reflection:&reflection
	                                                                                         error:&error];
	if (error != nil)
		NSLog(@"%@", [error localizedDescription]);
	assert(shader->impl._pipeline != NULL && !error);
	shader->impl._reflection = (__bridge_retained void *)reflection;
}

void kinc_g5_compute_shader_destroy(kinc_g5_compute_shader *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl._function;
	function = nil;
	shader->impl._function = NULL;

	id<MTLComputePipelineState> pipeline = (__bridge_transfer id<MTLComputePipelineState>)shader->impl._pipeline;
	pipeline = nil;
	shader->impl._pipeline = NULL;

	MTLComputePipelineReflection *reflection = (__bridge_transfer MTLComputePipelineReflection *)shader->impl._reflection;
	reflection = nil;
	shader->impl._reflection = NULL;
}

kinc_g5_constant_location_t kinc_g5_compute_shader_get_constant_location(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_constant_location_t location;
	location.impl.vertexOffset = -1;
	location.impl.fragmentOffset = -1;
	location.impl.computeOffset = -1;

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;

	for (MTLArgument *arg in reflection.arguments) {
		if (arg.type == MTLArgumentTypeBuffer && [arg.name isEqualToString:@"uniforms"]) {
			if ([arg bufferDataType] == MTLDataTypeStruct) {
				MTLStructType *structObj = [arg bufferStructType];
				for (MTLStructMember *member in structObj.members) {
					if (strcmp([[member name] UTF8String], name) == 0) {
						location.impl.computeOffset = (int)[member offset];
						break;
					}
				}
			}
			break;
		}
	}

	return location;
}

kinc_g5_texture_unit_t kinc_g5_compute_shader_get_texture_unit(kinc_g5_compute_shader *shader, const char *name) {
	kinc_g5_texture_unit_t unit;
	for (int i = 0; i < KINC_G5_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}

	MTLComputePipelineReflection *reflection = (__bridge MTLComputePipelineReflection *)shader->impl._reflection;
	for (MTLArgument *arg in reflection.arguments) {
		if ([arg type] == MTLArgumentTypeTexture && strcmp([[arg name] UTF8String], name) == 0) {
			unit.stages[KINC_G5_SHADER_TYPE_COMPUTE] = (int)[arg index];
		}
	}

	return unit;
}
