#include "transformer.h"

#include "compiler.h"
#include "functions.h"
#include "types.h"

#include <assert.h>
#include <string.h>

opcodes new_code;

static void copy_opcode(opcode *o) {
	uint8_t *new_data = &new_code.o[new_code.size];

	assert(new_code.size + o->size < OPCODES_SIZE);

	memcpy(new_data, o, o->size);

	new_code.size += o->size;
}

void transform(uint32_t flags) {
	for (function_id i = 0; get_function(i) != NULL; ++i) {
		function *f = get_function(i);

		if (f->block == NULL) {
			continue;
		}

		uint8_t *data = f->code.o;
		size_t   size = f->code.size;

		new_code.size = 0;

		size_t index = 0;
		while (index < size) {
			opcode *o = (opcode *)&data[index];

			switch (o->type) {
			case OPCODE_STORE_ACCESS_LIST: {
				access a = o->op_store_access_list.access_list[o->op_store_access_list.access_list_size - 1];

				if ((flags & TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE) != 0 && a.kind == ACCESS_SWIZZLE && a.access_swizzle.swizzle.size > 1) {
					assert(is_vector(o->op_store_access_list.from.type.type));

					type_id from_base_type = vector_base_type(o->op_store_access_list.from.type.type);

					type_ref from_base_type_ref;
					init_type_ref(&from_base_type_ref, NO_NAME);
					from_base_type_ref.type = from_base_type;

					for (uint32_t swizzle_index = 0; swizzle_index < a.access_swizzle.swizzle.size; ++swizzle_index) {
						variable from = allocate_variable(from_base_type_ref, o->op_store_access_list.from.kind);

						opcode from_opcode = {
						    .type = OPCODE_LOAD_ACCESS_LIST,
						    .op_load_access_list =
						        {
						            .from             = o->op_store_access_list.from,
						            .to               = from,
						            .access_list_size = 1,
						            .access_list      = {{
						                     .type = from_base_type,
						                     .kind = ACCESS_SWIZZLE,
						                     .access_swizzle =
                                            {
						                             .swizzle =
                                                    {
						                                     .size    = 1,
						                                     .indices = {swizzle_index},
                                                    },
                                            },
                                    }},
						        },
						};
						from_opcode.size = OP_SIZE(from_opcode, op_load_access_list);

						copy_opcode(&from_opcode);

						opcode new_opcode = *o;

						new_opcode.op_store_access_list.from = from;

						access *new_access = &new_opcode.op_store_access_list.access_list[new_opcode.op_store_access_list.access_list_size - 1];
						new_access->access_swizzle.swizzle.size       = 1;
						new_access->access_swizzle.swizzle.indices[0] = a.access_swizzle.swizzle.indices[swizzle_index];
						new_access->type                              = from_base_type;

						copy_opcode(&new_opcode);
					}
				}
				else {
					copy_opcode(o);
				}
				break;
			}
			case OPCODE_LOAD_ACCESS_LIST: {
				access a = o->op_load_access_list.access_list[o->op_load_access_list.access_list_size - 1];

				if ((flags & TRANSFORM_FLAG_ONE_COMPONENT_SWIZZLE) != 0 && a.kind == ACCESS_SWIZZLE && a.access_swizzle.swizzle.size > 1) {
					assert(is_vector(o->op_load_access_list.to.type.type));

					type_id to_type = vector_base_type(o->op_load_access_list.to.type.type);

					type_ref t;
					init_type_ref(&t, NO_NAME);
					t.type = to_type;

					variable to[4];

					for (uint32_t swizzle_index = 0; swizzle_index < a.access_swizzle.swizzle.size; ++swizzle_index) {
						to[swizzle_index] = allocate_variable(t, o->op_load_access_list.to.kind);

						opcode new_opcode = *o;

						new_opcode.op_load_access_list.to = to[swizzle_index];

						access *new_access = &new_opcode.op_load_access_list.access_list[new_opcode.op_load_access_list.access_list_size - 1];
						new_access->access_swizzle.swizzle.size       = 1;
						new_access->access_swizzle.swizzle.indices[0] = a.access_swizzle.swizzle.indices[swizzle_index];
						new_access->type                              = to_type;

						copy_opcode(&new_opcode);
					}

					opcode constructor_call = {
					    .type = OPCODE_CALL,
					    .op_call =
					        {
					            .func            = get_type(o->op_load_access_list.to.type.type)->name,
					            .parameters      = {to[0], to[1], to[2], to[3]},
					            .parameters_size = a.access_swizzle.swizzle.size,
					            .var             = o->op_load_access_list.to,
					        },
					};
					constructor_call.size = OP_SIZE(constructor_call, op_call);
					copy_opcode(&constructor_call);
				}
				else {
					copy_opcode(o);
				}
				break;
			}
			case OPCODE_ADD:
			case OPCODE_SUB:
			case OPCODE_MULTIPLY:
			case OPCODE_DIVIDE: {
				type_id left_type  = o->op_binary.left.type.type;
				type_id right_type = o->op_binary.right.type.type;

				if (is_matrix(left_type) || is_matrix(right_type)) {
					copy_opcode(o);
					break;
				}

				uint32_t left_size  = vector_size(left_type);
				uint32_t right_size = vector_size(right_type);

				if ((flags & TRANSFORM_FLAG_BINARY_UNIFY_LENGTH) != 0 && left_size != right_size) {
					if (left_size < right_size) {
						variable last;

						if (is_vector(left_type)) {
							type_ref t;
							init_type_ref(&t, NO_NAME);
							t.type = vector_base_type(left_type);

							last = allocate_variable(t, o->op_binary.left.kind);

							opcode load_call = {
							    .type = OPCODE_LOAD_ACCESS_LIST,
							    .op_load_access_list =
							        {
							            .from = o->op_binary.left,
							            .to   = last,
							            .access_list =
							                {
							                    {
							                        .kind = ACCESS_SWIZZLE,
							                        .type = t.type,
							                        .access_swizzle =
							                            {
							                                .swizzle =
							                                    {
							                                        .indices = {left_size - 1},
							                                        .size    = 1,
							                                    },
							                            },
							                    },
							                },
							            .access_list_size = 1,
							        },
							};
							load_call.size = OP_SIZE(load_call, op_load_access_list);

							copy_opcode(&load_call);
						}
						else {
							last = o->op_binary.left;
						}

						variable vec;
						{
							type_ref t;
							init_type_ref(&t, NO_NAME);
							t.type = vector_to_size(left_type, right_size);

							vec = allocate_variable(t, o->op_binary.left.kind);

							opcode constructor_call = {
							    .type = OPCODE_CALL,
							    .op_call =
							        {
							            .func            = get_type(o->op_load_access_list.to.type.type)->name,
							            .parameters_size = right_size,
							            .var             = vec,
							        },
							};
							constructor_call.size = OP_SIZE(constructor_call, op_call);

							constructor_call.op_call.parameters[0] = o->op_binary.left;

							for (uint32_t index = left_size; index < right_size; ++index) {
								constructor_call.op_call.parameters[index] = last;
							}

							copy_opcode(&constructor_call);
						}

						{
							opcode bin         = *o;
							bin.op_binary.left = vec;

							copy_opcode(&bin);
						}
					}
					else {
						variable last;

						if (is_vector(right_type)) {
							type_ref t;
							init_type_ref(&t, NO_NAME);
							t.type = vector_base_type(right_type);

							last = allocate_variable(t, o->op_binary.right.kind);

							opcode load_call = {
							    .type = OPCODE_LOAD_ACCESS_LIST,
							    .op_load_access_list =
							        {
							            .from = o->op_binary.right,
							            .to   = last,
							            .access_list =
							                {
							                    {
							                        .kind = ACCESS_SWIZZLE,
							                        .type = t.type,
							                        .access_swizzle =
							                            {
							                                .swizzle =
							                                    {
							                                        .indices = {right_size - 1},
							                                        .size    = 1,
							                                    },
							                            },
							                    },
							                },
							            .access_list_size = 1,
							        },
							};
							load_call.size = OP_SIZE(load_call, op_load_access_list);

							copy_opcode(&load_call);
						}
						else {
							last = o->op_binary.right;
						}

						variable vec;
						{
							type_ref t;
							init_type_ref(&t, NO_NAME);
							t.type = vector_to_size(left_type, left_size);

							vec = allocate_variable(t, o->op_binary.right.kind);

							opcode constructor_call = {
							    .type = OPCODE_CALL,
							    .op_call =
							        {
							            .func            = get_type(o->op_load_access_list.to.type.type)->name,
							            .parameters_size = left_size,
							            .var             = vec,
							        },
							};
							constructor_call.size = OP_SIZE(constructor_call, op_call);

							constructor_call.op_call.parameters[0] = o->op_binary.right;

							for (uint32_t index = right_size; index < left_size; ++index) {
								constructor_call.op_call.parameters[index] = last;
							}

							copy_opcode(&constructor_call);
						}

						{
							opcode bin          = *o;
							bin.op_binary.right = vec;

							copy_opcode(&bin);
						}
					}
				}
				else {
					copy_opcode(o);
				}
				break;
			}
			default:
				copy_opcode(o);
				break;
			}

			index += o->size;
		}

		f->code = new_code;
	}
}
