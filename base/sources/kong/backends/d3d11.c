#include "d3d11.h"

#include "../errors.h"

#ifdef _WIN32

#include "../log.h"

#define INITGUID
#include <Windows.h>
#include <d3d11.h>

#include <D3Dcompiler.h>

#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#endif

#ifdef _WIN32
static const char *shaderString(shader_stage stage, int version) {
	if (version == 4) {
		switch (stage) {
		case SHADER_STAGE_VERTEX:
			return "vs_4_0";
		case SHADER_STAGE_FRAGMENT:
			return "ps_4_0";
		case SHADER_STAGE_COMPUTE:
			return "cs_4_0";
		default:
			assert(false);
			return "";
		}
	}
	else if (version == 5) {
		switch (stage) {
		case SHADER_STAGE_VERTEX:
			return "vs_5_0";
		case SHADER_STAGE_FRAGMENT:
			return "ps_5_0";
		case SHADER_STAGE_COMPUTE:
			return "cs_5_0";
		default:
			assert(false);
			return "";
		}
	}

	debug_context context = {0};
	error(context, "Unsupported shader stage/version combination");
	return "unsupported";
}
#endif

int compile_hlsl_to_d3d11(const char *source, uint8_t **output, size_t *outputlength, shader_stage stage, bool debug) {
#ifdef _WIN32
	size_t length = strlen(source);

	ID3DBlob *errorMessage = NULL;
	ID3DBlob *shaderBuffer = NULL;
	UINT      flags        = 0;

	if (debug) {
		flags |= D3DCOMPILE_DEBUG;
	}

	HRESULT hr = D3DCompile(source, length, "Unknown", NULL, NULL, "main", shaderString(stage, 4), flags, 0, &shaderBuffer, &errorMessage);
	if (hr != S_OK) {
		hr = D3DCompile(source, length, "Unknown", NULL, NULL, "main", shaderString(stage, 5), flags, 0, &shaderBuffer, &errorMessage);
	}

	if (hr == S_OK) {
		/*std::ostream *file;
		std::ofstream actualfile;
		std::ostrstream arrayout(output, 1024 * 1024);
		*outputlength = 0;

		if (output) {
		    file = &arrayout;
		}
		else {
		    actualfile.open(to, std::ios_base::binary);
		    file = &actualfile;
		}

		file->put((char)attributes.size());
		*outputlength += 1;
		for (std::map<std::string, int>::const_iterator attribute = attributes.begin(); attribute != attributes.end(); ++attribute) {
		    (*file) << attribute->first.c_str();
		    *outputlength += attribute->first.length();
		    file->put(0);
		    *outputlength += 1;
		    file->put(attribute->second);
		    *outputlength += 1;
		}

		ID3D11ShaderReflection *reflector = nullptr;
		D3DReflect(shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize(), IID_ID3D11ShaderReflection, (void **)&reflector);

		D3D11_SHADER_DESC desc;
		reflector->GetDesc(&desc);

		file->put(desc.BoundResources);
		*outputlength += 1;
		for (unsigned i = 0; i < desc.BoundResources; ++i) {
		    D3D11_SHADER_INPUT_BIND_DESC bindDesc;
		    reflector->GetResourceBindingDesc(i, &bindDesc);
		    (*file) << bindDesc.Name;
		    *outputlength += strlen(bindDesc.Name);
		    file->put(0);
		    *outputlength += 1;
		    file->put(bindDesc.BindPoint);
		    *outputlength += 1;
		}

		ID3D11ShaderReflectionConstantBuffer *constants = reflector->GetConstantBufferByName("$Globals");
		D3D11_SHADER_BUFFER_DESC bufferDesc;
		hr = constants->GetDesc(&bufferDesc);
		if (hr == S_OK) {
		    file->put(bufferDesc.Variables);
		    *outputlength += 1;
		    for (unsigned i = 0; i < bufferDesc.Variables; ++i) {
		        ID3D11ShaderReflectionVariable *variable = constants->GetVariableByIndex(i);
		        D3D11_SHADER_VARIABLE_DESC variableDesc;
		        hr = variable->GetDesc(&variableDesc);
		        if (hr == S_OK) {
		            (*file) << variableDesc.Name;
		            *outputlength += strlen(variableDesc.Name);
		            file->put(0);
		            *outputlength += 1;
		            file->write((char *)&variableDesc.StartOffset, 4);
		            *outputlength += 4;
		            file->write((char *)&variableDesc.Size, 4);
		            *outputlength += 4;
		            D3D11_SHADER_TYPE_DESC typeDesc;
		            hr = variable->GetType()->GetDesc(&typeDesc);
		            if (hr == S_OK) {
		                file->put(typeDesc.Columns);
		                *outputlength += 1;
		                file->put(typeDesc.Rows);
		                *outputlength += 1;
		            }
		            else {
		                file->put(0);
		                *outputlength += 1;
		                file->put(0);
		                *outputlength += 1;
		            }
		        }
		    }
		}
		else {
		    file->put(0);
		    *outputlength += 1;
		}
		file->write((char *)shaderBuffer->GetBufferPointer(), shaderBuffer->GetBufferSize());
		*outputlength += shaderBuffer->GetBufferSize();*/

		SIZE_T size   = shaderBuffer->lpVtbl->GetBufferSize(shaderBuffer);
		*outputlength = size;
		*output       = (uint8_t *)malloc(size);
		memcpy(*output, shaderBuffer->lpVtbl->GetBufferPointer(shaderBuffer), size);

		return 0;
	}
	else {
		debug_context context = {0};
		check(errorMessage != NULL, context, "Error message missing");
		SIZE_T size  = errorMessage->lpVtbl->GetBufferSize(errorMessage);
		char  *error = malloc(size + 1);
		check(error != NULL, context, "Could not allocate error string");
		memcpy(error, errorMessage->lpVtbl->GetBufferPointer(errorMessage), size);
		error[size] = 0;
		kong_log(LOG_LEVEL_ERROR, error);
		return 1;
	}
#else
	return 1;
#endif
}
