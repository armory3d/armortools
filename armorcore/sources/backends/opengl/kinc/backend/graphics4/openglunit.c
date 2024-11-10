#include <stdbool.h>

static bool kinc_internal_opengl_force_16bit_index_buffer = false;
static int kinc_internal_opengl_max_vertex_attribute_arrays = 0;

#include "OpenGL.c.h"
#include "OpenGLWindow.c.h"
#include "compute.c.h"
#include "indexbuffer.c.h"
#include "pipeline.c.h"
#include "rendertarget.c.h"
#include "shader.c.h"
#include "texture.c.h"
#include "vertexbuffer.c.h"
