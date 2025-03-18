#ifndef KONG_ARRAY_HEADER
#define KONG_ARRAY_HEADER

#define static_array(type, name, max_size) \
	typedef struct name {                  \
		type   values[max_size];           \
		size_t size;                       \
		size_t max;                        \
	} name;

/*#define static_array_with_init(type, name, max_size) \
    struct {                                                                                                                                                   \
        type values[max_size];                                                                                                                                 \
        size_t size;                                                                                                                                           \
        size_t max;                                                                                                                                            \
    } name;                                                                                                                                                    \
    name.size = 0;                                                                                                                                             \
    name.max = max_size;*/

#define static_array_init(array) \
	array.size = 0;              \
	array.max  = sizeof(array.values) / sizeof(array.values[0])

#define static_array_push(array, value)   \
	if (array.size >= array.max) {        \
		debug_context context = {0};      \
		error(context, "Array overflow"); \
	}                                     \
	array.values[array.size++] = value;

#define static_array_push_p(array, value) \
	if (array->size >= array->max) {      \
		debug_context context = {0};      \
		error(context, "Array overflow"); \
	}                                     \
	array->values[array->size++] = value;

#endif
