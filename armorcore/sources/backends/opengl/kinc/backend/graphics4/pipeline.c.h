#include <kinc/global.h>

#include "ogl.h"

#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/pipeline.h>
#include <kinc/graphics4/shader.h>
#include <kinc/graphics4/vertexstructure.h>
#include <kinc/log.h>

#include <kinc/backend/graphics4/OpenGL.h>

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef KINC_OPENGL_ES
bool Kinc_Internal_ProgramUsesTessellation = false;
#endif
extern bool Kinc_Internal_SupportsConservativeRaster;

static GLenum convertStencilAction(kinc_g4_stencil_action_t action) {
	switch (action) {
	default:
	case KINC_G4_STENCIL_DECREMENT:
		return GL_DECR;
	case KINC_G4_STENCIL_DECREMENT_WRAP:
		return GL_DECR_WRAP;
	case KINC_G4_STENCIL_INCREMENT:
		return GL_INCR;
	case KINC_G4_STENCIL_INCREMENT_WRAP:
		return GL_INCR_WRAP;
	case KINC_G4_STENCIL_INVERT:
		return GL_INVERT;
	case KINC_G4_STENCIL_KEEP:
		return GL_KEEP;
	case KINC_G4_STENCIL_REPLACE:
		return GL_REPLACE;
	case KINC_G4_STENCIL_ZERO:
		return GL_ZERO;
	}
}

static GLenum convert_blend_factor(kinc_g4_blending_factor_t factor) {
	switch (factor) {
	case KINC_G4_BLEND_ZERO:
		return GL_ZERO;
	case KINC_G4_BLEND_ONE:
		return GL_ONE;
	case KINC_G4_BLEND_SOURCE_ALPHA:
		return GL_SRC_ALPHA;
	case KINC_G4_BLEND_DEST_ALPHA:
		return GL_DST_ALPHA;
	case KINC_G4_BLEND_INV_SOURCE_ALPHA:
		return GL_ONE_MINUS_SRC_ALPHA;
	case KINC_G4_BLEND_INV_DEST_ALPHA:
		return GL_ONE_MINUS_DST_ALPHA;
	case KINC_G4_BLEND_SOURCE_COLOR:
		return GL_SRC_COLOR;
	case KINC_G4_BLEND_DEST_COLOR:
		return GL_DST_COLOR;
	case KINC_G4_BLEND_INV_SOURCE_COLOR:
		return GL_ONE_MINUS_SRC_COLOR;
	case KINC_G4_BLEND_INV_DEST_COLOR:
		return GL_ONE_MINUS_DST_COLOR;
	case KINC_G4_BLEND_CONSTANT:
		return GL_CONSTANT_COLOR;
	case KINC_G4_BLEND_INV_CONSTANT:
		return GL_ONE_MINUS_CONSTANT_COLOR;
	default:
		assert(false);
		return GL_ONE;
	}
}

static GLenum convert_blend_operation(kinc_g4_blending_operation_t operation) {
	switch (operation) {
	case KINC_G4_BLENDOP_ADD:
		return GL_FUNC_ADD;
	case KINC_G4_BLENDOP_SUBTRACT:
		return GL_FUNC_SUBTRACT;
	case KINC_G4_BLENDOP_REVERSE_SUBTRACT:
		return GL_FUNC_REVERSE_SUBTRACT;
	case KINC_G4_BLENDOP_MIN:
		return GL_MIN;
	case KINC_G4_BLENDOP_MAX:
		return GL_MAX;
	default:
		assert(false);
		return GL_FUNC_ADD;
	}
}

