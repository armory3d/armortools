#pragma once

#include <stdbool.h>
#include <stdint.h>

#define MINIC_MEM_SIZE          (8 * 1024 * 1024)
#define MINIC_MAX_PARAMS        20
#define MINIC_MAX_EXTFUNS       1024
#define MINIC_MAX_SIG           64
#define MINIC_MAX_ENUM_CONSTS   512
#define MINIC_MAX_INT_TYPEDEFS  128
#define MINIC_MAX_STRUCT_FIELDS 16
#define MINIC_MAX_STRUCTS       32

typedef unsigned char minic_u8;

typedef enum {
	MINIC_T_INT   = 0,
	MINIC_T_FLOAT = 1,
	MINIC_T_PTR   = 2, // void *, always holds a real host pointer
	MINIC_T_BOOL  = 3, // used in extern-call ABI, stored as INT in vals
	MINIC_T_CHAR  = 4, // used in extern-call ABI, stored as INT in vals
} minic_type_t;

typedef struct {
	minic_type_t type;
	minic_type_t deref_type; // pointed-to type (for MINIC_T_PTR)
	union {
		int   i; // MINIC_T_INT
		float f; // MINIC_T_FLOAT
		void *p; // MINIC_T_PTR
	};
} minic_val_t;

typedef struct {
	char name[64];
	int  value;
} minic_enum_const_t;
typedef struct {
	char name[64];
} minic_int_typedef_t;

typedef struct {
	char         name[64];
	char         fields[MINIC_MAX_STRUCT_FIELDS][64];
	int          field_count;
	bool         has_native_layout;
	int          field_offsets[MINIC_MAX_STRUCT_FIELDS]; // byte offset of each field in the native C struct
	minic_type_t field_native_types[MINIC_MAX_STRUCT_FIELDS];
	minic_type_t field_deref_types[MINIC_MAX_STRUCT_FIELDS]; // pointed-to type for PTR fields
} minic_global_struct_t;

typedef void (*minic_ext_fn_raw_t)(void); // type-erased

typedef struct minic_ctx_s minic_ctx_t;

typedef minic_val_t (*minic_native_fn_t)(minic_val_t *args, int argc);

typedef struct {
	char               name[64];
	char               sig[MINIC_MAX_SIG];
	minic_ext_fn_raw_t fn;
	minic_native_fn_t  native_fn; // if non-NULL, bypasses typed dispatch
	minic_type_t       ret_type;
	minic_type_t       param_types[MINIC_MAX_PARAMS];
	int                param_count;
} minic_ext_func_t;

extern minic_enum_const_t    minic_enum_consts[MINIC_MAX_ENUM_CONSTS];
extern int                   minic_enum_const_count;
extern minic_int_typedef_t   minic_int_typedefs[MINIC_MAX_INT_TYPEDEFS];
extern int                   minic_int_typedef_count;
extern minic_global_struct_t minic_global_structs[MINIC_MAX_STRUCTS];
extern int                   minic_global_struct_count;

bool              minic_is_int_typedef(const char *name);
int               minic_enum_const_get(const char *name);
minic_ext_func_t *minic_ext_func_get(const char *name);
minic_val_t       minic_dispatch(minic_ext_func_t *ef, minic_val_t *args, int argc);
void minic_register_struct_native(const char *name, const char **fields, const int *offsets, const minic_type_t *types, const minic_type_t *deref_types,
                                  int field_count);

void        *minic_alloc(int size);
minic_ctx_t *minic_eval(const char *src);
void         minic_ctx_free(minic_ctx_t *ctx);
float        minic_ctx_result(minic_ctx_t *ctx);
minic_val_t  minic_ctx_call_fn(minic_ctx_t *ctx, void *fn_ptr, minic_val_t *args, int argc);
void         minic_register(const char *name, const char *sig, minic_ext_fn_raw_t fn);
void         minic_register_native(const char *name, minic_native_fn_t fn);
void         minic_register_struct(const char *name, const char **fields, int field_count);
void         minic_register_enum(const char *typedef_name, const char **names, const int *values, int count);
void         minic_register_builtins(void);
// Call a minic function from native C. fn_ptr is a minic_func_t* passed from a script.
// Valid as long as the owning minic_ctx_t has not been freed
minic_val_t minic_call_fn(void *fn_ptr, minic_val_t *args, int argc);

static inline minic_val_t minic_val_int(int v) {
	minic_val_t r;
	r.type       = MINIC_T_INT;
	r.deref_type = MINIC_T_INT;
	r.i          = v;
	return r;
}

static inline minic_val_t minic_val_float(float v) {
	minic_val_t r;
	r.type       = MINIC_T_FLOAT;
	r.deref_type = MINIC_T_FLOAT;
	r.f          = v;
	return r;
}

static inline minic_val_t minic_val_ptr(void *v) {
	minic_val_t r;
	r.type       = MINIC_T_PTR;
	r.deref_type = MINIC_T_PTR;
	r.p          = v;
	return r;
}

static inline minic_val_t minic_val_typed_ptr(void *v, minic_type_t deref_type) {
	minic_val_t r;
	r.type       = MINIC_T_PTR;
	r.deref_type = deref_type;
	r.p          = v;
	return r;
}

static inline minic_val_t minic_val_void(void) {
	minic_val_t r;
	r.type       = MINIC_T_INT;
	r.deref_type = MINIC_T_INT;
	r.i          = 0;
	return r;
}

static inline double minic_val_to_d(minic_val_t v) {
	switch (v.type) {
	case MINIC_T_INT:
	case MINIC_T_BOOL:
	case MINIC_T_CHAR:
		return (double)v.i;
	case MINIC_T_FLOAT:
		return (double)v.f;
	case MINIC_T_PTR:
		return (double)(uintptr_t)v.p;
	default:
		return 0.0;
	}
}

static inline void *minic_val_to_ptr(minic_val_t v) {
	return (v.type == MINIC_T_PTR) ? v.p : (void *)(uintptr_t)(uint64_t)minic_val_to_d(v);
}

static inline int minic_val_is_true(minic_val_t v) {
	return minic_val_to_d(v) != 0.0;
}

static inline minic_val_t minic_val_coerce(double d, minic_type_t t) {
	switch (t) {
	case MINIC_T_BOOL:
	case MINIC_T_CHAR:
	case MINIC_T_INT:
	default: {
		minic_val_t r;
		r.type       = MINIC_T_INT;
		r.deref_type = MINIC_T_INT;
		r.i          = (int)d;
		return r;
	}
	case MINIC_T_FLOAT: {
		minic_val_t r;
		r.type       = MINIC_T_FLOAT;
		r.deref_type = MINIC_T_FLOAT;
		r.f          = (float)d;
		return r;
	}
	case MINIC_T_PTR: {
		minic_val_t r;
		r.type       = MINIC_T_PTR;
		r.deref_type = MINIC_T_PTR;
		r.p          = (void *)(uintptr_t)(uint64_t)d;
		return r;
	}
	}
}

static inline minic_val_t minic_val_cast(minic_val_t v, minic_type_t t) {
	if (v.type == t)
		return v;
	return minic_val_coerce(minic_val_to_d(v), t);
}

void minic_tests();
