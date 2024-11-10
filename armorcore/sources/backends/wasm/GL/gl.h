#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>

typedef int GLint;
typedef unsigned GLuint;
typedef size_t GLsizei;
typedef float GLfloat;
typedef int GLenum;
typedef bool GLboolean;
typedef uint8_t GLubyte;
typedef float GLclampf;
typedef int GLbitfield;
typedef uint64_t GLsizeiptr;
typedef uint64_t GLintptr;
typedef char GLchar;

// custom
#define GL_MAJOR_VERSION 2
#define GL_EXTENSIONS 0
#define GL_TRUE 1
#define GL_FALSE 0

// not supported
#define GL_INFO_LOG_LENGTH 0xABCD

// regular WebGL defines
#define GL_CLAMP_TO_EDGE 0x812F
#define GL_INT_VEC2 0x8B53
#define GL_INT_VEC3 0x8B54
#define GL_INT_VEC4 0x8B55
#define GL_FLOAT_VEC2 0x8B50
#define GL_FLOAT_VEC3 0x8B51
#define GL_FLOAT_VEC4 0x8B52
#define GL_FLOAT_MAT4 0x8B5C
#define GL_UNSIGNED_SHORT 0x1403
#define GL_UNSIGNED_INT 0x1405
#define GL_TRIANGLES 0x0004
#define GL_SCISSOR_TEST 15
#define GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS 0x8B4D
#define GL_TEXTURE0 0x84C0
#define GL_TEXTURE_WRAP_S 0x2802
#define GL_TEXTURE_WRAP_T 0x2803
#define GL_REPEAT 0x2901
#define GL_DEPTH_TEST 0x0B71
#define GL_COLOR_BUFFER_BIT 0x00004000
#define GL_DEPTH_BUFFER_BIT 0x00000100
#define GL_STENCIL_BUFFER_BIT 0x00000400
#define GL_TEXTURE_2D 0x0DE1
#define GL_TEXTURE_MIN_FILTER 0x2801
#define GL_TEXTURE_MAG_FILTER 0x2800
#define GL_NEAREST 0x2600
#define GL_LINEAR 0x2601
#define GL_NEAREST_MIPMAP_NEAREST 0x2700
#define GL_LINEAR_MIPMAP_NEAREST 0x2701
#define GL_NEAREST_MIPMAP_LINEAR 0x2702
#define GL_LINEAR_MIPMAP_LINEAR 0x2703
#define GL_LEQUAL 0x0203
#define GL_NONE 0
#define GL_TEXTURE_CUBE_MAP 0x8513
#define GL_TEXTURE_MIN_LOD 0x813A
#define GL_TEXTURE_MAX_LOD 0x813B
#define GL_FRAMEBUFFER 0x8D40
#define GL_COLOR_ATTACHMENT0 0x8CE0
#define GL_DEPTH_ATTACHMENT 0x8D00
#define GL_TEXTURE_CUBE_MAP_POSITIVE_X 0x8515
#define GL_ANY_SAMPLES_PASSED 0x8C2F
#define GL_QUERY_RESULT_AVAILABLE 0x8867
#define GL_QUERY_RESULT 0x8866
#define GL_FRONT 0x0404
#define GL_BACK 0x0405
#define GL_ALWAYS 0x0207
#define GL_EQUAL 0x0202
#define GL_GREATER 0x0204
#define GL_GEQUAL 0x0206
#define GL_LESS 0x0201
#define GL_NEVER 0x0200
#define GL_NOTEQUAL 0x0205
#define GL_STATIC_DRAW 0x88E4
#define GL_DYNAMIC_DRAW 0x88E8
#define GL_ELEMENT_ARRAY_BUFFER 0x8893
#define GL_DECR 0x1E03
#define GL_DECR_WRAP 0x8508
#define GL_INCR 0x1E02
#define GL_INCR_WRAP 0x8507
#define GL_INVERT 0x150A
#define GL_KEEP 0x1E00
#define GL_REPLACE 0x1E01
#define GL_ZERO 0
#define GL_ONE 1
#define GL_SRC_ALPHA 0x0302
#define GL_DST_ALPHA 0x0304
#define GL_ONE_MINUS_SRC_ALPHA 0x0303
#define GL_ONE_MINUS_DST_ALPHA 0x0305
#define GL_SRC_COLOR 0x0300
#define GL_DST_COLOR 0x0306
#define GL_ONE_MINUS_SRC_COLOR 0x0301
#define GL_ONE_MINUS_DST_COLOR 0x0307
#define GL_FUNC_ADD 0x8006
#define GL_FUNC_SUBTRACT 0x800A
#define GL_FUNC_REVERSE_SUBTRACT 0x800B
#define GL_MIN 0x8007
#define GL_MAX 0x8008
#define GL_VERTEX_SHADER 0x8B31
#define GL_FRAGMENT_SHADER 0x8B30
#define GL_COMPILE_STATUS 0x8B81
#define GL_LINK_STATUS 0x8B82
#define GL_STENCIL_TEST 0x0B90
#define GL_CULL_FACE 0x0B44
#define GL_BLEND 0x0BE2
#define GL_FLOAT 0x1406
#define GL_ACTIVE_UNIFORMS 0x8B86
#define GL_DEPTH24_STENCIL8_OES 0x88F0
#define GL_DEPTH_COMPONENT 0x1902
#define GL_DEPTH_COMPONENT16 0x81A5
#define GL_STENCIL_ATTACHMENT 0x8D20
#define GL_RENDERBUFFER 0x8D41
#define GL_RGBA 0x1908
#define GL_LUMINANCE 0x1909
#define GL_R8 0x8229
#define GL_RED 0x1903
#define GL_UNSIGNED_BYTE 0x1401
#define GL_RGB 0x1907
#define GL_UNPACK_ALIGNMENT 0x0CF5
#define GL_ARRAY_BUFFER 0x8892
#define GL_BYTE 0x1400
#define GL_SHORT 0x1402
#define GL_INT 0x1404
#define GL_CONSTANT_COLOR 0x8001
#define GL_ONE_MINUS_CONSTANT_COLOR 0x8002
#define GL_MAX_VERTEX_ATTRIBS 0x8869