void kinc_g4_pipeline_init(kinc_g4_pipeline_t *state) {
	memset(state, 0, sizeof(kinc_g4_pipeline_t));

	kinc_g4_internal_pipeline_set_defaults(state);

	state->impl.textureCount = 0;
	// TODO: Get rid of allocations
	state->impl.textures = (char **)malloc(sizeof(char *) * 16);
	for (int i = 0; i < 16; ++i) {
		state->impl.textures[i] = (char *)malloc(sizeof(char) * 128);
		state->impl.textures[i][0] = 0;
	}
	state->impl.textureValues = (int *)malloc(sizeof(int) * 16);
	state->impl.programId = glCreateProgram();
	glCheckErrors();
}

void kinc_g4_pipeline_destroy(kinc_g4_pipeline_t *state) {
	for (int i = 0; i < 16; ++i) {
		free(state->impl.textures[i]);
	}
	free(state->impl.textures);
	free(state->impl.textureValues);
	glDeleteProgram(state->impl.programId);
}

static int toGlShader(kinc_g4_shader_type_t type) {
	switch (type) {
	case KINC_G4_SHADER_TYPE_VERTEX:
	default:
		return GL_VERTEX_SHADER;
	case KINC_G4_SHADER_TYPE_FRAGMENT:
		return GL_FRAGMENT_SHADER;
#ifndef KINC_OPENGL_ES
	case KINC_G4_SHADER_TYPE_GEOMETRY:
		return GL_GEOMETRY_SHADER;
	case KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL:
		return GL_TESS_CONTROL_SHADER;
	case KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION:
		return GL_TESS_EVALUATION_SHADER;
#endif
	}
}

