
// To be replaced with https://github.com/Kode/Kongruent

//
// Copyright (C) 2002-2005  3Dlabs Inc. Ltd.
// Copyright (C) 2013-2016 LunarG, Inc.
//
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions
// are met:
//
//    Redistributions of source code must retain the above copyright
//    notice, this list of conditions and the following disclaimer.
//
//    Redistributions in binary form must reproduce the above
//    copyright notice, this list of conditions and the following
//    disclaimer in the documentation and/or other materials provided
//    with the distribution.
//
//    Neither the name of 3Dlabs Inc. Ltd. nor the names of its
//    contributors may be used to endorse or promote products derived
//    from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
// FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
// COPYRIGHT HOLDERS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
// INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
// BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
// CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
// ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
//

#include <cstring>
#include <cstdlib>
#include <cctype>
#include <cmath>
#include <array>
#include <sstream>
#include <iostream>
#include <fstream>
#include <cstdint>
#include <map>
#include <string>
#include <vector>
#include <algorithm>
#include <string.h>
#include <strstream>
#include "../glslang/StandAlone/ResourceLimits.h"
#include "../glslang/StandAlone/Worklist.h"
#include "../glslang/SPIRV/GlslangToSpv.h"
#include "../glslang/SPIRV/spirv.hpp"
#include "../glslang/glslang/Public/ShaderLang.h"

namespace krafix {
	enum TargetLanguage {
		SpirV
	};

	enum ShaderStage {
		StageVertex,
		StageGeometry,
		StageFragment,
		StageCompute
	};

	enum TargetSystem {
		Linux,
		Android,
		Unknown
	};

	struct Target {
		int version;
		TargetSystem system;
	};

	class Instruction {
	public:
		Instruction(std::vector<unsigned>& spirv, unsigned& index);
		Instruction(int opcode, unsigned* operands, unsigned length);
		int opcode;
		unsigned* operands;
		unsigned length;
		const char* string;
	};

	class SpirVTranslator {
	public:
		SpirVTranslator(std::vector<unsigned>& spirv, ShaderStage stage);
		void outputCode(const Target& target, const char* sourcefilename, const char* filename, char* output, std::map<std::string, int>& attributes);
		int outputLength;
		int writeInstructions(const char* filename, char* output, std::vector<Instruction>& instructions);
		int writeInstructions(std::vector<uint32_t>& output, std::vector<Instruction>& instructions);
		std::vector<unsigned>& spirv;
		std::vector<Instruction> instructions;
		ShaderStage stage;
		spv::ExecutionModel executionModel();
		unsigned magicNumber;
		unsigned version;
		unsigned generator;
		unsigned bound;
		unsigned schema;
	};
}

using namespace krafix;

Instruction::Instruction(std::vector<unsigned>& spirv, unsigned& index) {
	using namespace spv;

	int wordCount = spirv[index] >> 16;
	opcode = (Op)(spirv[index] & 0xffff);

	operands = wordCount > 1 ? &spirv[index + 1] : NULL;
	length = wordCount - 1;

	switch (opcode) {
	case OpString:
		string = (char*)&spirv[index + 2];
		break;
	case OpName:
		string = (char*)&spirv[index + 2];
		break;
	case OpMemberName:
		string = (char*)&spirv[index + 3];
		break;
	case OpEntryPoint:
		string = (char*)&spirv[index + 3];
		break;
	case OpSourceExtension:
		string = (char*)&spirv[index + 1];
		break;
	default:
		string = NULL;
		break;
	}

	index += wordCount;
}

Instruction::Instruction(int opcode, unsigned* operands, unsigned length)
	: opcode(opcode), operands(operands), length(length), string(NULL) {
}

namespace {
	enum SpirVState {
		SpirVStart,
		SpirVDebugInformation,
		SpirVAnnotations,
		SpirVTypes,
		SpirVFunctions
	};

	void writeInstruction(std::ostream* out, unsigned word) {
		out->put(word & 0xff);
		out->put((word >> 8) & 0xff);
		out->put((word >> 16) & 0xff);
		out->put((word >> 24) & 0xff);
	}

	void writeInstruction(std::vector<uint32_t>& out, unsigned word) {
		out.push_back(word);
	}

	bool isDebugInformation(Instruction& instruction) {
		return instruction.opcode == spv::OpSource || instruction.opcode == spv::OpSourceExtension
			|| instruction.opcode == spv::OpName || instruction.opcode == spv::OpMemberName;
	}

	bool isAnnotation(Instruction& instruction) {
		return instruction.opcode == spv::OpDecorate || instruction.opcode == spv::OpMemberDecorate;
	}

	bool isType(Instruction& instruction) {
		return instruction.opcode == spv::OpTypeArray || instruction.opcode == spv::OpTypeBool || instruction.opcode == spv::OpTypeFloat || instruction.opcode == spv::OpTypeFunction
			|| instruction.opcode == spv::OpTypeInt || instruction.opcode == spv::OpTypePointer || instruction.opcode == spv::OpTypeVector || instruction.opcode == spv::OpTypeVoid;
	}

	struct Var {
		std::string name;
		unsigned id;
		unsigned type;
		unsigned pointertype;
	};

	bool varcompare(const Var& a, const Var& b) {
		return strcmp(a.name.c_str(), b.name.c_str()) < 0;
	}

	unsigned copyname(const std::string& name, unsigned* instructionsData, unsigned& instructionsDataIndex) {
		unsigned length = 0;
		bool zeroset = false;
		for (unsigned i2 = 0; i2 < name.size(); i2 += 4) {
			char* data = (char*)&instructionsData[instructionsDataIndex];
			for (unsigned i3 = 0; i3 < 4; ++i3) {
				if (i2 + i3 < name.size()) data[i3] = name[i2 + i3];
				else {
					data[i3] = 0;
					zeroset = true;
				}
			}
			++length;
			++instructionsDataIndex;
		}
		if (!zeroset) {
			instructionsData[instructionsDataIndex++] = 0;
			++length;
		}
		return length;
	}
}

SpirVTranslator::SpirVTranslator(std::vector<unsigned>& spirv, ShaderStage stage) : stage(stage), spirv(spirv) {
	if (spirv.size() < 5) { return; }

	unsigned index = 0;
	magicNumber = spirv[index++];
	version = spirv[index++];
	generator = spirv[index++];
	bound = spirv[index++];
	schema = spirv[index++];

	while (index < spirv.size()) {
		instructions.push_back(Instruction(spirv, index));
	}
}

spv::ExecutionModel SpirVTranslator::executionModel() {
	switch (stage) {
	case StageVertex:
		return spv::ExecutionModelVertex;
	case StageGeometry:
		return spv::ExecutionModelGeometry;
	case StageFragment:
		return spv::ExecutionModelFragment;
	case StageCompute:
		return spv::ExecutionModelGLCompute;
	default:
		throw "Unknown shader stage";
	}
}

int SpirVTranslator::writeInstructions(const char* filename, char* output, std::vector<Instruction>& instructions) {
	std::ofstream fileout;
	std::ostrstream arrayout(output, 1024 * 1024);
	std::ostream* out;

	if (output) {
		out = &arrayout;
	}
	else {
		fileout.open(filename, std::ios::binary | std::ios::out);
		out = &fileout;
	}

	int length = 0;
	writeInstruction(out, magicNumber);
	length += 4;
	writeInstruction(out, version);
	length += 4;
	writeInstruction(out, generator);
	length += 4;
	writeInstruction(out, bound);
	length += 4;
	writeInstruction(out, schema);
	length += 4;

	for (unsigned i = 0; i < instructions.size(); ++i) {
		Instruction& inst = instructions[i];
		writeInstruction(out, ((inst.length + 1) << 16) | (unsigned)inst.opcode);
		length += 4;
		for (unsigned i2 = 0; i2 < inst.length; ++i2) {
			writeInstruction(out, inst.operands[i2]);
			length += 4;
		}
	}

	if (!output) {
		fileout.close();
	}

	return length;
}

