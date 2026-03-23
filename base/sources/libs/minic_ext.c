
#include "minic.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

// ███████╗██╗  ██╗████████╗███████╗██████╗ ███╗   ██╗ █████╗ ██╗
// ██╔════╝╚██╗██╔╝╚══██╔══╝██╔════╝██╔══██╗████╗  ██║██╔══██╗██║
// █████╗   ╚███╔╝    ██║   █████╗  ██████╔╝██╔██╗ ██║███████║██║
// ██╔══╝   ██╔██╗    ██║   ██╔══╝  ██╔══██╗██║╚██╗██║██╔══██║██║
// ███████╗██╔╝ ██╗   ██║   ███████╗██║  ██║██║ ╚████║██║  ██║███████╗
// ╚══════╝╚═╝  ╚═╝   ╚═╝   ╚══════╝╚═╝  ╚═╝╚═╝  ╚═══╝╚═╝  ╚═╝╚══════╝

static minic_ext_func_t minic_ext_funcs[MINIC_MAX_EXTFUNS];
static int              minic_ext_func_count = 0;

minic_enum_const_t  minic_enum_consts[MINIC_MAX_ENUM_CONSTS];
int                 minic_enum_const_count = 0;
minic_int_typedef_t minic_int_typedefs[MINIC_MAX_INT_TYPEDEFS];
int                 minic_int_typedef_count = 0;

minic_global_struct_t minic_global_structs[MINIC_MAX_STRUCTS];
int                   minic_global_struct_count = 0;

void minic_register_struct_native(const char *name, const char **fields, const int *offsets, const minic_type_t *types, const minic_type_t *deref_types,
                                  int field_count) {
	minic_global_struct_t *def = NULL;
	for (int i = 0; i < minic_global_struct_count; ++i) {
		if (strcmp(minic_global_structs[i].name, name) == 0) {
			def = &minic_global_structs[i];
			break;
		}
	}
	if (def == NULL) {
		if (minic_global_struct_count >= MINIC_MAX_STRUCTS)
			return;
		def = &minic_global_structs[minic_global_struct_count++];
	}
	strncpy(def->name, name, 63);
	def->field_count       = field_count < MINIC_MAX_STRUCT_FIELDS ? field_count : MINIC_MAX_STRUCT_FIELDS;
	def->has_native_layout = true;
	for (int i = 0; i < def->field_count; ++i) {
		strncpy(def->fields[i], fields[i], 63);
		def->field_offsets[i]      = offsets[i];
		def->field_native_types[i] = types[i];
		def->field_deref_types[i]  = (deref_types != NULL) ? deref_types[i] : types[i];
	}
}

void minic_register_struct(const char *name, const char **fields, int field_count) {
	for (int i = 0; i < minic_global_struct_count; ++i) {
		if (strcmp(minic_global_structs[i].name, name) == 0)
			return;
	}
	if (minic_global_struct_count >= MINIC_MAX_STRUCTS)
		return;
	minic_global_struct_t *def = &minic_global_structs[minic_global_struct_count++];
	strncpy(def->name, name, 63);
	def->field_count = field_count < MINIC_MAX_STRUCT_FIELDS ? field_count : MINIC_MAX_STRUCT_FIELDS;
	for (int i = 0; i < def->field_count; ++i) {
		strncpy(def->fields[i], fields[i], 63);
	}
}

void minic_register_enum(const char *typedef_name, const char **names, const int *values, int count) {
	if (typedef_name != NULL) {
		bool found = false;
		for (int i = 0; i < minic_int_typedef_count; ++i)
			if (strcmp(minic_int_typedefs[i].name, typedef_name) == 0) {
				found = true;
				break;
			}
		if (!found && minic_int_typedef_count < MINIC_MAX_INT_TYPEDEFS)
			strncpy(minic_int_typedefs[minic_int_typedef_count++].name, typedef_name, 63);
	}
	for (int i = 0; i < count; ++i) {
		bool found = false;
		for (int j = 0; j < minic_enum_const_count; ++j)
			if (strcmp(minic_enum_consts[j].name, names[i]) == 0) {
				found = true;
				break;
			}
		if (!found && minic_enum_const_count < MINIC_MAX_ENUM_CONSTS) {
			strncpy(minic_enum_consts[minic_enum_const_count].name, names[i], 63);
			minic_enum_consts[minic_enum_const_count].value = values[i];
			minic_enum_const_count++;
		}
	}
}

