
#include <v8.h>
#include <kinc/log.h>

using namespace v8;

static Isolate *isolate;

extern "C" {
	void proc_xatlas_setVertexCount(int i);
	void proc_xatlas_setIndexCount(int i);
	int proc_xatlas_setPositions();
	int proc_xatlas_setNormals();
	int proc_xatlas_setIndices();
	uint8_t *proc_xatlas_getBuffer();
	uint32_t proc_xatlas_getBufferLength();
	int proc_xatlas_getVertexCount();
	int proc_xatlas_getIndexCount();
	int proc_xatlas_getPositions();
	int proc_xatlas_getNormals();
	int proc_xatlas_getUVs();
	int proc_xatlas_getIndices();
	void proc_xatlas_unwrap();
	void proc_xatlas_destroy();

	uint8_t *io_svg_getBuffer();
	uint32_t io_svg_getBufferLength();
	int io_svg_init(int i);
	void io_svg_parse();
	int io_svg_get_pixels_w();
	int io_svg_get_pixels_h();
	int io_svg_get_pixels();
	void io_svg_destroy();

	uint8_t *io_gltf_getBuffer();
	uint32_t io_gltf_getBufferLength();
	int io_gltf_init(int i);
	void io_gltf_parse();
	int io_gltf_get_vertex_count();
	int io_gltf_get_index_count();
	int io_gltf_get_indices();
	int io_gltf_get_positions();
	int io_gltf_get_normals();
	int io_gltf_get_uvs();
	float io_gltf_get_scale_pos();
	void io_gltf_destroy();

	uint8_t *io_usd_getBuffer();
	uint32_t io_usd_getBufferLength();
	int io_usd_init(int i);
	void io_usd_parse();
	int io_usd_get_vertex_count();
	int io_usd_get_index_count();
	int io_usd_get_indices();
	int io_usd_get_positions();
	int io_usd_get_normals();
	int io_usd_get_uvs();
	float io_usd_get_scale_pos();
	void io_usd_destroy();
}