int SpirVTranslator::writeInstructions(std::vector<uint32_t>& output, std::vector<Instruction>& instructions) {
	int length = 0;
	writeInstruction(output, magicNumber);
	length += 4;
	writeInstruction(output, version);
	length += 4;
	writeInstruction(output, generator);
	length += 4;
	writeInstruction(output, bound);
	length += 4;
	writeInstruction(output, schema);
	length += 4;

	for (unsigned i = 0; i < instructions.size(); ++i) {
		Instruction& inst = instructions[i];
		writeInstruction(output, ((inst.length + 1) << 16) | (unsigned)inst.opcode);
		length += 4;
		for (unsigned i2 = 0; i2 < inst.length; ++i2) {
			writeInstruction(output, inst.operands[i2]);
			length += 4;
		}
	}

	return length;
}

namespace {
	using namespace spv;

	uint32_t alignOffset(uint32_t offset, uint32_t alignment) {
		uint32_t mask = alignment - 1;
		if ((offset & mask) == 0) {
			return offset;
		}
		else {
			return (offset + alignment - 1) & ~mask;
		}
	}

	void outputNames(unsigned* instructionsData, unsigned& instructionsDataIndex, std::vector<unsigned>& structtypeindices, unsigned& structvarindex, std::vector<Instruction>& newinstructions, std::vector<Var>& uniforms) {
		if (uniforms.size() > 0) {
			Instruction structtypename(OpName, &instructionsData[instructionsDataIndex], 0);
			structtypeindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			structtypename.length = 1 + copyname("_k_global_uniform_buffer_type", instructionsData, instructionsDataIndex);
			newinstructions.push_back(structtypename);

			Instruction structname(OpName, &instructionsData[instructionsDataIndex], 0);
			structvarindex = instructionsDataIndex;
			instructionsData[instructionsDataIndex++] = 0;
			structname.length = 1 + copyname("_k_global_uniform_buffer", instructionsData, instructionsDataIndex);
			newinstructions.push_back(structname);

			for (unsigned i = 0; i < uniforms.size(); ++i) {
				Instruction name(OpMemberName, &instructionsData[instructionsDataIndex], 0);
				structtypeindices.push_back(instructionsDataIndex);
				instructionsData[instructionsDataIndex++] = 0;
				instructionsData[instructionsDataIndex++] = i;
				name.length = 2 + copyname(uniforms[i].name, instructionsData, instructionsDataIndex);
				newinstructions.push_back(name);
			}
		}
	}

	unsigned booltype = 0;
	unsigned inttype = 0;
	unsigned uinttype = 0;
	unsigned floattype = 0;
	unsigned vec4type = 0;
	unsigned vec3type = 0;
	unsigned vec2type = 0;
	unsigned mat4type = 0;
	unsigned mat3type = 0;
	unsigned mat2type = 0;
	unsigned floatarraytype = 0;
	unsigned vec2arraytype = 0;
	unsigned vec3arraytype = 0;
	unsigned vec4arraytype = 0;

	void outputDecorations(unsigned* instructionsData, unsigned& instructionsDataIndex, std::vector<unsigned>& structtypeindices, std::vector<unsigned>& structidindices, std::vector<Instruction>& newinstructions, std::vector<Var>& uniforms,
		std::map<unsigned, unsigned>& pointers, std::vector<Var>& invars, std::vector<Var>& outvars, std::vector<Var>& images, std::map<unsigned, unsigned> arraySizes, ShaderStage stage) {

		unsigned location = 0;
		for (auto var : invars) {
			Instruction newinst(OpDecorate, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = var.id;
			instructionsData[instructionsDataIndex++] = DecorationLocation;
			instructionsData[instructionsDataIndex++] = location;
			newinstructions.push_back(newinst);
			++location;
		}
		location = 0;
		for (auto var : outvars) {
			Instruction newinst(OpDecorate, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = var.id;
			instructionsData[instructionsDataIndex++] = DecorationLocation;
			instructionsData[instructionsDataIndex++] = location;
			newinstructions.push_back(newinst);
			++location;
		}
		unsigned binding = 2;
		for (auto var : images) {
			Instruction newinst(OpDecorate, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = var.id;
			instructionsData[instructionsDataIndex++] = DecorationBinding;
			instructionsData[instructionsDataIndex++] = binding;
			newinstructions.push_back(newinst);
			++binding;
		}
		unsigned offset = 0;
		for (unsigned i = 0; i < uniforms.size(); ++i) {
			Instruction nonwr(OpMemberDecorate, &instructionsData[instructionsDataIndex], 3);
			structtypeindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			instructionsData[instructionsDataIndex++] = i;
			instructionsData[instructionsDataIndex++] = DecorationNonWritable;
			newinstructions.push_back(nonwr);

			Instruction newinst(OpMemberDecorate, &instructionsData[instructionsDataIndex], 4);
			structtypeindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			instructionsData[instructionsDataIndex++] = i;
			instructionsData[instructionsDataIndex++] = DecorationOffset;
			unsigned int* offsetPointer = &instructionsData[instructionsDataIndex++];
			newinstructions.push_back(newinst);

			int utype = pointers[uniforms[i].type];

			if (utype == mat2type || utype == mat3type || utype == mat4type) {
				Instruction dec2(OpMemberDecorate, &instructionsData[instructionsDataIndex], 3);
				structtypeindices.push_back(instructionsDataIndex);
				instructionsData[instructionsDataIndex++] = 0;
				instructionsData[instructionsDataIndex++] = i;
				instructionsData[instructionsDataIndex++] = DecorationColMajor;
				newinstructions.push_back(dec2);

				Instruction dec3(OpMemberDecorate, &instructionsData[instructionsDataIndex], 4);
				structtypeindices.push_back(instructionsDataIndex);
				instructionsData[instructionsDataIndex++] = 0;
				instructionsData[instructionsDataIndex++] = i;
				instructionsData[instructionsDataIndex++] = DecorationMatrixStride;
				instructionsData[instructionsDataIndex++] = 16;
				newinstructions.push_back(dec3);
			}
			else if (utype == floatarraytype || utype == vec2arraytype || utype == vec3arraytype || utype == vec4arraytype) {
				Instruction dec3(OpDecorate, &instructionsData[instructionsDataIndex], 3);
				instructionsData[instructionsDataIndex++] = utype;
				instructionsData[instructionsDataIndex++] = DecorationArrayStride;
				if (utype == floatarraytype) {
					instructionsData[instructionsDataIndex++] = 1 * 4;
				}
				if (utype == vec2arraytype) {
					instructionsData[instructionsDataIndex++] = 2 * 4;
				}
				if (utype == vec3arraytype) {
					instructionsData[instructionsDataIndex++] = 3 * 4;
				}
				if (utype == vec4arraytype) {
					instructionsData[instructionsDataIndex++] = 4 * 4;
				}
				newinstructions.push_back(dec3);
			}

			if (utype == booltype || utype == inttype || utype == floattype || utype == uinttype) {
				offset = alignOffset(offset, 4);
			}
			else if (utype == vec2type) {
				offset = alignOffset(offset, 8);
			}
			else if (utype == vec3type) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == vec4type) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == mat2type) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == mat3type) {
				offset = alignOffset(offset, 48);
			}
			else if (utype == mat4type) {
				offset = alignOffset(offset, 64);
			}
			else if (utype == floatarraytype) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == vec2arraytype) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == vec3arraytype) {
				offset = alignOffset(offset, 16);
			}
			else if (utype == vec4arraytype) {
				offset = alignOffset(offset, 16);
			}

			*offsetPointer = offset;