bool minic_is_int_typedef(const char *name) {
	for (int i = 0; i < minic_int_typedef_count; ++i)
		if (strcmp(minic_int_typedefs[i].name, name) == 0)
			return true;
	return false;
}

int minic_enum_const_get(const char *name) {
	for (int i = 0; i < minic_enum_const_count; ++i)
		if (strcmp(minic_enum_consts[i].name, name) == 0)
			return minic_enum_consts[i].value;
	return -1;
}

static minic_type_t minic_sig_char(char c) {
	switch (c) {
	case 'i':
		return MINIC_T_INT;
	case 'f':
		return MINIC_T_FLOAT;
	case 'p':
		return MINIC_T_PTR;
	case 'b':
		return MINIC_T_BOOL;
	case 'c':
		return MINIC_T_CHAR;
	default:
		return MINIC_T_INT; // 'v' → INT/0 for void
	}
}

static void minic_parse_sig(minic_ext_func_t *ef) {
	const char *s = ef->sig;
	ef->ret_type  = minic_sig_char(*s);
	if (*s)
		s++; // skip ret type char
	if (*s == '(')
		s++; // skip '('
	ef->param_count = 0;
	while (*s && *s != ')') {
		if (*s != ',') {
			if (ef->param_count < MINIC_MAX_PARAMS) {
				ef->param_types[ef->param_count++] = minic_sig_char(*s);
			}
		}
		s++;
	}
}

void minic_register(const char *name, const char *sig, minic_ext_fn_raw_t fn) {
	// Check for existing entry with same name
	for (int i = 0; i < minic_ext_func_count; ++i) {
		if (strcmp(minic_ext_funcs[i].name, name) == 0) {
			// Update in-place
			strncpy(minic_ext_funcs[i].sig, sig ? sig : "d()", MINIC_MAX_SIG - 1);
			minic_ext_funcs[i].fn = fn;
			minic_parse_sig(&minic_ext_funcs[i]);
			return;
		}
	}
	if (minic_ext_func_count >= MINIC_MAX_EXTFUNS) {
		return;
	}
	minic_ext_func_t *ef = &minic_ext_funcs[minic_ext_func_count++];
	strncpy(ef->name, name, 63);
	strncpy(ef->sig, sig ? sig : "d()", MINIC_MAX_SIG - 1);
	ef->fn = fn;
	minic_parse_sig(ef);
}

void minic_register_native(const char *name, minic_native_fn_t fn) {
	for (int i = 0; i < minic_ext_func_count; ++i) {
		if (strcmp(minic_ext_funcs[i].name, name) == 0) {
			minic_ext_funcs[i].native_fn = fn;
			return;
		}
	}
	if (minic_ext_func_count >= MINIC_MAX_EXTFUNS) {
		return;
	}
	minic_ext_func_t *ef = &minic_ext_funcs[minic_ext_func_count++];
	strncpy(ef->name, name, 63);
	ef->native_fn = fn;
}

minic_ext_func_t *minic_ext_func_get(const char *name) {
	for (int i = 0; i < minic_ext_func_count; ++i) {
		if (strcmp(minic_ext_funcs[i].name, name) == 0) {
			return &minic_ext_funcs[i];
		}
	}
	return NULL;
}

typedef union {
	int   i;
	float f;
	void *p;
} minic_arg_u;

