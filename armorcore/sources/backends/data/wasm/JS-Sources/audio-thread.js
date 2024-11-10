// from https://developer.mozilla.org/en-US/docs/Web/API/AudioWorkletNode

function create_thread(func) {

}

class AudioThread extends AudioWorkletProcessor {
  constructor(options) {
    super();

    const self = this;

    this.port.onmessageerror = (e) => {
      this.port.postMessage('Error: ' + JSON.stringify(e));
    };

    const mod = options.processorOptions.mod;
    const memory = options.processorOptions.memory;
    
    const importObject = {
      env: { memory },
      imports: {
        imported_func: arg => console.log('thread: ' + arg),
        create_thread,
        glViewport: function(x, y, width, height) { },
        glScissor: function(x, y, width, height) { },
        glGetIntegerv: function(pname, data) { },
        glGetFloatv: function(pname, data) { },
        glGetString: function(name) { },
        glDrawElements: function(mode, count, type, offset) { },
        glDrawElementsInstanced: function(mode, count, type, indices, instancecount) { },
        glVertexAttribDivisor: function(index, divisor) { },
        glBindFramebuffer: function(target, framebuffer) { },
        glFramebufferTexture2D: function(target, attachment, textarget, texture, level) { },
        glGenFramebuffers: function(n, framebuffers) { },
        glGenRenderbuffers: function(n, renderbuffers) { },
        glBindRenderbuffer: function(target, renderbuffer) { },
        glRenderbufferStorage: function(target, internalformat, width, height) { },
        glFramebufferRenderbuffer: function(target, attachment, renderbuffertarget, renderbuffer) { },
        glReadPixels: function(x, y, width, height, format, type, data) { },
        glTexSubImage2D: function(target, level, xoffset, yoffset, width, height, format, type, pixels) { },
        glEnable: function(cap) { },
        glDisable: function(cap) { },
        glColorMask: function(red, green, blue, alpha) { },
        glClearColor: function(red, green, blue, alpha) { },
        glDepthMask: function(flag) { },
        glClearDepthf: function(depth) { },
        glStencilMask: function(mask) { },
        glClearStencil: function(s) { },
        glClear: function(mask) { },
        glBindBuffer: function(target, buffer) { },
        glUseProgram: function(program) { },
        glStencilMaskSeparate: function(face, mask) { },
        glStencilOpSeparate: function(face, fail, zfail, zpass) { },
        glStencilFuncSeparate: function(face, func, ref, mask) { },
        glDepthFunc: function(func) { },
        glCullFace: function(mode) { },
        glBlendFuncSeparate: function(src_rgb, dst_rgb, src_alpha, dst_alpha) { },
        glBlendEquationSeparate: function(mode_rgb, mode_alpha) { },
        glGenBuffers: function(n, buffers) { },
        glBufferData: function(target, size, data, usage) { },
        glCreateProgram: function() { },
        glAttachShader: function(program, shader) { },
        glBindAttribLocation: function(program, index, name) { },
        glLinkProgram: function(program) { },
        glGetProgramiv: function(program, pname, params) { },
        glGetProgramInfoLog: function(program) { },
        glCreateShader: function(type) { },
        glShaderSource: function(shader, count, source, length) { },
        glCompileShader: function(shader) { },
        glGetShaderiv: function(shader, pname, params) { },
        glGetShaderInfoLog: function(shader) { },
        glBufferSubData: function(target, offset, size, data) { },
        glEnableVertexAttribArray: function(index) { },
        glVertexAttribPointer: function(index, size, type, normalized, stride, offset) { },
        glDisableVertexAttribArray: function(index) { },
        glGetUniformLocation: function(program, name) { },
        glUniform1i: function(location, v0) { },
        glUniform2i: function(location, v0, v1) { },
        glUniform3i: function(location, v0, v1, v2) { },
        glUniform4i: function(location, v0, v1, v2, v3) { },
        glUniform1iv: function(location, count, value) { },
        glUniform2iv: function(location, count, value) { },
        glUniform3iv: function(location, count, value) { },
        glUniform4iv: function(location, count, value) { },
        glUniform1f: function(location, v0) { },
        glUniform2f: function(location, v0, v1) { },
        glUniform3f: function(location, v0, v1, v2) { },
        glUniform4f: function(location, v0, v1, v2, v3) { },
        glUniform1fv: function(location, count, value) { },
        glUniform2fv: function(location, count, value) { },
        glUniform3fv: function(location, count, value) { },
        glUniform4fv: function(location, count, value) { },
        glUniformMatrix3fv: function(location, count, transpose, value) { },
        glUniformMatrix4fv: function(location, count, transpose, value) { },
        glTexParameterf: function(target, pname, param) { },
        glActiveTexture: function(texture) { },
        glBindTexture: function(target, texture) { },
        glTexParameteri: function(target, pname, param) { },
        glGetActiveUniform: function(program, index, bufSize, length, size, type, name) { },
        glGenTextures: function(n, textures) { },
        glTexImage2D: function(target, level, internalformat, width, height, border, format, type, data) { },
        glPixelStorei: function(pname, param) { },
        glCompressedTexImage2D: function(target, level, internalformat, width, height, border, imageSize, data) { },
        glDrawBuffers: function(n, bufs) { },
        glGenerateMipmap: function(target) { },
        glFlush: function() { },
        glDeleteBuffers: function(n, buffers) { },
        glDeleteTextures: function(n, textures) { },
        glDeleteFramebuffers: function(n, framebuffers) { },
        glDeleteProgram: function(program) { },
        glDeleteShader: function(shader) { },
        js_fprintf: function(format) {
          console.log(read_string(format));
        },
        js_fopen: function(filename) {
          return 0;
        },
        js_ftell: function(stream) {
          return 0;
        },
        js_fseek: function(stream, offset, origin) {
          return 0;
        },
        js_fread: function(ptr, size, count, stream) {
          return 0;
        },
        js_time: function() {
          return window.performance.now();
        },
        js_pow: function(x) {
          return Math.pow(x);
        },
        js_floor: function(x) {
          return Math.floor(x);
        },
        js_sin: function(x) {
          return Math.sin(x);
        },
        js_cos: function(x) {
          return Math.cos(x);
        },
        js_tan: function(x) {
          return Math.tan(x);
        },
        js_log: function(base, exponent) {
          return Math.log(base, exponent);
        },
        js_exp: function(x) {
          return Math.exp(x);
        },
        js_sqrt: function(x) {
          return Math.sqrt(x);
        },
        js_eval: function(str) { }
      }
    };

    WebAssembly.instantiate(mod, importObject).then((instance) => {
      this.port.postMessage('Running audio thread');
      self.audio_func = instance.exports.audio_func;
      self.audio_pointer = instance.exports.malloc(16 * 1024);
      this.port.postMessage('Audio pointer: ' + self.audio_pointer);
      this.port.postMessage('Memory byteLength: ' + memory.buffer.byteLength);
      self.audio_data = new Float32Array(
        memory.buffer,
        self.audio_pointer,
        16 * 256
      );
    });
  }

  process(inputs, outputs, parameters) {
    const output = outputs[0];

    const data = output[0];

    //for (let i = 0; i < data.length; ++i) {
    //  data[i] = Math.random() * 2 - 1;
    //}

    if (this.audio_func) {
      let offset = 0;
      for (;;) {
        const length = Math.min(data.length - offset, this.audio_data.length);
        this.audio_func(this.audio_pointer + offset, length);
        for (let i = 0; i < length; ++i) {
          data[offset + i] = this.audio_data[i];
        }

        if (offset + this.audio_data.length >= data.length) {
          break;
        }
        offset += this.audio_data.length;
      }
    }

    return true;
  }
}

registerProcessor('audio-thread', AudioThread);