namespace {
	void krom_uv_unwrap_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(proc_xatlas_getBuffer(), proc_xatlas_getBufferLength(), [](void*, size_t, void*) {}, nullptr);
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(buffer);
	}

	void krom_uv_unwrap_setVertexCount(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		proc_xatlas_setVertexCount(i);
	}

	void krom_uv_unwrap_setIndexCount(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		proc_xatlas_setIndexCount(i);
	}

	void krom_uv_unwrap_setPositions(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_setPositions();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_setNormals(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_setNormals();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_setIndices(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_setIndices();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getVertexCount(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getVertexCount();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getIndexCount(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getIndexCount();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getPositions(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getPositions();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getNormals(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getNormals();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getUVs(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getUVs();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_getIndices(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = proc_xatlas_getIndices();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_uv_unwrap_unwrap(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		proc_xatlas_unwrap();
	}

	void krom_uv_unwrap_destroy(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		proc_xatlas_destroy();
	}


	void krom_import_svg_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(io_svg_getBuffer(), io_svg_getBufferLength(), [](void*, size_t, void*) {}, nullptr);
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(buffer);
	}

	void krom_import_svg_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int j = io_svg_init(i);
		args.GetReturnValue().Set(Int32::New(isolate, j));
	}

	void krom_import_svg_parse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_svg_parse();
	}

	void krom_import_svg_get_pixels_w(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_svg_get_pixels_w();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_svg_get_pixels_h(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_svg_get_pixels_h();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_svg_get_pixels(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_svg_get_pixels();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_svg_destroy(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_svg_destroy();
	}


	void krom_import_gltf_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(io_gltf_getBuffer(), io_gltf_getBufferLength(), [](void*, size_t, void*) {}, nullptr);
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(buffer);
	}

	void krom_import_gltf_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int j = io_gltf_init(i);
		args.GetReturnValue().Set(Int32::New(isolate, j));
	}

	void krom_import_gltf_parse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_gltf_parse();
	}

	void krom_import_gltf_get_vertex_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_vertex_count();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_index_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_index_count();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_indices(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_indices();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_positions(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_positions();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_normals(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_normals();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_uvs(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_gltf_get_uvs();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_gltf_get_scale_pos(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float f = io_gltf_get_scale_pos();
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	void krom_import_gltf_destroy(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_gltf_destroy();
	}


	void krom_import_usdc_buffer(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		std::unique_ptr<v8::BackingStore> backing = v8::ArrayBuffer::NewBackingStore(io_usd_getBuffer(), io_usd_getBufferLength(), [](void*, size_t, void*) {}, nullptr);
		Local<ArrayBuffer> buffer = ArrayBuffer::New(isolate, std::move(backing));
		args.GetReturnValue().Set(buffer);
	}

	void krom_import_usdc_init(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int j = io_usd_init(i);
		args.GetReturnValue().Set(Int32::New(isolate, j));
	}

	void krom_import_usdc_parse(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_usd_parse();
	}

	void krom_import_usdc_get_vertex_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_vertex_count();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_index_count(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_index_count();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_indices(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_indices();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_positions(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_positions();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_normals(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_normals();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_uvs(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int i = io_usd_get_uvs();
		args.GetReturnValue().Set(Int32::New(isolate, i));
	}

	void krom_import_usdc_get_scale_pos(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		float f = io_usd_get_scale_pos();
		args.GetReturnValue().Set(Number::New(isolate, f));
	}

	void krom_import_usdc_destroy(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		io_usd_destroy();
	}
}

#define SET_FUNCTION(object, name, fn)\
	object->Set(String::NewFromUtf8(isolate, name).ToLocalChecked(),\
	FunctionTemplate::New(isolate, fn, Local<v8::Value>(), Local<v8::Signature>(), 0,\
	v8::ConstructorBehavior::kThrow, v8::SideEffectType::kHasNoSideEffect, nullptr))

void plugin_embed(Isolate *_isolate, Local<ObjectTemplate> global) {
	isolate = _isolate;
	Isolate::Scope isolate_scope(isolate);
	HandleScope handle_scope(isolate);

	Local<ObjectTemplate> krom_uv_unwrap = ObjectTemplate::New(isolate);
	SET_FUNCTION(krom_uv_unwrap, "_buffer", krom_uv_unwrap_buffer);
	SET_FUNCTION(krom_uv_unwrap, "_setVertexCount", krom_uv_unwrap_setVertexCount);
	SET_FUNCTION(krom_uv_unwrap, "_setIndexCount", krom_uv_unwrap_setIndexCount);
	SET_FUNCTION(krom_uv_unwrap, "_setPositions", krom_uv_unwrap_setPositions);
	SET_FUNCTION(krom_uv_unwrap, "_setNormals", krom_uv_unwrap_setNormals);
	SET_FUNCTION(krom_uv_unwrap, "_setIndices", krom_uv_unwrap_setIndices);
	SET_FUNCTION(krom_uv_unwrap, "_getVertexCount", krom_uv_unwrap_getVertexCount);
	SET_FUNCTION(krom_uv_unwrap, "_getIndexCount", krom_uv_unwrap_getIndexCount);
	SET_FUNCTION(krom_uv_unwrap, "_getPositions", krom_uv_unwrap_getPositions);
	SET_FUNCTION(krom_uv_unwrap, "_getNormals", krom_uv_unwrap_getNormals);
	SET_FUNCTION(krom_uv_unwrap, "_getUVs", krom_uv_unwrap_getUVs);
	SET_FUNCTION(krom_uv_unwrap, "_getIndices", krom_uv_unwrap_getIndices);
	SET_FUNCTION(krom_uv_unwrap, "_unwrap", krom_uv_unwrap_unwrap);
	SET_FUNCTION(krom_uv_unwrap, "_destroy", krom_uv_unwrap_destroy);
	global->Set(String::NewFromUtf8(isolate, "Krom_uv_unwrap").ToLocalChecked(), krom_uv_unwrap);

	Local<ObjectTemplate> krom_import_svg = ObjectTemplate::New(isolate);
	SET_FUNCTION(krom_import_svg, "_buffer", krom_import_svg_buffer);
	SET_FUNCTION(krom_import_svg, "_init", krom_import_svg_init);
	SET_FUNCTION(krom_import_svg, "_parse", krom_import_svg_parse);
	SET_FUNCTION(krom_import_svg, "_get_pixels_w", krom_import_svg_get_pixels_w);
	SET_FUNCTION(krom_import_svg, "_get_pixels_h", krom_import_svg_get_pixels_h);
	SET_FUNCTION(krom_import_svg, "_get_pixels", krom_import_svg_get_pixels);
	SET_FUNCTION(krom_import_svg, "_destroy", krom_import_svg_destroy);
	global->Set(String::NewFromUtf8(isolate, "Krom_import_svg").ToLocalChecked(), krom_import_svg);

	Local<ObjectTemplate> krom_import_gltf = ObjectTemplate::New(isolate);
	SET_FUNCTION(krom_import_gltf, "_buffer", krom_import_gltf_buffer);
	SET_FUNCTION(krom_import_gltf, "_init", krom_import_gltf_init);
	SET_FUNCTION(krom_import_gltf, "_parse", krom_import_gltf_parse);
	SET_FUNCTION(krom_import_gltf, "_get_vertex_count", krom_import_gltf_get_vertex_count);
	SET_FUNCTION(krom_import_gltf, "_get_index_count", krom_import_gltf_get_index_count);
	SET_FUNCTION(krom_import_gltf, "_get_indices", krom_import_gltf_get_indices);
	SET_FUNCTION(krom_import_gltf, "_get_positions", krom_import_gltf_get_positions);
	SET_FUNCTION(krom_import_gltf, "_get_normals", krom_import_gltf_get_normals);
	SET_FUNCTION(krom_import_gltf, "_get_uvs", krom_import_gltf_get_uvs);
	SET_FUNCTION(krom_import_gltf, "_get_scale_pos", krom_import_gltf_get_scale_pos);
	SET_FUNCTION(krom_import_gltf, "_destroy", krom_import_gltf_destroy);
	global->Set(String::NewFromUtf8(isolate, "Krom_import_gltf").ToLocalChecked(), krom_import_gltf);

	Local<ObjectTemplate> krom_import_usdc = ObjectTemplate::New(isolate);
	SET_FUNCTION(krom_import_usdc, "_buffer", krom_import_usdc_buffer);
	SET_FUNCTION(krom_import_usdc, "_init", krom_import_usdc_init);
	SET_FUNCTION(krom_import_usdc, "_parse", krom_import_usdc_parse);
	SET_FUNCTION(krom_import_usdc, "_get_vertex_count", krom_import_usdc_get_vertex_count);
	SET_FUNCTION(krom_import_usdc, "_get_index_count", krom_import_usdc_get_index_count);
	SET_FUNCTION(krom_import_usdc, "_get_indices", krom_import_usdc_get_indices);
	SET_FUNCTION(krom_import_usdc, "_get_positions", krom_import_usdc_get_positions);
	SET_FUNCTION(krom_import_usdc, "_get_normals", krom_import_usdc_get_normals);
	SET_FUNCTION(krom_import_usdc, "_get_uvs", krom_import_usdc_get_uvs);
	SET_FUNCTION(krom_import_usdc, "_get_scale_pos", krom_import_usdc_get_scale_pos);
	SET_FUNCTION(krom_import_usdc, "_destroy", krom_import_usdc_destroy);
	global->Set(String::NewFromUtf8(isolate, "Krom_import_usdc").ToLocalChecked(), krom_import_usdc);
}