minic_val_t minic_dispatch(minic_ext_func_t *ef, minic_val_t *args, int argc) {
	if (ef->native_fn) {
		return ef->native_fn(args, argc);
	}
	// Build typed argument array
	minic_arg_u a[MINIC_MAX_PARAMS] = {0};
	int         n                   = argc < ef->param_count ? argc : ef->param_count;
	for (int i = 0; i < n; i++) {
		minic_type_t pt = ef->param_types[i];
		double       dv = minic_val_to_d(args[i]);
		switch (pt) {
		case MINIC_T_BOOL:
		case MINIC_T_CHAR:
		case MINIC_T_INT:
			a[i].i = (int)dv;
			break;
		case MINIC_T_FLOAT:
			a[i].f = (float)dv;
			break;
		case MINIC_T_PTR:
			if (args[i].type == MINIC_T_PTR) {
				a[i].p = args[i].p;
			}
			else {
				a[i].p = (dv == 0.0) ? NULL : (void *)(uintptr_t)(uint64_t)dv;
			}
			break;
		}
	}

	// Helper macros to extract args by declared type
#define A0i a[0].i
#define A1i a[1].i
#define A2i a[2].i
#define A3i a[3].i
#define A4i a[4].i
#define A5i a[5].i
#define A6i a[6].i
#define A7i a[7].i
#define A0f a[0].f
#define A1f a[1].f
#define A2f a[2].f
#define A3f a[3].f
#define A0p a[0].p
#define A1p a[1].p
#define A2p a[2].p
#define A3p a[3].p
#define A4p a[4].p

	// Build an argument descriptor string so we can dispatch concisely
	char adesc[MINIC_MAX_PARAMS + 1];
	for (int i = 0; i < n; i++) {
		switch (ef->param_types[i]) {
		case MINIC_T_FLOAT:
			adesc[i] = 'f';
			break;
		case MINIC_T_PTR:
			adesc[i] = 'p';
			break;
		default:
			adesc[i] = 'i';
			break;
		}
	}
	adesc[n] = '\0';

	if (ef->ret_type == MINIC_T_INT) {
		typedef int (*fn_v)(void);
		typedef int (*fn_i)(int);
		typedef int (*fn_ii)(int, int);
		typedef int (*fn_iii)(int, int, int);
		typedef int (*fn_iiii)(int, int, int, int);
		typedef int (*fn_p)(void *);
		typedef int (*fn_pi)(void *, int);
		typedef int (*fn_pii)(void *, int, int);
		typedef int (*fn_pp)(void *, void *);
		typedef int (*fn_ppi)(void *, void *, int);
		typedef int (*fn_pip)(void *, int, void *);
		typedef int (*fn_pf)(void *, float);
		typedef int (*fn_f)(float);
		typedef int (*fn_ff)(float, float);
		typedef int (*fn_pppp)(void *, void *, void *, void *);
		typedef int (*fn_ppp)(void *, void *, void *);
		typedef int (*fn_ip)(int, void *);
		typedef int (*fn_ppffff)(void *, void *, float, float, float, float);
		int r = 0;
		if (strcmp(adesc, "") == 0)
			r = ((fn_v)ef->fn)();
		else if (strcmp(adesc, "i") == 0)
			r = ((fn_i)ef->fn)(A0i);
		else if (strcmp(adesc, "ii") == 0)
			r = ((fn_ii)ef->fn)(A0i, A1i);
		else if (strcmp(adesc, "iii") == 0)
			r = ((fn_iii)ef->fn)(A0i, A1i, A2i);
		else if (strcmp(adesc, "iiii") == 0)
			r = ((fn_iiii)ef->fn)(A0i, A1i, A2i, A3i);
		else if (strcmp(adesc, "p") == 0)
			r = ((fn_p)ef->fn)(A0p);
		else if (strcmp(adesc, "pi") == 0)
			r = ((fn_pi)ef->fn)(A0p, A1i);
		else if (strcmp(adesc, "pii") == 0)
			r = ((fn_pii)ef->fn)(A0p, A1i, A2i);
		else if (strcmp(adesc, "pp") == 0)
			r = ((fn_pp)ef->fn)(A0p, A1p);
		else if (strcmp(adesc, "ppi") == 0)
			r = ((fn_ppi)ef->fn)(A0p, A1p, A2i);
		else if (strcmp(adesc, "pip") == 0)
			r = ((fn_pip)ef->fn)(A0p, A1i, A2p);
		else if (strcmp(adesc, "pf") == 0)
			r = ((fn_pf)ef->fn)(A0p, A1f);
		else if (strcmp(adesc, "f") == 0)
			r = ((fn_f)ef->fn)(A0f);
		else if (strcmp(adesc, "ff") == 0)
			r = ((fn_ff)ef->fn)(A0f, A1f);
		else if (strcmp(adesc, "pppp") == 0)
			r = ((fn_pppp)ef->fn)(A0p, A1p, A2p, A3p);
		else if (strcmp(adesc, "ppp") == 0)
			r = ((fn_ppp)ef->fn)(A0p, A1p, A2p);
		else if (strcmp(adesc, "ip") == 0)
			r = ((fn_ip)ef->fn)(A0i, A1p);
		else if (strcmp(adesc, "ppffff") == 0)
			r = ((fn_ppffff)ef->fn)(A0p, A1p, A2f, A3f, a[4].f, a[5].f);
		else if (strcmp(adesc, "pppiii") == 0) {
			typedef int (*fn_pppiii)(void *, void *, void *, int, int, int);
			r = ((fn_pppiii)ef->fn)(A0p, A1p, A2p, A3i, a[4].i, a[5].i);
		}
		else if (strcmp(adesc, "piip") == 0) {
			typedef int (*fn_piip)(void *, int, int, void *);
			r = ((fn_piip)ef->fn)(A0p, A1i, A2i, A3p);
		}
		else {
			fprintf(stderr, "minic: unsupported signature '%s' for int-returning '%s'\n", ef->sig, ef->name);
		}
		return minic_val_int(r);
	}

	if (ef->ret_type == MINIC_T_BOOL) {
		typedef bool (*fn_v)(void);
		typedef bool (*fn_p)(void *);
		typedef bool (*fn_i)(int);
		typedef bool (*fn_pi)(void *, int);
		typedef bool (*fn_pii)(void *, int, int);
		typedef bool (*fn_pip)(void *, int, void *);
		typedef bool (*fn_pp)(void *, void *);
		typedef bool (*fn_ppi)(void *, void *, int);
		typedef bool (*fn_ppp)(void *, void *, void *);
		typedef bool (*fn_ppii)(void *, void *, int, int);
		typedef bool (*fn_ppiii)(void *, void *, int, int, int);
		typedef bool (*fn_pipp)(void *, int, void *, void *);
		typedef bool (*fn_piiiii)(void *, int, int, int, int, int);
		bool r = false;
		if (strcmp(adesc, "") == 0)
			r = ((fn_v)ef->fn)();
		else if (strcmp(adesc, "p") == 0)
			r = ((fn_p)ef->fn)(A0p);
		else if (strcmp(adesc, "i") == 0)
			r = ((fn_i)ef->fn)(A0i);
		else if (strcmp(adesc, "pi") == 0)
			r = ((fn_pi)ef->fn)(A0p, A1i);
		else if (strcmp(adesc, "pii") == 0)
			r = ((fn_pii)ef->fn)(A0p, A1i, A2i);
		else if (strcmp(adesc, "pip") == 0)
			r = ((fn_pip)ef->fn)(A0p, A1i, A2p);
		else if (strcmp(adesc, "pp") == 0)
			r = ((fn_pp)ef->fn)(A0p, A1p);
		else if (strcmp(adesc, "ppi") == 0)
			r = ((fn_ppi)ef->fn)(A0p, A1p, A2i);
		else if (strcmp(adesc, "ppp") == 0)
			r = ((fn_ppp)ef->fn)(A0p, A1p, A2p);
		else if (strcmp(adesc, "ppii") == 0)
			r = ((fn_ppii)ef->fn)(A0p, A1p, A2i, A3i);
		else if (strcmp(adesc, "ppiii") == 0)
			r = ((fn_ppiii)ef->fn)(A0p, A1p, A2i, A3i, a[4].i);
		else if (strcmp(adesc, "pipp") == 0)
			r = ((fn_pipp)ef->fn)(A0p, A1i, A2p, A3p);
		else if (strcmp(adesc, "piiiii") == 0)
			r = ((fn_piiiii)ef->fn)(A0p, A1i, A2i, A3i, A4i, A5i);
		else {
			fprintf(stderr, "minic: unsupported signature '%s' for bool-returning '%s'\n", ef->sig, ef->name);
		}
		return minic_val_int((int)r);
	}

	if (ef->ret_type == MINIC_T_CHAR) {
		typedef char (*fn_v)(void);
		typedef char (*fn_p)(void *);
		typedef char (*fn_i)(int);
		typedef char (*fn_pi)(void *, int);
		typedef char (*fn_pp)(void *, void *);
		char r = 0;
		if (strcmp(adesc, "") == 0)
			r = ((fn_v)ef->fn)();
		else if (strcmp(adesc, "p") == 0)
			r = ((fn_p)ef->fn)(A0p);
		else if (strcmp(adesc, "i") == 0)
			r = ((fn_i)ef->fn)(A0i);
		else if (strcmp(adesc, "pi") == 0)
			r = ((fn_pi)ef->fn)(A0p, A1i);
		else if (strcmp(adesc, "pp") == 0)
			r = ((fn_pp)ef->fn)(A0p, A1p);
		else {
			fprintf(stderr, "minic: unsupported signature '%s' for char-returning '%s'\n", ef->sig, ef->name);
		}
		return minic_val_int((int)r);
	}

	if (ef->ret_type == MINIC_T_FLOAT) {
		typedef float (*fn_v)(void);
		typedef float (*fn_f)(float);
		typedef float (*fn_ff)(float, float);
		typedef float (*fn_fff)(float, float, float);
		typedef float (*fn_ffff)(float, float, float, float);
		typedef float (*fn_fffff)(float, float, float, float, float);
		typedef float (*fn_ffffff)(float, float, float, float, float, float);
		typedef float (*fn_fffffff)(float, float, float, float, float, float, float);
		typedef float (*fn_i)(int);
		typedef float (*fn_p)(void *);
		typedef float (*fn_pi)(void *, int);
		typedef float (*fn_pf)(void *, float);
		typedef float (*fn_pff)(void *, float, float);
		typedef float (*fn_pfff)(void *, float, float, float);
		typedef float (*fn_pffff)(void *, float, float, float, float);
		typedef float (*fn_pfffff)(void *, float, float, float, float, float);
		typedef float (*fn_pffffff)(void *, float, float, float, float, float, float);
		typedef float (*fn_pfffffffx)(void *, float, float, float, float, float, float, float);
		float r = 0.0f;
		if (strcmp(adesc, "") == 0)
			r = ((fn_v)ef->fn)();
		else if (strcmp(adesc, "f") == 0)
			r = ((fn_f)ef->fn)(A0f);
		else if (strcmp(adesc, "ff") == 0)
			r = ((fn_ff)ef->fn)(A0f, A1f);
		else if (strcmp(adesc, "fff") == 0)
			r = ((fn_fff)ef->fn)(A0f, A1f, A2f);
		else if (strcmp(adesc, "ffff") == 0)
			r = ((fn_ffff)ef->fn)(A0f, A1f, A2f, A3f);
		else if (strcmp(adesc, "fffff") == 0)
			r = ((fn_fffff)ef->fn)(A0f, A1f, A2f, A3f, a[4].f);
		else if (strcmp(adesc, "ffffff") == 0)
			r = ((fn_ffffff)ef->fn)(A0f, A1f, A2f, A3f, a[4].f, a[5].f);
		else if (strcmp(adesc, "fffffff") == 0)
			r = ((fn_fffffff)ef->fn)(A0f, A1f, A2f, A3f, a[4].f, a[5].f, a[6].f);
		else if (strcmp(adesc, "i") == 0)
			r = ((fn_i)ef->fn)(A0i);
		else if (strcmp(adesc, "p") == 0)
			r = ((fn_p)ef->fn)(A0p);
		else if (strcmp(adesc, "pi") == 0)
			r = ((fn_pi)ef->fn)(A0p, A1i);
		else if (strcmp(adesc, "pf") == 0)
			r = ((fn_pf)ef->fn)(A0p, A1f);
		else if (strcmp(adesc, "pff") == 0)
			r = ((fn_pff)ef->fn)(A0p, A1f, A2f);
		else if (strcmp(adesc, "pfff") == 0)
			r = ((fn_pfff)ef->fn)(A0p, A1f, A2f, A3f);
		else if (strcmp(adesc, "pffff") == 0)
			r = ((fn_pffff)ef->fn)(A0p, A1f, A2f, A3f, a[4].f);
		else if (strcmp(adesc, "pfffff") == 0)
			r = ((fn_pfffff)ef->fn)(A0p, A1f, A2f, A3f, a[4].f, a[5].f);
		else if (strcmp(adesc, "pffffff") == 0)
			r = ((fn_pffffff)ef->fn)(A0p, A1f, A2f, A3f, a[4].f, a[5].f, a[6].f);
		else if (strcmp(adesc, "pfffffffx") == 0)
			r = ((fn_pfffffffx)ef->fn)(A0p, A1f, A2f, A3f, a[4].f, a[5].f, a[6].f, a[7].f);
		else if (strcmp(adesc, "ppffifiii") == 0) {
			typedef float (*fn_ppffifiii)(void *, void *, float, float, int, float, int, int, int);
			r = ((fn_ppffifiii)ef->fn)(A0p, A1p, A2f, A3f, a[4].i, a[5].f, a[6].i, a[7].i, a[8].i);
		}
		else {
			fprintf(stderr, "minic: unsupported signature '%s' for float-returning '%s'\n", ef->sig, ef->name);
		}
		return minic_val_float(r);
	}

	if (ef->ret_type == MINIC_T_PTR) {
		typedef void *(*fn_v)(void);
		typedef void *(*fn_p)(void *);
		typedef void *(*fn_pp)(void *, void *);
		typedef void *(*fn_ppp)(void *, void *, void *);
		typedef void *(*fn_pi)(void *, int);
		typedef void *(*fn_pii)(void *, int, int);
		typedef void *(*fn_pppi)(void *, void *, void *, int);
		typedef void *(*fn_i)(int);
		typedef void *(*fn_ii)(int, int);
		typedef void *(*fn_ppi)(void *, void *, int);
		typedef void *(*fn_pppp)(void *, void *, void *, void *);
		typedef void *(*fn_ppppp)(void *, void *, void *, void *, void *);
		typedef void *(*fn_ppiii)(void *, void *, int, int, int);
		void *r = NULL;
		if (strcmp(adesc, "") == 0)
			r = ((fn_v)ef->fn)();
		else if (strcmp(adesc, "p") == 0)
			r = ((fn_p)ef->fn)(A0p);
		else if (strcmp(adesc, "pp") == 0)
			r = ((fn_pp)ef->fn)(A0p, A1p);
		else if (strcmp(adesc, "ppp") == 0)
			r = ((fn_ppp)ef->fn)(A0p, A1p, A2p);
		else if (strcmp(adesc, "pi") == 0)
			r = ((fn_pi)ef->fn)(A0p, A1i);
		else if (strcmp(adesc, "pii") == 0)
			r = ((fn_pii)ef->fn)(A0p, A1i, A2i);
		else if (strcmp(adesc, "pppi") == 0)
			r = ((fn_pppi)ef->fn)(A0p, A1p, A2p, A3i);
		else if (strcmp(adesc, "i") == 0)
			r = ((fn_i)ef->fn)(A0i);
		else if (strcmp(adesc, "ii") == 0)
			r = ((fn_ii)ef->fn)(A0i, A1i);
		else if (strcmp(adesc, "ppi") == 0)
			r = ((fn_ppi)ef->fn)(A0p, A1p, A2i);
		else if (strcmp(adesc, "pppp") == 0)
			r = ((fn_pppp)ef->fn)(A0p, A1p, A2p, A3p);
		else if (strcmp(adesc, "ppppp") == 0)
			r = ((fn_ppppp)ef->fn)(A0p, A1p, A2p, A3p, A4p);
		else if (strcmp(adesc, "ppiii") == 0)
			r = ((fn_ppiii)ef->fn)(A0p, A1p, A2i, A3i, A4i);
		else if (strcmp(adesc, "ppppf") == 0) {
			typedef void *(*fn_ppppf)(void *, void *, void *, void *, float);
			r = ((fn_ppppf)ef->fn)(A0p, A1p, A2p, A3p, a[4].f);
		}
		else if (strcmp(adesc, "f") == 0) {
			typedef void *(*fn_f)(float);
			r = ((fn_f)ef->fn)(a[0].f);
		}
		else if (strcmp(adesc, "ff") == 0) {
			typedef void *(*fn_ff)(float, float);
			r = ((fn_ff)ef->fn)(a[0].f, a[1].f);
		}
		else if (strcmp(adesc, "fff") == 0) {
			typedef void *(*fn_fff)(float, float, float);
			r = ((fn_fff)ef->fn)(a[0].f, a[1].f, a[2].f);
		}
		else if (strcmp(adesc, "ffff") == 0) {
			typedef void *(*fn_ffff)(float, float, float, float);
			r = ((fn_ffff)ef->fn)(a[0].f, a[1].f, a[2].f, a[3].f);
		}
		else if (strcmp(adesc, "fffff") == 0) {
			typedef void *(*fn_fffff)(float, float, float, float, float);
			r = ((fn_fffff)ef->fn)(a[0].f, a[1].f, a[2].f, a[3].f, a[4].f);
		}
		else {
			fprintf(stderr, "minic: unsupported signature '%s' for ptr-returning '%s'\n", ef->sig, ef->name);
		}
		return minic_val_ptr(r);
	}

	return minic_val_int(0);

#undef A0i
#undef A1i
#undef A2i
#undef A3i
#undef A4i
#undef A5i
#undef A6i
#undef A7i
#undef A0f
#undef A1f
#undef A2f
#undef A3f
#undef A0p
#undef A1p
#undef A2p
#undef A3p
#undef A4p
}