__attribute__((import_module("imports"), import_name("glUniform1i"))) void glUniform1i(GLint location, GLint v0);
__attribute__((import_module("imports"), import_name("glUniform2i"))) void glUniform2i(GLint location, GLint v0, GLint v1);
__attribute__((import_module("imports"), import_name("glUniform3i"))) void glUniform3i(GLint location, GLint v0, GLint v1, GLint v2);
__attribute__((import_module("imports"), import_name("glUniform4i"))) void glUniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
__attribute__((import_module("imports"), import_name("glUniform1iv"))) void glUniform1iv(GLint location, GLsizei count, const GLint *value);
__attribute__((import_module("imports"), import_name("glUniform2iv"))) void glUniform2iv(GLint location, GLsizei count, const GLint *value);
__attribute__((import_module("imports"), import_name("glUniform3iv"))) void glUniform3iv(GLint location, GLsizei count, const GLint *value);
__attribute__((import_module("imports"), import_name("glUniform4iv"))) void glUniform4iv(GLint location, GLsizei count, const GLint *value);
__attribute__((import_module("imports"), import_name("glUniform1f"))) void glUniform1f(GLint location, GLfloat v0);
__attribute__((import_module("imports"), import_name("glUniform2f"))) void glUniform2f(GLint location, GLfloat v0, GLfloat v1);
__attribute__((import_module("imports"), import_name("glUniform3f"))) void glUniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
__attribute__((import_module("imports"), import_name("glUniform4f"))) void glUniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
__attribute__((import_module("imports"), import_name("glUniform1fv"))) void glUniform1fv(GLint location, GLsizei count, const GLfloat *value);
__attribute__((import_module("imports"), import_name("glUniform2fv"))) void glUniform2fv(GLint location, GLsizei count, const GLfloat *value);
__attribute__((import_module("imports"), import_name("glUniform3fv"))) void glUniform3fv(GLint location, GLsizei count, const GLfloat *value);
__attribute__((import_module("imports"), import_name("glUniform4fv"))) void glUniform4fv(GLint location, GLsizei count, const GLfloat *value);
__attribute__((import_module("imports"), import_name("glUniformMatrix3fv"))) void glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose,
                                                                                                     const GLfloat *value);
__attribute__((import_module("imports"), import_name("glUniformMatrix4fv"))) void glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose,
                                                                                                     const GLfloat *value);
