#pragma once

#include "iron_array.h"
#include "iron_map.h"
#include <stdbool.h>

void      *json_parse(char *s);
any_map_t *json_parse_to_map(char *s);

void  json_encode_begin();
char *json_encode_end();
void  json_encode_string(char *k, char *v);
void  json_encode_string_array(char *k, char_ptr_array_t *a);
void  json_encode_f32(char *k, float f);
void  json_encode_i32(char *k, int i);
void  json_encode_null(char *k);
void  json_encode_f32_array(char *k, f32_array_t *a);
void  json_encode_i32_array(char *k, i32_array_t *a);
void  json_encode_bool(char *k, bool b);
void  json_encode_begin_array(char *k);
void  json_encode_end_array();
void  json_encode_begin_object();
void  json_encode_end_object();
void  json_encode_map(any_map_t *m);

typedef enum {
	JSON_TYPE_UNDEFINED = 0,
	JSON_TYPE_OBJECT    = (1 << 0),
	JSON_TYPE_ARRAY     = (1 << 1),
	JSON_TYPE_STRING    = (1 << 2),
	JSON_TYPE_NUMBER    = (1 << 3),
	JSON_TYPE_BOOL      = (1 << 4),
	JSON_TYPE_NULL      = (1 << 5),
} _json_type_t;

typedef struct json_object {
	_json_type_t type;
	int32_t      index;
	void        *data;
} json_object_t;

json_object_t *json_decode(const char *const s);
json_object_t *json_decode_object_value(const char *const s, json_object_t *o, const char *name);
json_object_t *json_decode_array_value(const char *const s, json_object_t *o, uint32_t index);
char          *json_decode_string_value(const char *const s, json_object_t *o);
float          json_decode_number_value(const char *const s, json_object_t *o);
bool           json_decode_bool_value(const char *const s, json_object_t *o);
