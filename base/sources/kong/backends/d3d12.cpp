#include "d3d12.h"

#include "../errors.h"

#ifdef _WIN32

#include "../log.h"

#ifdef noreturn
#undef noreturn
#endif

#include <atlbase.h>
#include <dxcapi.h>

#endif

#ifdef _WIN32
static const wchar_t *shader_string(shader_stage stage) {
	switch (stage) {
	case SHADER_STAGE_VERTEX:
		return L"vs_6_0";
	case SHADER_STAGE_FRAGMENT:
		return L"ps_6_0";
	case SHADER_STAGE_COMPUTE:
		return L"cs_6_0";
	case SHADER_STAGE_RAY_GENERATION:
		return L"lib_6_3";
	case SHADER_STAGE_AMPLIFICATION:
		return L"as_6_5";
	case SHADER_STAGE_MESH:
		return L"ms_6_5";
	default: {
		debug_context context = {0};
		error(context, "Unsupported shader stage/version combination");
		return L"unsupported";
	}
	}
}
#endif

int compile_hlsl_to_d3d12(const char *source, uint8_t **output, size_t *outputlength, shader_stage stage, bool debug) {
#ifdef _WIN32
	CComPtr<IDxcCompiler3> compiler;
	DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler));

	LPCWSTR compiler_args[] = {
	    L"-E", L"main",              // entry point
	    L"-T", shader_string(stage), // target
	    // L"-Qstrip_reflect",          // strip reflection into a seperate blob
	};

	LPCWSTR debug_compiler_args[] = {
	    L"-E",  L"main",              // entry point
	    L"-T",  shader_string(stage), // target
	    L"-Zi",                       // enable debug info
	    // L"-Fd", L"myshader.pdb", // the file name of the pdb.  This must either be supplied or the auto generated file name must be used
	    // L"myshader.hlsl", // optional shader source file name for error reporting and for PIX shader source view
	    // L"-Qstrip_reflect",          // strip reflection into a seperate blob
	};

	DxcBuffer source_buffer;
	source_buffer.Ptr      = source;
	source_buffer.Size     = strlen(source);
	source_buffer.Encoding = DXC_CP_ACP; // assume BOM says UTF8 or UTF16 or this is ANSI text

	CComPtr<IDxcResult> compiler_result;
	compiler->Compile(&source_buffer,                                                  // source buffer
	                  debug ? debug_compiler_args : compiler_args,                     // Array of pointers to arguments
	                  debug ? _countof(debug_compiler_args) : _countof(compiler_args), // Number of arguments
	                  NULL,                                                            // user-provided interface to handle #include directives (optional)
	                  IID_PPV_ARGS(&compiler_result)                                   // Compiler output status, buffer, and errors
	);

	CComPtr<IDxcBlobUtf8> errors = nullptr;
	compiler_result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errors), nullptr);
	// note that d3dcompiler would return null if no errors or warnings are present.
	// IDxcCompiler3::Compile will always return an error buffer but it's length will be zero if there are no warnings or errors
	if (errors != nullptr && errors->GetStringLength() != 0) {
		kong_log(LOG_LEVEL_INFO, "Warnings and Errors:\n%s", errors->GetStringPointer());
	}

	HRESULT result;
	compiler_result->GetStatus(&result);

	if (result == S_OK) {
		CComPtr<IDxcBlob>      shader_buffer = nullptr;
		CComPtr<IDxcBlobUtf16> shader_name   = nullptr;
		compiler_result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shader_buffer), &shader_name);
		if (shader_buffer == nullptr) {
			return 1;
		}
		else {
			*outputlength = shader_buffer->GetBufferSize();
			*output       = (uint8_t *)malloc(*outputlength);
			memcpy(*output, shader_buffer->GetBufferPointer(), shader_buffer->GetBufferSize());
		}

		/*
		        //
		        // save pdb
		        //
		        CComPtr<IDxcBlob> pPDB = nullptr;
		        CComPtr<IDxcBlobUtf16> pPDBName = nullptr;
		        pResults->GetOutput(DXC_OUT_PDB, IID_PPV_ARGS(&pPDB), &pPDBName);
		        {
		            FILE* fp = NULL;

		            // note that if you do not specifiy -Fd a pdb name will be automatically generated.  Use this filename to save the pdb so that PIX can find
		   it quickly _wfopen_s(&fp, pPDBName->GetStringPointer(), L"wb"); fwrite(pPDB->GetBufferPointer(), pPDB->GetBufferSize(), 1, fp); fclose(fp);
		        }

		        //
		        // print hash
		        //
		        CComPtr<IDxcBlob> pHash = nullptr;
		        pResults->GetOutput(DXC_OUT_SHADER_HASH, IID_PPV_ARGS(&pHash), nullptr);
		        if (pHash != nullptr)
		        {
		            wprintf(L"Hash: ");
		            DxcShaderHash* pHashBuf = (DxcShaderHash*)pHash->GetBufferPointer();
		            for (int i = 0; i < _countof(pHashBuf->HashDigest); i++)
		                wprintf(L"%x", pHashBuf->HashDigest[i]);
		            wprintf(L"\n");
		        }
		*/

		return 0;
	}
	else {
		return 1;
	}
#else
	return 1;
#endif
}