__attribute__((import_module("imports"), import_name("glViewport"))) void glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
__attribute__((import_module("imports"), import_name("glGetIntegerv"))) void glGetIntegerv(GLenum pname, GLint *data);
__attribute__((import_module("imports"), import_name("glGetString"))) const GLubyte *glGetString(GLenum name);
__attribute__((import_module("imports"), import_name("glDrawElements"))) void glDrawElements(GLenum mode, GLsizei count, GLenum type, const void *indices);
__attribute__((import_module("imports"), import_name("glEnable"))) void glEnable(GLenum cap);
__attribute__((import_module("imports"), import_name("glDisable"))) void glDisable(GLenum cap);
__attribute__((import_module("imports"), import_name("glScissor"))) void glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
__attribute__((import_module("imports"), import_name("glColorMask"))) void glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
__attribute__((import_module("imports"), import_name("glClearColor"))) void glClearColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
__attribute__((import_module("imports"), import_name("glDepthMask"))) void glDepthMask(GLboolean flag);
__attribute__((import_module("imports"), import_name("glClearDepthf"))) void glClearDepthf(GLclampf depth);
__attribute__((import_module("imports"), import_name("glStencilMask"))) void glStencilMask(GLuint mask);
__attribute__((import_module("imports"), import_name("glClearStencil"))) void glClearStencil(GLint s);
__attribute__((import_module("imports"), import_name("glClear"))) void glClear(GLbitfield mask);
__attribute__((import_module("imports"), import_name("glTexParameteri"))) void glTexParameteri(GLenum target, GLenum pname, GLint param);
__attribute__((import_module("imports"), import_name("glActiveTexture"))) void glActiveTexture(GLenum texture);
__attribute__((import_module("imports"), import_name("glGetFloatv"))) void glGetFloatv(GLenum pname, GLfloat *params);
__attribute__((import_module("imports"), import_name("glTexParameterf"))) void glTexParameterf(GLenum target, GLenum pname, GLfloat param);
__attribute__((import_module("imports"), import_name("glBindFramebuffer"))) void glBindFramebuffer(GLenum target, GLuint framebuffer);
__attribute__((import_module("imports"), import_name("glFramebufferTexture2D"))) void glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget,
                                                                                                             GLuint texture, GLint level);
__attribute__((import_module("imports"), import_name("glGenQueries"))) void glGenQueries(GLsizei n, GLuint *ids);
__attribute__((import_module("imports"), import_name("glDeleteQueries"))) void glDeleteQueries(GLsizei n, const GLuint *ids);
__attribute__((import_module("imports"), import_name("glBeginQuery"))) void glBeginQuery(GLenum target, GLuint id);
__attribute__((import_module("imports"), import_name("glEndQuery"))) void glEndQuery(GLenum target);
__attribute__((import_module("imports"), import_name("glGetQueryObjectuiv"))) void glGetQueryObjectuiv(GLuint id, GLenum pname, GLuint *params);
__attribute__((import_module("imports"), import_name("glDrawArrays"))) void glDrawArrays(GLenum mode, GLint first, GLsizei count);
__attribute__((import_module("imports"), import_name("glFlush"))) void glFlush(void);
__attribute__((import_module("imports"), import_name("glStencilFuncSeparate"))) void glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
__attribute__((import_module("imports"), import_name("glGenBuffers"))) void glGenBuffers(GLsizei n, GLuint *buffers);
__attribute__((import_module("imports"), import_name("glDeleteBuffers"))) void glDeleteBuffers(GLsizei n, const GLuint *buffers);
__attribute__((import_module("imports"), import_name("glBindBuffer"))) void glBindBuffer(GLenum target, GLuint buffer);
__attribute__((import_module("imports"), import_name("glBufferData"))) void glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
__attribute__((import_module("imports"), import_name("glCreateProgram"))) GLuint glCreateProgram(void);
__attribute__((import_module("imports"), import_name("glDeleteProgram"))) void glDeleteProgram(GLuint program);
__attribute__((import_module("imports"), import_name("glCreateShader"))) GLuint glCreateShader(GLenum shaderType);
__attribute__((import_module("imports"), import_name("glShaderSource"))) void glShaderSource(GLuint shader, GLsizei count, const GLchar **string,
                                                                                             const GLint *length);
__attribute__((import_module("imports"), import_name("glCompileShader"))) void glCompileShader(GLuint shader);
__attribute__((import_module("imports"), import_name("glGetShaderiv"))) void glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
__attribute__((import_module("imports"), import_name("glGetShaderInfoLog"))) void glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length,
                                                                                                     GLchar *infoLog);
__attribute__((import_module("imports"), import_name("glAttachShader"))) void glAttachShader(GLuint program, GLuint shader);
__attribute__((import_module("imports"), import_name("glBindAttribLocation"))) void glBindAttribLocation(GLuint program, GLuint index, const GLchar *name);
__attribute__((import_module("imports"), import_name("glLinkProgram"))) void glLinkProgram(GLuint program);
__attribute__((import_module("imports"), import_name("glGetProgramiv"))) void glGetProgramiv(GLuint program, GLenum pname, GLint *params);
__attribute__((import_module("imports"), import_name("glGetProgramInfoLog"))) void glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length,
                                                                                                       GLchar *infoLog);