static void compileShader(unsigned *id, const char *source, size_t length, kinc_g4_shader_type_t type) {
	*id = glCreateShader(toGlShader(type));
	glCheckErrors();
	glShaderSource(*id, 1, (const GLchar **)&source, 0);
	glCompileShader(*id);

	int result;
	glGetShaderiv(*id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		int length = 0;
		glGetShaderiv(*id, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(length);
		glGetShaderInfoLog(*id, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL compiler error: %s", errormessage);
		free(errormessage);
	}
}

void kinc_g4_pipeline_compile(kinc_g4_pipeline_t *state) {
	compileShader(&state->vertex_shader->impl._glid, state->vertex_shader->impl.source, state->vertex_shader->impl.length, KINC_G4_SHADER_TYPE_VERTEX);
	compileShader(&state->fragment_shader->impl._glid, state->fragment_shader->impl.source, state->fragment_shader->impl.length, KINC_G4_SHADER_TYPE_FRAGMENT);
#ifndef KINC_OPENGL_ES
	if (state->geometry_shader != NULL) {
		compileShader(&state->geometry_shader->impl._glid, state->geometry_shader->impl.source, state->geometry_shader->impl.length,
		              KINC_G4_SHADER_TYPE_GEOMETRY);
	}
	if (state->tessellation_control_shader != NULL) {
		compileShader(&state->tessellation_control_shader->impl._glid, state->tessellation_control_shader->impl.source,
		              state->tessellation_control_shader->impl.length, KINC_G4_SHADER_TYPE_TESSELLATION_CONTROL);
	}
	if (state->tessellation_evaluation_shader != NULL) {
		compileShader(&state->tessellation_evaluation_shader->impl._glid, state->tessellation_evaluation_shader->impl.source,
		              state->tessellation_evaluation_shader->impl.length, KINC_G4_SHADER_TYPE_TESSELLATION_EVALUATION);
	}
#endif
	glAttachShader(state->impl.programId, state->vertex_shader->impl._glid);
	glAttachShader(state->impl.programId, state->fragment_shader->impl._glid);
#ifndef KINC_OPENGL_ES
	if (state->geometry_shader != NULL) {
		glAttachShader(state->impl.programId, state->geometry_shader->impl._glid);
	}
	if (state->tessellation_control_shader != NULL) {
		glAttachShader(state->impl.programId, state->tessellation_control_shader->impl._glid);
	}
	if (state->tessellation_evaluation_shader != NULL) {
		glAttachShader(state->impl.programId, state->tessellation_evaluation_shader->impl._glid);
	}
#endif
	glCheckErrors();

	int index = 0;
	for (int i1 = 0; state->input_layout[i1] != NULL; ++i1) {
		for (int i2 = 0; i2 < state->input_layout[i1]->size; ++i2) {
			kinc_g4_vertex_element_t element = state->input_layout[i1]->elements[i2];
			glBindAttribLocation(state->impl.programId, index, element.name);
			glCheckErrors();
			if (element.data == KINC_G4_VERTEX_DATA_F32_4X4) {
				index += 4;
			}
			else {
				++index;
			}
		}
	}

	glLinkProgram(state->impl.programId);

	int result;
	glGetProgramiv(state->impl.programId, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length = 0;
		glGetProgramiv(state->impl.programId, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(length);
		glGetProgramInfoLog(state->impl.programId, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL linker error: %s", errormessage);
		free(errormessage);
	}

#ifndef KINC_OPENGL_ES
#ifndef KINC_LINUX
	if (state->tessellation_control_shader != NULL) {
		glPatchParameteri(GL_PATCH_VERTICES, 3);
		glCheckErrors();
	}
#endif
#endif
}

void kinc_g4_internal_set_pipeline(kinc_g4_pipeline_t *pipeline) {
#ifndef KINC_OPENGL_ES
	Kinc_Internal_ProgramUsesTessellation = pipeline->tessellation_control_shader != NULL;
#endif
	glUseProgram(pipeline->impl.programId);
	glCheckErrors();
	for (int index = 0; index < pipeline->impl.textureCount; ++index) {
		glUniform1i(pipeline->impl.textureValues[index], index);
		glCheckErrors();
	}

	if (pipeline->stencil_front_mode == KINC_G4_COMPARE_ALWAYS && pipeline->stencil_back_mode == KINC_G4_COMPARE_ALWAYS &&
	    pipeline->stencil_front_both_pass == KINC_G4_STENCIL_KEEP && pipeline->stencil_back_both_pass == KINC_G4_STENCIL_KEEP &&
	    pipeline->stencil_front_depth_fail == KINC_G4_STENCIL_KEEP && pipeline->stencil_back_depth_fail == KINC_G4_STENCIL_KEEP &&
	    pipeline->stencil_front_fail == KINC_G4_STENCIL_KEEP && pipeline->stencil_back_fail == KINC_G4_STENCIL_KEEP) {
		glDisable(GL_STENCIL_TEST);
	}
	else {
		glEnable(GL_STENCIL_TEST);

		glStencilMaskSeparate(GL_FRONT, pipeline->stencil_write_mask);
		glStencilOpSeparate(GL_FRONT, convertStencilAction(pipeline->stencil_front_fail), convertStencilAction(pipeline->stencil_front_depth_fail),
		                    convertStencilAction(pipeline->stencil_front_both_pass));
		glStencilFuncSeparate(GL_FRONT, Kinc_G4_Internal_StencilFunc(pipeline->stencil_front_mode), pipeline->stencil_reference_value,
		                      pipeline->stencil_read_mask);

		glStencilMaskSeparate(GL_BACK, pipeline->stencil_write_mask);
		glStencilOpSeparate(GL_BACK, convertStencilAction(pipeline->stencil_back_fail), convertStencilAction(pipeline->stencil_back_depth_fail),
		                    convertStencilAction(pipeline->stencil_back_both_pass));
		glStencilFuncSeparate(GL_BACK, Kinc_G4_Internal_StencilFunc(pipeline->stencil_back_mode), pipeline->stencil_reference_value,
		                      pipeline->stencil_read_mask);
	}

#ifdef KINC_OPENGL_ES
	glColorMask(pipeline->color_write_mask_red[0], pipeline->color_write_mask_green[0], pipeline->color_write_mask_blue[0],
	            pipeline->color_write_mask_alpha[0]);
#else
	for (int i = 0; i < 8; ++i)
		glColorMaski(i, pipeline->color_write_mask_red[i], pipeline->color_write_mask_green[i], pipeline->color_write_mask_blue[i],
		             pipeline->color_write_mask_alpha[i]);
#endif

	if (Kinc_Internal_SupportsConservativeRaster) {
		if (pipeline->conservative_rasterization) {
			glEnable(0x9346); // GL_CONSERVATIVE_RASTERIZATION_NV
		}
		else {
			glDisable(0x9346);
		}
	}

	glCheckErrors();

	/*switch (state) {
	case Normalize:
	device->SetRenderState(D3DRS_NORMALIZENORMALS, on ? TRUE : FALSE);
	break;
	case BackfaceCulling:
	if (on) device->SetRenderState(D3DRS_CULLMODE, D3DCULL_CCW);
	else device->SetRenderState(D3DRS_CULLMODE, D3DCULL_NONE);
	break;
	case FogState:
	device->SetRenderState(D3DRS_FOGENABLE, on ? TRUE : FALSE);
	break;
	case ScissorTestState:
	device->SetRenderState(D3DRS_SCISSORTESTENABLE, on ? TRUE : FALSE);
	break;
	case AlphaTestState:
	device->SetRenderState(D3DRS_ALPHATESTENABLE, on ? TRUE : FALSE);
	device->SetRenderState(D3DRS_ALPHAFUNC, D3DCMP_GREATEREQUAL);
	break;
	default:
	throw Exception();
	}*/

	if (pipeline->depth_write) {
		glDepthMask(GL_TRUE);
	}
	else {
		glDepthMask(GL_FALSE);
	}

	if (pipeline->depth_mode != KINC_G4_COMPARE_ALWAYS) {
		glEnable(GL_DEPTH_TEST);
	}
	else {
		glDisable(GL_DEPTH_TEST);
	}

	GLenum func = GL_ALWAYS;
	switch (pipeline->depth_mode) {
	default:
	case KINC_G4_COMPARE_ALWAYS:
		func = GL_ALWAYS;
		break;
	case KINC_G4_COMPARE_NEVER:
		func = GL_NEVER;
		break;
	case KINC_G4_COMPARE_EQUAL:
		func = GL_EQUAL;
		break;
	case KINC_G4_COMPARE_NOT_EQUAL:
		func = GL_NOTEQUAL;
		break;
	case KINC_G4_COMPARE_LESS:
		func = GL_LESS;
		break;
	case KINC_G4_COMPARE_LESS_EQUAL:
		func = GL_LEQUAL;
		break;
	case KINC_G4_COMPARE_GREATER:
		func = GL_GREATER;
		break;
	case KINC_G4_COMPARE_GREATER_EQUAL:
		func = GL_GEQUAL;
		break;
	}
	glDepthFunc(func);
	glCheckErrors();

	switch (pipeline->cull_mode) {
	case KINC_G4_CULL_CLOCKWISE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_BACK);
		glCheckErrors();
		break;
	case KINC_G4_CULL_COUNTER_CLOCKWISE:
		glEnable(GL_CULL_FACE);
		glCullFace(GL_FRONT);
		glCheckErrors();
		break;
	case KINC_G4_CULL_NOTHING:
		glDisable(GL_CULL_FACE);
		glCheckErrors();
		break;
	default:
		break;
	}

	/*switch (state) {
	case DepthTestCompare:
	switch (v) {
	// TODO: Cmp-Konstanten systemabhaengig abgleichen
	default:
	case ZCmp_Always      : v = D3DCMP_ALWAYS; break;
	case ZCmp_Never       : v = D3DCMP_NEVER; break;
	case ZCmp_Equal       : v = D3DCMP_EQUAL; break;
	case ZCmp_NotEqual    : v = D3DCMP_NOTEQUAL; break;
	case ZCmp_Less        : v = D3DCMP_LESS; break;
	case ZCmp_LessEqual   : v = D3DCMP_LESSEQUAL; break;
	case ZCmp_Greater     : v = D3DCMP_GREATER; break;
	case ZCmp_GreaterEqual: v = D3DCMP_GREATEREQUAL; break;
	}
	device->SetRenderState(D3DRS_ZFUNC, v);
	break;
	case FogTypeState:
	switch (v) {
	case LinearFog:
	device->SetRenderState(D3DRS_FOGVERTEXMODE, D3DFOG_LINEAR);
	}
	break;
	case AlphaReferenceState:
	device->SetRenderState(D3DRS_ALPHAREF, (DWORD)v);
	break;
	default:
	throw Exception();
	}*/

	if (pipeline->blend_source != KINC_G4_BLEND_ONE || pipeline->blend_destination != KINC_G4_BLEND_ZERO || pipeline->alpha_blend_source != KINC_G4_BLEND_ONE ||
	    pipeline->alpha_blend_destination != KINC_G4_BLEND_ZERO) {
		glEnable(GL_BLEND);
	}
	else {
		glDisable(GL_BLEND);
	}

	// glBlendFunc(convert(pipeline->blendSource), convert(pipeline->blendDestination));
	glBlendFuncSeparate(convert_blend_factor(pipeline->blend_source), convert_blend_factor(pipeline->blend_destination),
	                    convert_blend_factor(pipeline->alpha_blend_source), convert_blend_factor(pipeline->alpha_blend_destination));
	glBlendEquationSeparate(convert_blend_operation(pipeline->blend_operation), convert_blend_operation(pipeline->alpha_blend_operation));
}

void kinc_g4_pipeline_get_constant_locations(kinc_g4_pipeline_t *state, kinc_g4_constant_location_t *vertex_locations,
                                             kinc_g4_constant_location_t *fragment_locations, int *vertex_sizes, int *fragment_sizes, int *max_vertex,
                                             int *max_fragment) {
	// GLint count = 0;
	// glGetProgramiv(state->impl.programId, GL_ACTIVE_UNIFORMS, &count);
	// if (locations == NULL || sizes == NULL) {
	// 	*max_count = count;
	// }
	// else {
	// 	for (GLint i = 0; i < count; ++i) {
	// 		GLenum type;
	// 		char uniformName[1024];
	// 		GLsizei length;
	// 		GLint size;
	// 		glGetActiveUniform(state->impl.programId, i, 1024 - 1, &length, &size, &type, uniformName);
	// 		locations[i].impl.location = glGetUniformLocation(state->impl.programId, uniformName);
	// 		locations[i].impl.type = type;
	// 		sizes[i] = size;
	// 	}
	// }
}

kinc_g4_constant_location_t kinc_g4_pipeline_get_constant_location(kinc_g4_pipeline_t *state, const char *name) {
	kinc_g4_constant_location_t location;
	location.impl.location = glGetUniformLocation(state->impl.programId, name);
	location.impl.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(state->impl.programId, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(state->impl.programId, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.impl.type = type;
			break;
		}
	}
	glCheckErrors();
	if (location.impl.location < 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}
	return location;
}

static int findTexture(kinc_g4_pipeline_t *state, const char *name) {
	for (int index = 0; index < state->impl.textureCount; ++index) {
		if (strcmp(state->impl.textures[index], name) == 0)
			return index;
	}
	return -1;
}

kinc_g4_texture_unit_t kinc_g4_pipeline_get_texture_unit(kinc_g4_pipeline_t *state, const char *name) {
	int index = findTexture(state, name);
	if (index < 0) {
		int location = glGetUniformLocation(state->impl.programId, name);
		glCheckErrors();
		index = state->impl.textureCount;
		state->impl.textureValues[index] = location;
		strcpy(state->impl.textures[index], name);
		++state->impl.textureCount;
	}
	kinc_g4_texture_unit_t unit;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}
	unit.stages[KINC_G4_SHADER_TYPE_FRAGMENT] = index;
	return unit;
}
