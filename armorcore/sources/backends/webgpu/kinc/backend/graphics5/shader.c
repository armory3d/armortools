#include <kinc/graphics5/shader.h>
#include <string.h>

extern WGPUDevice device;

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