__attribute__((import_module("imports"), import_name("glUseProgram"))) void glUseProgram(GLuint program);
__attribute__((import_module("imports"), import_name("glStencilMaskSeparate"))) void glStencilMaskSeparate(GLenum face, GLuint mask);
__attribute__((import_module("imports"), import_name("glStencilOpSeparate"))) void glStencilOpSeparate(GLenum face, GLenum sfail, GLenum dpfail, GLenum dppass);
__attribute__((import_module("imports"), import_name("glDepthFunc"))) void glDepthFunc(GLenum func);
__attribute__((import_module("imports"), import_name("glCullFace"))) void glCullFace(GLenum mode);
__attribute__((import_module("imports"), import_name("glBlendFuncSeparate"))) void glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha,
                                                                                                       GLenum dstAlpha);
__attribute__((import_module("imports"), import_name("glBlendEquationSeparate"))) void glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
__attribute__((import_module("imports"), import_name("glGetUniformLocation"))) GLint glGetUniformLocation(GLuint program, const GLchar *name);
__attribute__((import_module("imports"), import_name("glGetActiveUniform"))) void glGetActiveUniform(GLuint program, GLuint index, GLsizei bufSize,
                                                                                                     GLsizei *length, GLint *size, GLenum *type, GLchar *name);
__attribute__((import_module("imports"), import_name("glGenTextures"))) void glGenTextures(GLsizei n, GLuint *textures);
__attribute__((import_module("imports"), import_name("glDeleteTextures"))) void glDeleteTextures(GLsizei n, const GLuint *textures);
__attribute__((import_module("imports"), import_name("glBindTexture"))) void glBindTexture(GLenum target, GLuint texture);
__attribute__((import_module("imports"), import_name("glTexImage2D"))) void
glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const void *data);
__attribute__((import_module("imports"), import_name("glGenRenderbuffers"))) void glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
__attribute__((import_module("imports"), import_name("glBindRenderbuffer"))) void glBindRenderbuffer(GLenum target, GLuint renderbuffer);
__attribute__((import_module("imports"), import_name("glRenderbufferStorage"))) void glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width,
                                                                                                           GLsizei height);
__attribute__((import_module("imports"), import_name("glGenFramebuffers"))) void glGenFramebuffers(GLsizei n, GLuint *ids);
__attribute__((import_module("imports"), import_name("glDeleteFramebuffers"))) void glDeleteFramebuffers(GLsizei n, const GLuint *framebuffers);
__attribute__((import_module("imports"), import_name("glReadPixels"))) void glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format,
                                                                                         GLenum type, void *data);
__attribute__((import_module("imports"), import_name("glFramebufferRenderbuffer"))) void
glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
__attribute__((import_module("imports"), import_name("glGenerateMipmap"))) void glGenerateMipmap(GLenum target);
__attribute__((import_module("imports"), import_name("glDeleteShader"))) void glDeleteShader(GLuint shader);
__attribute__((import_module("imports"), import_name("glPixelStorei"))) void glPixelStorei(GLenum pname, GLint param);
__attribute__((import_module("imports"), import_name("glCompressedTexImage2D"))) void
glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
__attribute__((import_module("imports"), import_name("glTexSubImage2D"))) void
glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const void *pixels);
__attribute__((import_module("imports"), import_name("glBufferSubData"))) void glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size,
                                                                                               const void *data);
__attribute__((import_module("imports"), import_name("glEnableVertexAttribArray"))) void glEnableVertexAttribArray(GLuint index);
__attribute__((import_module("imports"), import_name("glDisableVertexAttribArray"))) void glDisableVertexAttribArray(GLuint index);
__attribute__((import_module("imports"), import_name("glVertexAttribPointer"))) void
glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void *pointer);
__attribute__((import_module("imports"), import_name("glBlendColor"))) void glBlendColor(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
__attribute__((import_module("imports"), import_name("glDrawBuffers"))) void glDrawBuffers(GLsizei n, const GLenum *bufs);
__attribute__((import_module("imports"), import_name("glDrawElementsInstanced"))) void glDrawElementsInstanced(GLenum mode, GLsizei count, GLenum type,
                                                                                                               const void *indices, GLsizei instancecount);
__attribute__((import_module("imports"), import_name("glVertexAttribDivisor"))) void glVertexAttribDivisor(GLuint index, GLuint divisor);
