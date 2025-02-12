#include <kinc/graphics5/graphics.h>
#include <kinc/graphics5/shader.h>
#include <kinc/log.h>
#include <kinc/math/core.h>
#include <Metal/Metal.h>

id getMetalDevice(void);
id getMetalLibrary(void);

void kinc_g5_shader_destroy(kinc_g5_shader_t *shader) {
	id<MTLFunction> function = (__bridge_transfer id<MTLFunction>)shader->impl.mtlFunction;
	function = nil;
	shader->impl.mtlFunction = NULL;
}

void kinc_g5_shader_init(kinc_g5_shader_t *shader, const void *source, size_t length, kinc_g5_shader_type_t type) {
	shader->impl.name[0] = 0;

	{
		uint8_t *data = (uint8_t *)source;
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

	char *data = (char *)source;
	id<MTLLibrary> library = nil;
	if (length > 1 && data[0] == '>') {
		library = getMetalLibrary();
	}
	else {
		id<MTLDevice> device = getMetalDevice();
		NSError *error = nil;
		library = [device newLibraryWithSource:[[NSString alloc] initWithBytes:data length:length encoding:NSUTF8StringEncoding] options:nil error:&error];
		if (library == nil) {
			kinc_log(KINC_LOG_LEVEL_ERROR, "%s", error.localizedDescription.UTF8String);
		}
	}
	shader->impl.mtlFunction = (__bridge_retained void *)[library newFunctionWithName:[NSString stringWithCString:shader->impl.name
	                                                                                                     encoding:NSUTF8StringEncoding]];

	assert(shader->impl.mtlFunction);
}