			if (utype == booltype || utype == inttype || utype == floattype || utype == uinttype) {
				offset += 4;
			}
			else if (utype == vec2type) {
				offset += 8;
			}
			else if (utype == vec3type) {
				offset += 12;
			}
			else if (utype == vec4type) {
				offset += 16;
			}
			else if (utype == mat2type) {
				offset += 16;
			}
			else if (utype == mat3type) {
				offset += 48; // 36 + 12 padding for DecorationMatrixStride of 16
			}
			else if (utype == mat4type) offset += 64;
			else if (utype == floatarraytype) {
				offset += arraySizes[floatarraytype] * 4;
				if (offset % 8 != 0) {
					offset += 4;
				}
			}
			else if (utype == vec2arraytype) {
				offset += arraySizes[vec2arraytype] * 4 * 2;
			}
			else if (utype == vec3arraytype) {
				offset += arraySizes[vec3arraytype] * 4 * 3;
				if (offset % 8 != 0) {
					offset += 4;
				}
			}
			else if (utype == vec4arraytype) {
				offset += arraySizes[vec4arraytype] * 4 * 4;
			}
			else {
				offset += 1; // Type not handled
			}
		}
		if (uniforms.size() > 0) {
			Instruction decbind(OpDecorate, &instructionsData[instructionsDataIndex], 3);
			structidindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			instructionsData[instructionsDataIndex++] = DecorationBinding;
			instructionsData[instructionsDataIndex++] = stage == StageVertex ? 0 : 1;
			newinstructions.push_back(decbind);

			Instruction decdescset(OpDecorate, &instructionsData[instructionsDataIndex], 3);
			structidindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			instructionsData[instructionsDataIndex++] = DecorationDescriptorSet;
			instructionsData[instructionsDataIndex++] = 0;
			newinstructions.push_back(decdescset);

			Instruction dec1(OpDecorate, &instructionsData[instructionsDataIndex], 2);
			structtypeindices.push_back(instructionsDataIndex);
			instructionsData[instructionsDataIndex++] = 0;
			instructionsData[instructionsDataIndex++] = DecorationBufferBlock;
			newinstructions.push_back(dec1);
		}
	}

	void outputTypes(unsigned* instructionsData, unsigned& instructionsDataIndex, std::vector<unsigned>& structtypeindices, std::vector<unsigned>& structidindices, unsigned& structvarindex, std::vector<Instruction>& newinstructions, std::vector<Var>& uniforms,
		std::map<unsigned, unsigned>& pointers, std::map<unsigned, unsigned>& constants, unsigned& currentId, unsigned& structid, unsigned& floatpointertype,
		unsigned& dotfive, unsigned& two, unsigned& three, unsigned& tempposition, ShaderStage stage) {
		if (uniforms.size() > 0) {
			Instruction typestruct(OpTypeStruct, &instructionsData[instructionsDataIndex], 1 + (unsigned)uniforms.size());
			unsigned structtype = instructionsData[instructionsDataIndex++] = currentId++;
			for (unsigned i = 0; i < uniforms.size(); ++i) {
				instructionsData[instructionsDataIndex++] = pointers[uniforms[i].type];
			}
			for (auto index : structtypeindices) instructionsData[index] = structtype;
			newinstructions.push_back(typestruct);
			Instruction typepointer(OpTypePointer, &instructionsData[instructionsDataIndex], 3);
			unsigned pointertype = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = StorageClassUniform;
			instructionsData[instructionsDataIndex++] = structtype;
			newinstructions.push_back(typepointer);
			Instruction variable(OpVariable, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = pointertype;
			structid = instructionsData[instructionsDataIndex++] = currentId++;
			for (auto index : structidindices) instructionsData[index] = structid;
			instructionsData[structvarindex] = structid;
			instructionsData[instructionsDataIndex++] = StorageClassUniform;
			newinstructions.push_back(variable);

			if (uinttype == 0) {
				Instruction typeint(OpTypeInt, &instructionsData[instructionsDataIndex], 3);
				uinttype = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = 32;
				instructionsData[instructionsDataIndex++] = 0;
				newinstructions.push_back(typeint);
			}
			for (unsigned i = 0; i < uniforms.size(); ++i) {
				Instruction constant(OpConstant, &instructionsData[instructionsDataIndex], 3);
				instructionsData[instructionsDataIndex++] = uinttype;
				unsigned constantid = currentId++;
				instructionsData[instructionsDataIndex++] = constantid;
				constants[i] = constantid;
				instructionsData[instructionsDataIndex++] = i;
				newinstructions.push_back(constant);
				Instruction typepointer(OpTypePointer, &instructionsData[instructionsDataIndex], 3);
				uniforms[i].pointertype = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = StorageClassUniform;
				instructionsData[instructionsDataIndex++] = pointers[uniforms[i].type];
				newinstructions.push_back(typepointer);
			}
		}

		if (stage == StageVertex) {
			if (floattype == 0) {
				Instruction floaty(OpTypeFloat, &instructionsData[instructionsDataIndex], 2);
				floattype = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = 32;
				newinstructions.push_back(floaty);
			}

			Instruction floatpointer(OpTypePointer, &instructionsData[instructionsDataIndex], 3);
			floatpointertype = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = StorageClassPrivate;
			instructionsData[instructionsDataIndex++] = floattype;
			newinstructions.push_back(floatpointer);

			Instruction dotfiveconstant(OpConstant, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = floattype;
			dotfive = instructionsData[instructionsDataIndex++] = currentId++;
			*(float*)&instructionsData[instructionsDataIndex++] = 0.5f;
			newinstructions.push_back(dotfiveconstant);

			if (uinttype == 0) {
				Instruction inty(OpTypeInt, &instructionsData[instructionsDataIndex], 3);
				uinttype = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = 32;
				instructionsData[instructionsDataIndex++] = 0;
				newinstructions.push_back(inty);
			}

			Instruction twoconstant(OpConstant, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = uinttype;
			two = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = 2;
			newinstructions.push_back(twoconstant);

			Instruction threeconstant(OpConstant, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = uinttype;
			three = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = 3;
			newinstructions.push_back(threeconstant);

			if (vec4type == 0) {
				Instruction vec4(OpTypeVector, &instructionsData[instructionsDataIndex], 3);
				vec4type = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = floattype;
				instructionsData[instructionsDataIndex++] = 4;
				newinstructions.push_back(vec4);
			}

			Instruction vec4pointer(OpTypePointer, &instructionsData[instructionsDataIndex], 3);
			unsigned vec4pointertype = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = StorageClassPrivate;
			instructionsData[instructionsDataIndex++] = vec4type;
			newinstructions.push_back(vec4pointer);

			Instruction varinst(OpVariable, &instructionsData[instructionsDataIndex], 3);
			instructionsData[instructionsDataIndex++] = vec4pointertype;
			tempposition = instructionsData[instructionsDataIndex++] = currentId++;
			instructionsData[instructionsDataIndex++] = StorageClassPrivate;
			newinstructions.push_back(varinst);
		}
	}
}

void SpirVTranslator::outputCode(const Target& target, const char* sourcefilename, const char* filename, char* output, std::map<std::string, int>& attributes) {
	booltype = 0;
	inttype = 0;
	uinttype = 0;
	floattype = 0;
	vec4type = 0;
	vec3type = 0;
	vec2type = 0;
	mat4type = 0;
	mat3type = 0;
	mat2type = 0;
	floatarraytype = 0;
	vec2arraytype = 0;
	vec3arraytype = 0;
	vec4arraytype = 0;

	using namespace spv;

	std::map<unsigned, std::string> names;
	std::vector<Var> invars;
	std::vector<Var> outvars;
	std::vector<Var> tempvars;
	std::vector<Var> images;
	std::vector<Var> uniforms;
	std::map<unsigned, bool> imageTypes;
	std::map<unsigned, unsigned> pointers;
	std::map<unsigned, unsigned> constants;
	std::map<unsigned, unsigned> accessChains;
	std::map<unsigned, unsigned> arraySizeConstants;
	std::map<unsigned, unsigned> arraySizes;
	unsigned position;

	for (unsigned i = 0; i < instructions.size(); ++i) {
		Instruction& inst = instructions[i];
		switch (inst.opcode) {
		case OpName: {
			unsigned id = inst.operands[0];
			if (strcmp(inst.string, "") != 0) {
				names[id] = inst.string;
			}
			break;
		}
		case OpDecorate: {
			unsigned id = inst.operands[0];
			Decoration decoration = (Decoration)inst.operands[1];
			if (decoration == DecorationBuiltIn) {
				names[id] = "";
			}
			break;
		}
		case OpAccessChain: {
			unsigned id = inst.operands[1];
			unsigned accessId = inst.operands[2];
			accessChains[id] = accessId;
			break;
		}
		case OpTypeSampledImage: {
			unsigned id = inst.operands[0];
			imageTypes[id] = true;
			break;
		}
		case OpTypeImage: {
			unsigned id = inst.operands[0];
			imageTypes[id] = true;
			break;
		}
		case OpTypeSampler: {
			unsigned id = inst.operands[0];
			imageTypes[id] = true;
			break;
		}
		case OpTypePointer: {
			unsigned id = inst.operands[0];
			unsigned type = inst.operands[2];
			if (imageTypes[type]) imageTypes[id] = true;
			pointers[id] = type;
			break;
		}
		case OpTypeBool: {
			unsigned id = inst.operands[0];
			booltype = id;
			break;
		}
		case OpTypeInt: {
			unsigned id = inst.operands[0];
			unsigned width = inst.operands[1];
			unsigned signedness = inst.operands[2];
			if (width == 32 && signedness == 1) {
				inttype = id;
			}
			else if (width == 32 && signedness == 0) {
				uinttype = id;
			}
			break;
		}
		case OpTypeFloat: {
			unsigned id = inst.operands[0];
			unsigned width = inst.operands[1];
			if (width == 32) {
				floattype = id;
			}
			break;
		}
		case OpTypeVector: {
			unsigned id = inst.operands[0];
			unsigned componentType = inst.operands[1];
			unsigned componentCount = inst.operands[2];
			if (componentType == floattype) {
				if (componentCount == 4) {
					vec4type = id;
				}
				else if (componentCount == 3) {
					vec3type = id;
				}
				else if (componentCount == 2) {
					vec2type = id;
				}
			}
			break;
		}
		case OpTypeMatrix: {
			unsigned id = inst.operands[0];
			// unsigned columnType = inst.operands[1];
			unsigned columnCount = inst.operands[2];
			if (columnCount == 4) {
				mat4type = id;
			}
			else if (columnCount == 3) {
				mat3type = id;
			}
			else if (columnCount == 2) {
				mat2type = id;
			}

			break;
		}
		case OpConstant: {
			unsigned id = inst.operands[1];
			if (arraySizeConstants[id] == 0) {
				arraySizeConstants[id] = inst.operands[2];
			}
			break;
		}
		case OpTypeArray: {
			unsigned id = inst.operands[0];
			unsigned componentType = inst.operands[1];
			if (imageTypes[componentType]) {
				imageTypes[id] = true;
			}
			arraySizes[id] = arraySizeConstants[inst.operands[2]];
			if (componentType == floattype) {
				floatarraytype = id;
			}
			else if (componentType == vec2type) {
				vec2arraytype = id;
			}
			else if (componentType == vec3type) {
				vec3arraytype = id;
			}
			else if (componentType == vec4type) {
				vec4arraytype = id;
			}
			break;
		}
		case OpVariable: {
			unsigned type = inst.operands[0];
			unsigned id = inst.operands[1];
			StorageClass storage = (StorageClass)inst.operands[2];
			Var var;
			var.name = names[id];
			var.id = id;
			var.type = type;
			if (var.name != "") {
				if (storage == StorageClassInput) invars.push_back(var);
				if (storage == StorageClassOutput) outvars.push_back(var);
				if (storage == StorageClassUniformConstant) {
					if (imageTypes[type]) {
						images.push_back(var);
					}
					else {
						uniforms.push_back(var);
					}
				}
			}
			else tempvars.push_back(var);
			break;
		}
		case OpStore: {
			unsigned to = inst.operands[0];
			int accessId = accessChains[to];
			for (unsigned j = 0; j < tempvars.size(); ++j) {
				if (tempvars[j].id == accessId) {
					for (const auto& pair : pointers) {
						if (tempvars[j].type == pair.first) {
							if (strcmp(names[pair.second].c_str(), "gl_PerVertex") == 0) {
								position = to;
								break;
							}
						}
					}
				}
			}
			break;
		}
		}
	}

	std::sort(invars.begin(), invars.end(), varcompare);
	std::sort(outvars.begin(), outvars.end(), varcompare);
	std::sort(images.begin(), images.end(), varcompare);

	SpirVState state = SpirVStart;
	std::vector<Instruction> newinstructions;
	unsigned instructionsData[4096];
	unsigned instructionsDataIndex = 0;
	unsigned currentId = bound;
	unsigned structid;
	std::vector<unsigned> structtypeindices, structidindices;
	unsigned structvarindex;
	unsigned tempposition;
	unsigned floatpointertype;
	unsigned dotfive;
	unsigned two;
	unsigned three;
	bool namesInserted = false;
	bool decorationsInserted = false;
	for (unsigned i = 0; i < instructions.size(); ++i) {
		Instruction& inst = instructions[i];

		switch (state) {
		case SpirVStart:
			if (isDebugInformation(inst)) {
				state = SpirVDebugInformation;
				if (!namesInserted) {
					outputNames(instructionsData, instructionsDataIndex, structtypeindices, structvarindex, newinstructions, uniforms);
					namesInserted = true;
				}
			}
			break;
		case SpirVDebugInformation:
			if (isAnnotation(inst)) {
				state = SpirVAnnotations;
				if (!namesInserted) {
					outputNames(instructionsData, instructionsDataIndex, structtypeindices, structvarindex, newinstructions, uniforms);
					namesInserted = true;
				}
				if (!decorationsInserted) {
					outputDecorations(instructionsData, instructionsDataIndex, structtypeindices, structidindices, newinstructions, uniforms, pointers, invars, outvars, images, arraySizes, stage);
					decorationsInserted = true;
				}
			}
			if (isType(inst)) {
				state = SpirVTypes;

				if (!namesInserted) {
					outputNames(instructionsData, instructionsDataIndex, structtypeindices, structvarindex, newinstructions, uniforms);
					namesInserted = true;
				}
				if (!decorationsInserted) {
					outputDecorations(instructionsData, instructionsDataIndex, structtypeindices, structidindices, newinstructions, uniforms, pointers, invars, outvars, images, arraySizes, stage);
					decorationsInserted = true;
				}
			}
			break;
		case SpirVAnnotations:
			if (!isAnnotation(inst)) {
				state = SpirVTypes;

				if (!namesInserted) {
					outputNames(instructionsData, instructionsDataIndex, structtypeindices, structvarindex, newinstructions, uniforms);
					namesInserted = true;
				}
				if (!decorationsInserted) {
					outputDecorations(instructionsData, instructionsDataIndex, structtypeindices, structidindices, newinstructions, uniforms, pointers, invars, outvars, images, arraySizes, stage);
					decorationsInserted = true;
				}
			}
			break;
		case SpirVTypes:
			if (inst.opcode == OpFunction) {
				outputTypes(instructionsData, instructionsDataIndex, structtypeindices, structidindices, structvarindex, newinstructions, uniforms, pointers, constants, currentId,
					structid, floatpointertype, dotfive, two, three, tempposition, stage);
				state = SpirVFunctions;
			}
			break;
		}

		if (inst.opcode == OpEntryPoint) {
			unsigned executionModel = inst.operands[0];
			if (executionModel == 4) { // Fragment Shader
				unsigned i = 2;
				for (; ; ++i) {
					char* chars = (char*)&inst.operands[i];
					if (chars[0] == 0 || chars[1] == 0 || chars[2] == 0 || chars[3] == 0) break;
				}
				Instruction newinst(OpEntryPoint, &instructionsData[instructionsDataIndex], 0);
				unsigned length = 0;
				for (unsigned i2 = 0; i2 <= i; ++i2) {
					instructionsData[instructionsDataIndex++] = inst.operands[i2];
					++length;
				}
				for (auto var : invars) {
					instructionsData[instructionsDataIndex++] = var.id;
					++length;
				}
				for (auto var : outvars) {
					instructionsData[instructionsDataIndex++] = var.id;
					++length;
				}
				newinst.length = length;
				newinstructions.push_back(newinst);
			}
			else {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpName) {
			bool isInput = false;
			for (auto var : invars) {
				if (inst.operands[0] == var.id) {
					isInput = true;
				}
			}

			bool isImage = false;
			for (auto image : images) {
				if (inst.operands[0] == image.id) {
					isImage = true;
				}
			}

			if (isInput || isImage) {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpMemberName) {

		}
		else if (inst.opcode == OpSource) {

		}
		else if (inst.opcode == OpSourceContinued) {

		}
		else if (inst.opcode == OpSourceExtension) {

		}
		else if (inst.opcode == OpExecutionMode) {
			unsigned executionMode = inst.operands[1];
			if (executionMode == 8) {
				Instruction copy = inst;
				copy.operands[1] = 7;
				newinstructions.push_back(copy);
			}
			else {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpTypeImage) {
			Instruction copy = inst;
			if (stage == StageCompute) {
				copy.length -= 1;
			}
			else {
				copy.length -= 2;
			}
			newinstructions.push_back(copy);
		}
		else if (inst.opcode == OpVariable) {
			unsigned type = inst.operands[0];
			unsigned id = inst.operands[1];
			StorageClass storage = (StorageClass)inst.operands[2];
			if (storage != StorageClassUniformConstant || imageTypes[type]) {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpTypePointer) {
			// Putting uniforms into a uniform-block changes the types from UniformConstant to Uniform
			unsigned resultId = inst.operands[0];
			unsigned storageClass = inst.operands[1];
			unsigned typeId = inst.operands[2];

			bool replaced = false;

			if (storageClass == 0) {
				if (!imageTypes[typeId]) {
					Instruction typePointer(OpTypePointer, &instructionsData[instructionsDataIndex], 3);
					instructionsData[instructionsDataIndex++] = resultId;
					instructionsData[instructionsDataIndex++] = 2; // Uniform
					instructionsData[instructionsDataIndex++] = typeId;
					newinstructions.push_back(typePointer);
					replaced = true;
				}
			}

			if (!replaced) {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpAccessChain) {
			// replace all accesses to global uniforms
			unsigned resultType = inst.operands[0];
			unsigned resultId = inst.operands[1];
			unsigned base = inst.operands[2];

			Var uniform;
			unsigned index;
			bool found = false;
			for (unsigned i = 0; i < uniforms.size(); ++i) {
				if (uniforms[i].id == base) {
					uniform = uniforms[i];
					index = i;
					found = true;
					break;
				}
			}

			if (found) {
				// OpAccessChain can be a chain of any size so we just sneak in the access to the
				// uniform-struct at the front
				Instruction access(OpAccessChain, &instructionsData[instructionsDataIndex], inst.length + 1);
				instructionsData[instructionsDataIndex++] = resultType;
				instructionsData[instructionsDataIndex++] = resultId;
				instructionsData[instructionsDataIndex++] = structid;
				instructionsData[instructionsDataIndex++] = constants[index];
				for (unsigned i = 3; i < inst.length; ++i) {
					instructionsData[instructionsDataIndex++] = inst.operands[i];
				}
				newinstructions.push_back(access);
			}
			else {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpLoad) {
			// replace all loads from global uniforms
			unsigned type = inst.operands[0];
			unsigned id = inst.operands[1];
			unsigned pointer = inst.operands[2];

			Var uniform;
			unsigned index;
			bool found = false;
			for (unsigned i = 0; i < uniforms.size(); ++i) {
				if (uniforms[i].id == pointer) {
					uniform = uniforms[i];
					index = i;
					found = true;
					break;
				}
			}

			if (found) {
				Instruction access(OpAccessChain, &instructionsData[instructionsDataIndex], 4);
				instructionsData[instructionsDataIndex++] = uniform.pointertype;
				unsigned newpointer = instructionsData[instructionsDataIndex++] = currentId++;
				instructionsData[instructionsDataIndex++] = structid;
				instructionsData[instructionsDataIndex++] = constants[index];
				newinstructions.push_back(access);
				Instruction load(OpLoad, &instructionsData[instructionsDataIndex], 3);
				instructionsData[instructionsDataIndex++] = type;
				instructionsData[instructionsDataIndex++] = id;
				instructionsData[instructionsDataIndex++] = newpointer;
				newinstructions.push_back(load);
			}
			else {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpStore) {
			if (stage == StageVertex) {
				//gl_Position.z = (gl_Position.z + gl_Position.w) * 0.5;
				unsigned to = inst.operands[0];
				unsigned from = inst.operands[1];
				if (to == position) {
					//OpStore tempposition from
					Instruction store1(OpStore, &instructionsData[instructionsDataIndex], 2);
					instructionsData[instructionsDataIndex++] = tempposition;
					instructionsData[instructionsDataIndex++] = from;
					newinstructions.push_back(store1);

					//%27 = OpAccessChain floatpointer tempposition two
					Instruction access1(OpAccessChain, &instructionsData[instructionsDataIndex], 4);
					instructionsData[instructionsDataIndex++] = floatpointertype;
					unsigned _27 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = tempposition;
					instructionsData[instructionsDataIndex++] = two;
					newinstructions.push_back(access1);

					//%28 = OpLoad float %27
					Instruction load1(OpLoad, &instructionsData[instructionsDataIndex], 3);
					instructionsData[instructionsDataIndex++] = floattype;
					unsigned _28 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = _27;
					newinstructions.push_back(load1);

					//%30 = OpAccessChain floatpointer tempposition three
					Instruction access2(OpAccessChain, &instructionsData[instructionsDataIndex], 4);
					instructionsData[instructionsDataIndex++] = floatpointertype;
					unsigned _30 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = tempposition;
					instructionsData[instructionsDataIndex++] = three;
					newinstructions.push_back(access2);

					//%31 = OpLoad float %30
					Instruction load2(OpLoad, &instructionsData[instructionsDataIndex], 3);
					instructionsData[instructionsDataIndex++] = floattype;
					unsigned _31 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = _30;
					newinstructions.push_back(load2);

					//%32 = OpFAdd float %28 %31
					Instruction add(OpFAdd, &instructionsData[instructionsDataIndex], 4);
					instructionsData[instructionsDataIndex++] = floattype;
					unsigned _32 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = _28;
					instructionsData[instructionsDataIndex++] = _31;
					newinstructions.push_back(add);

					//%34 = OpFMul float %32 dotfive
					Instruction mult(OpFMul, &instructionsData[instructionsDataIndex], 4);
					instructionsData[instructionsDataIndex++] = floattype;
					unsigned _34 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = _32;
					instructionsData[instructionsDataIndex++] = dotfive;
					newinstructions.push_back(mult);

					//%35 = OpAccessChain floatpointer tempposition two
					Instruction access3(OpAccessChain, &instructionsData[instructionsDataIndex], 4);
					instructionsData[instructionsDataIndex++] = floatpointertype;
					unsigned _35 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = tempposition;
					instructionsData[instructionsDataIndex++] = two;
					newinstructions.push_back(access3);

					//OpStore %35 %34
					Instruction store2(OpStore, &instructionsData[instructionsDataIndex], 2);
					instructionsData[instructionsDataIndex++] = _35;
					instructionsData[instructionsDataIndex++] = _34;
					newinstructions.push_back(store2);

					//%38 = OpLoad vec4 tempposition
					Instruction load3(OpLoad, &instructionsData[instructionsDataIndex], 3);
					instructionsData[instructionsDataIndex++] = vec4type;
					unsigned _38 = instructionsData[instructionsDataIndex++] = currentId++;
					instructionsData[instructionsDataIndex++] = tempposition;
					newinstructions.push_back(load3);

					//OpStore position %38
					Instruction store3(OpStore, &instructionsData[instructionsDataIndex], 2);
					instructionsData[instructionsDataIndex++] = position;
					instructionsData[instructionsDataIndex++] = _38;
					newinstructions.push_back(store3);
				}
				else {
					newinstructions.push_back(inst);
				}
			}
			else {
				newinstructions.push_back(inst);
			}
		}
		else if (inst.opcode == OpDecorate) {
			Decoration decoration = (Decoration)inst.operands[1];
			if (decoration == DecorationBuiltIn && inst.operands[2] == BuiltInVertexId) {
				// VertexId is not allowed in Vulkan
				Instruction copy = inst;
				copy.operands[2] = BuiltInVertexIndex;
				newinstructions.push_back(copy);
			}
			else if (decoration != DecorationBinding) {
				newinstructions.push_back(inst);
			}
		}
		else {
			newinstructions.push_back(inst);
		}
	}

	bound = currentId + 1;

	std::vector<uint32_t> spirv;
	outputLength = writeInstructions(spirv, newinstructions);
	std::vector<uint32_t> optimizedSpirv;
	optimizedSpirv = spirv;

	outputLength = (int)(optimizedSpirv.size() * 4);
	if (output) {
		memcpy(output, optimizedSpirv.data(), outputLength);
	}
	else {
		FILE* file = fopen(filename, "wb");
		fwrite(optimizedSpirv.data(), 4, optimizedSpirv.size(), file);
		fclose(file);
	}
}

enum TOptions {
	EOptionNone = 0,
	EOptionIntermediate = (1 << 0),
	EOptionSuppressInfolog = (1 << 1),
	EOptionMemoryLeakMode = (1 << 2),
	EOptionRelaxedErrors = (1 << 3),
	EOptionGiveWarnings = (1 << 4),
	EOptionLinkProgram = (1 << 5),
	EOptionMultiThreaded = (1 << 6),
	EOptionDumpConfig = (1 << 7),
	EOptionDumpReflection = (1 << 8),
	EOptionSuppressWarnings = (1 << 9),
	EOptionDumpVersions = (1 << 10),
	EOptionSpv = (1 << 11),
	EOptionHumanReadableSpv = (1 << 12),
	EOptionVulkanRules = (1 << 13),
	EOptionDefaultDesktop = (1 << 14),
	EOptionOutputPreprocessed = (1 << 15),
	EOptionOutputHexadecimal = (1 << 16),
	EOptionReadHlsl = (1 << 17),
	EOptionCascadingErrors = (1 << 18),
	EOptionAutoMapBindings = (1 << 19),
	EOptionFlattenUniformArrays = (1 << 20),
	EOptionNoStorageFormat = (1 << 21),
	EOptionKeepUncalled = (1 << 22),
};

enum TFailCode {
	ESuccess = 0,
	EFailUsage,
	EFailCompile,
	EFailLink,
	EFailCompilerCreate,
	EFailThreadCreate,
	EFailLinkerCreate
};

EShLanguage FindLanguage(const std::string& name, bool parseSuffix = true);
void CompileFile(const char* fileName, ShHandle);
void usage();
void FreeFileData(char** data);
char** ReadFileData(const char* fileName);

bool CompileFailed = false;
bool LinkFailed = false;
static bool quiet = false;
int NumShaderStrings;
TBuiltInResource Resources;
std::string ConfigFile;

void ProcessConfigFile() {
	char** configStrings = 0;
	char* config = 0;
	if (ConfigFile.size() > 0) {
		configStrings = ReadFileData(ConfigFile.c_str());
		if (configStrings)
			config = *configStrings;
		else {
			printf("Error opening configuration file; will instead use the default configuration\n");
			usage();
		}
	}

	if (config == 0) {
		Resources = glslang::DefaultTBuiltInResource;
		return;
	}

	glslang::DecodeResourceLimits(&Resources, config);

	if (configStrings)
		FreeFileData(configStrings);
	else
		delete[] config;
}

glslang::TWorklist Worklist;
glslang::TWorkItem** Work = 0;
int NumWorkItems = 0;

int Options = 0;
const char* ExecutableName = nullptr;
const char* binaryFileName = nullptr;
const char* entryPointName = nullptr;
const char* sourceEntryPointName = nullptr;
const char* shaderStageName = nullptr;
const char* variableName = nullptr;

std::array<unsigned int, EShLangCount> baseSamplerBinding;
std::array<unsigned int, EShLangCount> baseTextureBinding;
std::array<unsigned int, EShLangCount> baseImageBinding;
std::array<unsigned int, EShLangCount> baseUboBinding;
std::array<unsigned int, EShLangCount> baseSsboBinding;

bool SetConfigFile(const std::string& name) {
	if (name.size() < 5)
		return false;

	if (name.compare(name.size() - 5, 5, ".conf") == 0) {
		ConfigFile = name;
		return true;
	}

	return false;
}

void Error(const char* message) {
	printf("%s: Error %s (use -h for usage)\n", ExecutableName, message);
	exit(EFailUsage);
}

void SetMessageOptions(EShMessages& messages) {
	if (Options & EOptionRelaxedErrors)
		messages = (EShMessages)(messages | EShMsgRelaxedErrors);
	if (Options & EOptionIntermediate)
		messages = (EShMessages)(messages | EShMsgAST);
	if (Options & EOptionSuppressWarnings)
		messages = (EShMessages)(messages | EShMsgSuppressWarnings);
	if (Options & EOptionSpv)
		messages = (EShMessages)(messages | EShMsgSpvRules);
	if (Options & EOptionVulkanRules)
		messages = (EShMessages)(messages | EShMsgVulkanRules);
	if (Options & EOptionOutputPreprocessed)
		messages = (EShMessages)(messages | EShMsgOnlyPreprocessor);
	if (Options & EOptionReadHlsl)
		messages = (EShMessages)(messages | EShMsgReadHlsl);
	if (Options & EOptionCascadingErrors)
		messages = (EShMessages)(messages | EShMsgCascadingErrors);
	if (Options & EOptionKeepUncalled)
		messages = (EShMessages)(messages | EShMsgKeepUncalled);
}

void PutsIfNonEmpty(const char* str) {
	if (str && str[0]) {
		puts(str);
	}
}

void StderrIfNonEmpty(const char* str) {
	if (str && str[0]) {
		fprintf(stderr, "%s\n", str);
	}
}

struct ShaderCompUnit {
	EShLanguage stage;
	std::string fileName;
	char** text;             // memory owned/managed externally
	const char* fileNameList[1];

	// Need to have a special constructors to adjust the fileNameList, since back end needs a list of ptrs
	ShaderCompUnit(EShLanguage istage, std::string& ifileName, char** itext)
	{
		stage = istage;
		fileName = ifileName;
		text = itext;
		fileNameList[0] = fileName.c_str();
	}

	ShaderCompUnit(const ShaderCompUnit& rhs)
	{
		stage = rhs.stage;
		fileName = rhs.fileName;
		text = rhs.text;
		fileNameList[0] = fileName.c_str();
	}

};

class NullIncluder : public glslang::TShader::Includer {
public:
	NullIncluder() {}

	IncludeResult* includeSystem(const char* headerName, const char* includerName, size_t inclusionDepth) override {
		return includeLocal(headerName, includerName, inclusionDepth);
	}

	IncludeResult* includeLocal(const char* headerName, const char* includerName, size_t inclusionDepth) override {
		return nullptr;
	}

	void releaseInclude(IncludeResult* result) override {}
};

krafix::ShaderStage shLanguageToShaderStage(EShLanguage lang) {
	switch (lang) {
	case EShLangVertex: return krafix::StageVertex;
	case EShLangGeometry: return krafix::StageGeometry;
	case EShLangFragment: return krafix::StageFragment;
	case EShLangCompute: return krafix::StageCompute;
	case EShLangCount:
	default:
		return krafix::StageCompute;
	}
}

static void preprocessSpirv(std::vector<unsigned int>& spirv) {
	unsigned binding = 0;
	for (unsigned index = 0; index < spirv.size(); ++index) {
		int wordCount = spirv[index] >> 16;
		int opcode = spirv[index] & 0xffff;

		unsigned* operands = wordCount > 1 ? &spirv[index + 1] : NULL;
		int length = wordCount - 1;

		if (opcode == 71 && length >= 2) {
			if (operands[1] == 33) {
				operands[2] = binding++;
			}
		}
	}
}

void CompileAndLinkShaderUnits(std::vector<ShaderCompUnit> compUnits, krafix::Target target, const char* sourcefilename, const char* filename, const char* tempdir, char* output, int* length,
	glslang::TShader::Includer& includer, const char* defines, bool relax) {

	std::list<glslang::TShader*> shaders;
	EShMessages messages = EShMsgDefault;
	SetMessageOptions(messages);

	glslang::TProgram& program = *new glslang::TProgram;
	for (auto it = compUnits.cbegin(); it != compUnits.cend(); ++it) {
		const auto& compUnit = *it;
		glslang::TShader* shader = new glslang::TShader(compUnit.stage);
		shader->setStringsWithLengthsAndNames(compUnit.text, NULL, compUnit.fileNameList, 1);
		if (entryPointName) // HLSL todo: this needs to be tracked per compUnits
			shader->setEntryPoint(entryPointName);
		if (sourceEntryPointName)
			shader->setSourceEntryPoint(sourceEntryPointName);

		shader->setShiftSamplerBinding(baseSamplerBinding[compUnit.stage]);
		shader->setShiftTextureBinding(baseTextureBinding[compUnit.stage]);
		shader->setShiftImageBinding(baseImageBinding[compUnit.stage]);
		shader->setShiftUboBinding(baseUboBinding[compUnit.stage]);
		shader->setShiftSsboBinding(baseSsboBinding[compUnit.stage]);
		// shader->setFlattenUniformArrays((Options & EOptionFlattenUniformArrays) != 0);
		shader->setNoStorageFormat((Options & EOptionNoStorageFormat) != 0);
		shader->setPreamble(defines);

		if (Options & EOptionAutoMapBindings)
			shader->setAutoMapBindings(true);

		shaders.push_back(shader);

		const int defaultVersion = Options & EOptionDefaultDesktop ? 110 : 100;

		if (Options & EOptionOutputPreprocessed) {
			std::string str;
			//glslang::TShader::ForbidIncluder includer;
			if (shader->preprocess(&Resources, defaultVersion, ENoProfile, false, false,
				messages, &str, includer)) {
				PutsIfNonEmpty(str.c_str());
			}
			else {
				CompileFailed = true;
			}
			StderrIfNonEmpty(shader->getInfoLog());
			StderrIfNonEmpty(shader->getInfoDebugLog());
			continue;
		}
		if (!shader->parse(&Resources, defaultVersion, ENoProfile, false, false, messages, includer))
			CompileFailed = true;

		program.addShader(shader);

		if (!(Options & EOptionSuppressInfolog) &&
			!(Options & EOptionMemoryLeakMode)) {
			//PutsIfNonEmpty(compUnit.fileName.c_str());
			PutsIfNonEmpty(shader->getInfoLog());
			PutsIfNonEmpty(shader->getInfoDebugLog());
		}
	}

	// Link
	if (!(Options & EOptionOutputPreprocessed) && !program.link(messages))
		LinkFailed = true;

	// Map IO
	if (Options & EOptionSpv) {
		if (!program.mapIO())
			LinkFailed = true;
	}

	// Report
	if (!(Options & EOptionSuppressInfolog) &&
		!(Options & EOptionMemoryLeakMode)) {
		PutsIfNonEmpty(program.getInfoLog());
		PutsIfNonEmpty(program.getInfoDebugLog());
	}

	// Reflect
	if (Options & EOptionDumpReflection) {
		program.buildReflection();
		program.dumpReflection();
	}

	// Dump SPIR-V
	if (Options & EOptionSpv) {
		if (CompileFailed || LinkFailed)
			printf("SPIR-V is not generated for failed compile or link\n");
		else {
			for (int stage = 0; stage < EShLangCount; ++stage) {
				if (program.getIntermediate((EShLanguage)stage)) {
					std::vector<unsigned int> spirv;
					std::string warningsErrors;
					spv::SpvBuildLogger logger;
					glslang::GlslangToSpv(*program.getIntermediate((EShLanguage)stage), spirv, &logger);
					preprocessSpirv(spirv);
					krafix::SpirVTranslator* translator = NULL;
					std::map<std::string, int> attributes;
					translator = new krafix::SpirVTranslator(spirv, shLanguageToShaderStage((EShLanguage)stage));
					translator->outputCode(target, sourcefilename, filename, output, attributes);
					if (output != nullptr) {
						*length = dynamic_cast<krafix::SpirVTranslator*>(translator)->outputLength;
					}
					delete translator;
				}
			}
		}
	}

	delete& program;
	while (shaders.size() > 0) {
		delete shaders.back();
		shaders.pop_back();
	}
}

krafix::TargetSystem getSystem(const char* system) {
	if (strcmp(system, "linux") == 0) return krafix::Linux;
	if (strcmp(system, "android") == 0) return krafix::Android;
	return krafix::Unknown;
}

void CompileAndLinkShaderFiles(krafix::Target target, const char* sourcefilename, const char* filename, const char* tempdir, const char* source, char* output, int* length, glslang::TShader::Includer& includer, const char* defines, bool relax)
{
	std::vector<ShaderCompUnit> compUnits;

	char* sources[] = { (char*)source, nullptr, nullptr, nullptr, nullptr };

	glslang::TWorkItem* workItem;
	while (Worklist.remove(workItem)) {
		ShaderCompUnit compUnit(
			FindLanguage(workItem->name),
			workItem->name,
			source != nullptr ? sources : ReadFileData(workItem->name.c_str())
		);

		if (!compUnit.text) {
			usage();
			return;
		}

		compUnits.push_back(compUnit);
	}

	for (int i = 0; i < ((Options & EOptionMemoryLeakMode) ? 100 : 1); ++i) {
		for (int j = 0; j < ((Options & EOptionMemoryLeakMode) ? 100 : 1); ++j)
			CompileAndLinkShaderUnits(compUnits, target, sourcefilename, filename, tempdir, output, length, includer, defines, relax);

		if (Options & EOptionMemoryLeakMode)
			glslang::OS_DumpMemoryCounters();
	}

	if (source == nullptr) {
		for (auto it = compUnits.begin(); it != compUnits.end(); ++it)
			FreeFileData(it->text);
	}
}

int compile(const char* targetlang, const char* from, std::string to, const char* tempdir, const char* source, char* output, int* length, const char* system,
	glslang::TShader::Includer& includer, std::string defines, int version, bool relax) {
	CompileFailed = false;

	Options |= EOptionSpv;
	Options |= EOptionLinkProgram;

	NumWorkItems = 1;
	Work = new glslang::TWorkItem * [NumWorkItems];
	Work[0] = 0;

	if (from) {
		std::string name(from);
		if (!SetConfigFile(name)) {
			Work[0] = new glslang::TWorkItem(name);
			Worklist.add(Work[0]);
		}
	}
	else {
		std::string name = std::string("nothing.") + to;
		Work[0] = new glslang::TWorkItem(name);
		Worklist.add(Work[0]);
	}

	glslang::InitializeProcess();

	krafix::Target target;
	target.system = getSystem(system);
	target.version = version > 0 ? version : 1;
	defines += "#define SPIRV " + std::to_string(target.version) + "\n";
	CompileAndLinkShaderFiles(target, from, to.c_str(), tempdir, source, output, length, includer, defines.c_str(), relax);
	if (!CompileFailed && !quiet) {
		std::cerr << "#file:" << to << std::endl;
	}

	glslang::FinalizeProcess();

	if (CompileFailed || LinkFailed) return 1;
	else return 0;
}

int compileOptionallyRelaxed(const char* targetlang, const char* from, std::string to, std::string ext, const char* tempdir, const char* source, char* output, int* length, const char* system,
	glslang::TShader::Includer& includer, std::string defines, int version, bool relax) {
	int regularErrors = 0, relaxErrors = 0, es3Errors = 0;

	regularErrors = compile(targetlang, from, to + ext, tempdir, source, output, length, system, includer, defines, version, false);
	if (relax) {
		relaxErrors = compile(targetlang, from, to + "-relaxed" + ext, tempdir, source, output, length, system, includer, defines, version, true);
		return std::min(regularErrors, relaxErrors);
	}
	else {
		return regularErrors;
	}
}

int compileOptionallyInstanced(const char* targetlang, const char* from, std::string to, std::string ext, const char* tempdir, const char* source, char* output, int* length, const char* system,
	glslang::TShader::Includer& includer, std::string defines, int version, bool instanced, bool relax) {
	int errors = 0;
	if (instanced) {
		errors += compileOptionallyRelaxed(targetlang, from, to + "-noinst", ext, tempdir, source, output, length, system, includer, defines, version, relax);
		errors += compileOptionallyRelaxed(targetlang, from, to + "-inst", ext, tempdir, source, output, length, system, includer, defines + "#define INSTANCED_RENDERING\n", version, relax);
	}
	else {
		errors += compileOptionallyRelaxed(targetlang, from, to, ext, tempdir, source, output, length, system, includer, defines, version, relax);
	}
	return errors;
}

int compileWithTextureUnits(const char* targetlang, const char* from, std::string to, std::string ext, const char* tempdir, const char* source, char* output, int* length, const char* system,
	glslang::TShader::Includer& includer, std::string defines, int version, const std::vector<int>& textureUnitCounts, bool usesTextureUnitsCount, bool instanced, bool relax) {
	int errors = 0;
	if (usesTextureUnitsCount && textureUnitCounts.size() > 0) {
		for (size_t i = 0; i < textureUnitCounts.size(); ++i) {
			int texcount = textureUnitCounts[i];
			std::stringstream toto;
			toto << to << "-tex" << texcount << ext;
			std::stringstream definesplustex;
			definesplustex << defines << "#define MAX_TEXTURE_UNITS=" << texcount << "\n";
			errors += compileOptionallyInstanced(targetlang, from, toto.str(), ext, tempdir, source, output, length, system, includer, definesplustex.str(), version, instanced, relax);
		}
	}
	else {
		errors += compileOptionallyInstanced(targetlang, from, to, ext, tempdir, source, output, length, system, includer, defines, version, instanced, relax);
	}
	return errors;
}

extern "C" int krafix_compile(const char *source, char *output, int *length, const char *targetlang, const char *system, const char *shadertype, int version) {
	CompileFailed = false;
	LinkFailed = false;

	std::string defines;
	std::vector<int> textureUnitCounts;
	bool instancedoptional = false;
	bool relax = false;
	quiet = true;

	ProcessConfigFile();

	NullIncluder includer;

	bool usesTextureUnitsCount = false;
	bool usesInstancedoptional = false;

	char from[256];
	strcpy(from, ".");
	strcat(from, shadertype);
	strcat(from, ".glsl");

	return compileWithTextureUnits(targetlang, from, "", shadertype, nullptr, source, output, length, system, includer, defines, version, textureUnitCounts, usesTextureUnitsCount, instancedoptional && usesInstancedoptional, relax);
}

EShLanguage FindLanguage(const std::string& name, bool parseSuffix) {
	size_t ext = 0;
	std::string suffix;

	if (shaderStageName)
		suffix = shaderStageName;
	else {
		if (parseSuffix) {
			ext = name.rfind('.');
			if (ext == std::string::npos) {
				usage();
				return EShLangVertex;
			}
			++ext;
		}
		suffix = name.substr(ext, std::string::npos);
	}

	if (suffix == "glsl") {
		size_t ext2 = name.substr(0, ext - 1).rfind('.');
		suffix = name.substr(ext2 + 1, ext - ext2 - 2);
	}

	if (suffix == "vert")
		return EShLangVertex;
	else if (suffix == "geom")
		return EShLangGeometry;
	else if (suffix == "frag")
		return EShLangFragment;
	else if (suffix == "comp")
		return EShLangCompute;

	usage();
	return EShLangVertex;
}

void CompileFile(const char* fileName, ShHandle compiler) {
	int ret = 0;
	char** shaderStrings = ReadFileData(fileName);
	if (!shaderStrings) {
		usage();
	}

	int* lengths = new int[NumShaderStrings];

	// move to length-based strings, rather than null-terminated strings
	for (int s = 0; s < NumShaderStrings; ++s)
		lengths[s] = (int)strlen(shaderStrings[s]);

	if (!shaderStrings) {
		CompileFailed = true;
		return;
	}

	EShMessages messages = EShMsgDefault;
	SetMessageOptions(messages);

	for (int i = 0; i < ((Options & EOptionMemoryLeakMode) ? 100 : 1); ++i) {
		for (int j = 0; j < ((Options & EOptionMemoryLeakMode) ? 100 : 1); ++j) {
			ret = ShCompile(compiler, shaderStrings, NumShaderStrings, nullptr, EShOptNone, &Resources, Options, (Options & EOptionDefaultDesktop) ? 110 : 100, false, messages);
		}

		if (Options & EOptionMemoryLeakMode)
			glslang::OS_DumpMemoryCounters();
	}

	delete[] lengths;
	FreeFileData(shaderStrings);

	if (ret == 0)
		CompileFailed = true;
}

void usage() {
	printf("Usage: krafix profile in out tempdir system\n");
	exit(EFailUsage);
}

char** ReadFileData(const char* fileName) {
	FILE *in = fopen(fileName, "r");

	int count = 0;
	const int maxSourceStrings = 5;  // for testing splitting shader/tokens across multiple strings
	char** return_data = (char**)malloc(sizeof(char*) * (maxSourceStrings + 1)); // freed in FreeFileData()

	if (in == nullptr)
		Error("unable to open input file");

	while (fgetc(in) != EOF)
		count++;

	fseek(in, 0, SEEK_SET);

	char* fdata = (char*)malloc(count + 2); // freed before return of this function
	if (!fdata)
		Error("can't allocate memory");

	if ((int)fread(fdata, 1, count, in) != count) {
		free(fdata);
		Error("can't read input file");
	}

	fdata[count] = '\0';
	fclose(in);

	if (count == 0) {
		// recover from empty file
		return_data[0] = (char*)malloc(count + 2);  // freed in FreeFileData()
		return_data[0][0] = '\0';
		NumShaderStrings = 0;
		free(fdata);

		return return_data;
	}
	else
		NumShaderStrings = 1;  // Set to larger than 1 for testing multiple strings

	// compute how to split up the file into multiple strings, for testing multiple strings
	int len = (int)(ceil)((float)count / (float)NumShaderStrings);
	int ptr_len = 0;
	int i = 0;
	while (count > 0) {
		return_data[i] = (char*)malloc(len + 2);  // freed in FreeFileData()
		memcpy(return_data[i], fdata + ptr_len, len);
		return_data[i][len] = '\0';
		count -= len;
		ptr_len += len;
		if (count < len) {
			if (count == 0) {
				NumShaderStrings = i + 1;
				break;
			}
			len = count;
		}
		++i;
	}

	free(fdata);

	return return_data;
}

void FreeFileData(char** data) {
	for (int i = 0; i < NumShaderStrings; i++)
		free(data[i]);

	free(data);
}
