#include <kinc/graphics4/compute.h>
#include <kinc/graphics4/graphics.h>
#include <kinc/graphics4/rendertarget.h>
#include <kinc/graphics4/texture.h>
#include <kinc/image.h>
#include <kinc/log.h>
#include <kinc/math/core.h>

#include <kinc/backend/graphics4/ogl.h>

#include <stdio.h>
#include <string.h>

#if defined(KINC_WINDOWS) || (defined(KINC_LINUX) && defined(GL_VERSION_4_3)) || (defined(KINC_ANDROID) && defined(GL_ES_VERSION_3_1))
#define HAS_COMPUTE
bool kinc_internal_gl_has_compute = true;
#else
bool kinc_internal_gl_has_compute = false;
#endif

#ifdef HAS_COMPUTE
static int convertInternalImageFormat(kinc_image_format_t format) {
	switch (format) {
	case KINC_IMAGE_FORMAT_RGBA128:
		return GL_RGBA32F;
	case KINC_IMAGE_FORMAT_RGBA64:
		return GL_RGBA16F;
	case KINC_IMAGE_FORMAT_RGBA32:
	default:
		return GL_RGBA8;
	case KINC_IMAGE_FORMAT_A32:
		return GL_R32F;
	case KINC_IMAGE_FORMAT_A16:
		return GL_R16F;
	case KINC_IMAGE_FORMAT_GREY8:
		return GL_R8;
	}
}

static int convertInternalRTFormat(kinc_g4_render_target_format_t format) {
	switch (format) {
	case KINC_G4_RENDER_TARGET_FORMAT_64BIT_FLOAT:
		return GL_RGBA16F;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT_RED_FLOAT:
		return GL_R32F;
	case KINC_G4_RENDER_TARGET_FORMAT_128BIT_FLOAT:
		return GL_RGBA32F;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_DEPTH:
		return GL_DEPTH_COMPONENT16;
	case KINC_G4_RENDER_TARGET_FORMAT_8BIT_RED:
		return GL_RED;
	case KINC_G4_RENDER_TARGET_FORMAT_16BIT_RED_FLOAT:
		return GL_R16F;
	case KINC_G4_RENDER_TARGET_FORMAT_32BIT:
	default:
		return GL_RGBA;
	}
}
#endif

void kinc_g4_compute_shader_init(kinc_g4_compute_shader *shader, void *source, int length) {
	shader->impl._length = length;
	shader->impl.textureCount = 0;
	shader->impl._source = (char *)malloc(sizeof(char) * (length + 1));
	for (int i = 0; i < length; ++i) {
		shader->impl._source[i] = ((char *)source)[i];
	}
	shader->impl._source[length] = 0;

#ifdef HAS_COMPUTE
	shader->impl._id = glCreateShader(GL_COMPUTE_SHADER);
	glCheckErrors();
	glShaderSource(shader->impl._id, 1, (const GLchar **)&shader->impl._source, NULL);
	glCompileShader(shader->impl._id);

	int result;
	glGetShaderiv(shader->impl._id, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetShaderiv(shader->impl._id, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(sizeof(char) * length);
		glGetShaderInfoLog(shader->impl._id, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL compiler error: %s\n", errormessage);
		free(errormessage);
	}

	shader->impl._programid = glCreateProgram();
	glAttachShader(shader->impl._programid, shader->impl._id);
	glLinkProgram(shader->impl._programid);

	glGetProgramiv(shader->impl._programid, GL_LINK_STATUS, &result);
	if (result != GL_TRUE) {
		int length;
		glGetProgramiv(shader->impl._programid, GL_INFO_LOG_LENGTH, &length);
		char *errormessage = (char *)malloc(sizeof(char) * length);
		glGetProgramInfoLog(shader->impl._programid, length, NULL, errormessage);
		kinc_log(KINC_LOG_LEVEL_ERROR, "GLSL linker error: %s\n", errormessage);
		free(errormessage);
	}
#endif

	// TODO: Get rid of allocations
	shader->impl.textures = (char **)malloc(sizeof(char *) * 16);
	for (int i = 0; i < 16; ++i) {
		shader->impl.textures[i] = (char *)malloc(sizeof(char) * 128);
		shader->impl.textures[i][0] = 0;
	}
	shader->impl.textureValues = (int *)malloc(sizeof(int) * 16);
}

void kinc_g4_compute_shader_destroy(kinc_g4_compute_shader *shader) {
	free(shader->impl._source);
	shader->impl._source = NULL;
#ifdef HAS_COMPUTE
	glDeleteProgram(shader->impl._programid);
	glDeleteShader(shader->impl._id);
#endif
}
kinc_g4_constant_location_t kinc_g4_compute_shader_get_constant_location(kinc_g4_compute_shader *shader, const char *name) {
	kinc_g4_constant_location_t location;
#ifdef HAS_COMPUTE
	location.impl.location = glGetUniformLocation(shader->impl._programid, name);
	location.impl.type = GL_FLOAT;
	GLint count = 0;
	glGetProgramiv(shader->impl._programid, GL_ACTIVE_UNIFORMS, &count);
	char arrayName[1024];
	strcpy(arrayName, name);
	strcat(arrayName, "[0]");
	for (GLint i = 0; i < count; ++i) {
		GLenum type;
		char uniformName[1024];
		GLsizei length;
		GLint size;
		glGetActiveUniform(shader->impl._programid, i, 1024 - 1, &length, &size, &type, uniformName);
		if (strcmp(uniformName, name) == 0 || strcmp(uniformName, arrayName) == 0) {
			location.impl.type = type;
			break;
		}
	}
	glCheckErrors();
	if (location.impl.location < 0) {
		kinc_log(KINC_LOG_LEVEL_WARNING, "Uniform %s not found.", name);
	}
#endif
	return location;
}

static int compute_findTexture(kinc_g4_compute_shader *shader, const char *name) {
	for (int index = 0; index < shader->impl.textureCount; ++index) {
		if (strcmp(shader->impl.textures[index], name) == 0)
			return index;
	}
	return -1;
}

kinc_g4_texture_unit_t kinc_g4_compute_shader_get_texture_unit(kinc_g4_compute_shader *shader, const char *name) {
	int index = compute_findTexture(shader, name);
	if (index < 0) {
		int location = glGetUniformLocation(shader->impl._programid, name);
		glCheckErrors();
		index = shader->impl.textureCount;
		shader->impl.textureValues[index] = location;
		strcpy(shader->impl.textures[index], name);
		++shader->impl.textureCount;
	}
	kinc_g4_texture_unit_t unit;
	for (int i = 0; i < KINC_G4_SHADER_TYPE_COUNT; ++i) {
		unit.stages[i] = -1;
	}
	unit.stages[KINC_G4_SHADER_TYPE_COMPUTE] = index;
	return unit;
}

void kinc_g4_set_compute_shader(kinc_g4_compute_shader *shader) {
#ifdef HAS_COMPUTE
	glUseProgram(shader->impl._programid);
	glCheckErrors();

	for (int index = 0; index < shader->impl.textureCount; ++index) {
		glUniform1i(shader->impl.textureValues[index], index);
		glCheckErrors();
	}
#endif
}

void kinc_g4_compute(int x, int y, int z) {
#ifdef HAS_COMPUTE
	glDispatchCompute(x, y, z);
	glCheckErrors();
	glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
	glCheckErrors();
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	glCheckErrors();
#endif
}
