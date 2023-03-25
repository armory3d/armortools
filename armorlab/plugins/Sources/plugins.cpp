
#include <v8.h>
#include <kinc/log.h>

using namespace v8;

static Isolate *isolate;

extern "C" {
	void texsynth_inpaint(int w, int h, void *output_ptr, void *image_ptr, void *mask_ptr, bool tiling);
}

namespace {
	void krom_texsynth_inpaint(const FunctionCallbackInfo<Value> &args) {
		HandleScope scope(args.GetIsolate());
		int32_t w = args[0]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		int32_t h = args[1]->ToInt32(isolate->GetCurrentContext()).ToLocalChecked()->Value();
		Local<ArrayBuffer> bufferOut = Local<ArrayBuffer>::Cast(args[2]);
		std::shared_ptr<BackingStore> contentOut = bufferOut->GetBackingStore();
		Local<ArrayBuffer> bufferImage = Local<ArrayBuffer>::Cast(args[3]);
		std::shared_ptr<BackingStore> contentImage = bufferImage->GetBackingStore();
		Local<ArrayBuffer> bufferMask = Local<ArrayBuffer>::Cast(args[4]);
		std::shared_ptr<BackingStore> contentMask = bufferMask->GetBackingStore();
		bool tiling = args[5]->ToBoolean(isolate)->Value();
		texsynth_inpaint(w, h, contentOut->Data(), contentImage->Data(), contentMask->Data(), tiling);
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

	Local<ObjectTemplate> krom_texsynth = ObjectTemplate::New(isolate);
	SET_FUNCTION(krom_texsynth, "inpaint", krom_texsynth_inpaint);
	global->Set(String::NewFromUtf8(isolate, "Krom_texsynth").ToLocalChecked(), krom_texsynth);
}
