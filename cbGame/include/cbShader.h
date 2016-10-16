#pragma once
#include "cbInclude.h"
#include "cbGame.h"
#include "cbBasic.h"
#include "cbMemory.h"
#include <GL/glew.h>

#define MAX_UNIFORMS 16
#define MAX_UNIFORM_NAME_LENGTH 32

enum cbShaderType
{
	cb_FRAGMENT_SHADER	= 0x8B30,
	cb_VERTEX_SHADER	= 0x8B31
};

struct cbShader
{
	cbShaderType ShaderType;
	char Filename[128];
	cbFiletime LastWriteTime;
};

struct cbShaderProgram
{
	cbShader *VertexShader;
	cbShader *FragmentShader;

	bool IsValid;
	uint32 ShaderId;
	uint32 CurrentUniformCount;
	char UniformKeys[MAX_UNIFORMS][MAX_UNIFORM_NAME_LENGTH];
	int UniformLocations[MAX_UNIFORMS];
};

inline bool cbCheckAndLogShaderErrors(uint32 handle)
{
	GLint success = 0;
	int infoLogLength;
	char text[1024];

	glGetShaderiv(handle, GL_COMPILE_STATUS, &success);
	if (success == GL_FALSE)
	{
		glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			glGetShaderInfoLog(handle, infoLogLength, NULL, &text[0]);
			//ToDo: Log to console
		}
		return false;
	}

	return true;
}

inline bool cbCheckAndLogProgramErrors(uint32 handle)
{
	GLint success = 0;
	int infoLogLength;
	char text[1024];

	glGetProgramiv(handle, GL_LINK_STATUS, &success);
	if (success == GL_FALSE)
	{
		glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			glGetProgramInfoLog(handle, infoLogLength, NULL, &text[0]);
			//ToDo: Log to console
		}
		return false;
	}

	return true;
}

inline cbShader* cbCreateShader_(cbArena *memory, cbShaderType type, char* path)
{
	cbShader *result = PushStruct(memory, cbShader);
	result->ShaderType = type;
	cbStrCopy(result->Filename, path);

	result->LastWriteTime = Platform.GetLastFileTime(path);

	return result;
}
#define cbCreateProgram(vertSource, fragSource)	cbCreateProgram_(&TransStorage->ShaderArena,							\
												cbCreateShader_(&TransStorage->ShaderArena, cb_VERTEX_SHADER, vertSource),	\
												cbCreateShader_(&TransStorage->ShaderArena, cb_FRAGMENT_SHADER,fragSource))


inline cbShaderProgram* cbCreateProgram_(cbShaderProgram *result, cbShader *vShader, cbShader *fShader)
{
	result->VertexShader = vShader;
	result->FragmentShader = fShader;

	result->ShaderId = glCreateProgram();
	uint32 vertHandle = glCreateShader(vShader->ShaderType);
	uint32 fragHandle = glCreateShader(fShader->ShaderType);

	long size;
	char* vertexShaderCode = Platform.cbReadTextFile(result->VertexShader->Filename, size);
	char* fragmentShaderCode = Platform.cbReadTextFile(result->FragmentShader->Filename, size);

	glShaderSource(vertHandle, 1, &vertexShaderCode, 0);
	glCompileShader(vertHandle);
	bool isValid = cbCheckAndLogShaderErrors(vertHandle);

	glShaderSource(fragHandle, 1, &fragmentShaderCode, 0);
	glCompileShader(fragHandle);
	isValid = isValid && cbCheckAndLogShaderErrors(fragHandle);

	glAttachShader(result->ShaderId, vertHandle);
	glAttachShader(result->ShaderId, fragHandle);
	glLinkProgram(result->ShaderId);
	isValid = isValid && cbCheckAndLogProgramErrors(result->ShaderId);

	glDetachShader(result->ShaderId, vertHandle);
	glDetachShader(result->ShaderId, fragHandle);

	glDeleteShader(vertHandle);
	glDeleteShader(fragHandle);

	Platform.cbFreeFile(vertexShaderCode);
	Platform.cbFreeFile(fragmentShaderCode);

	result->IsValid = isValid;
	if (!isValid)
		glDeleteProgram(result->ShaderId);

	return result;
}

inline cbShaderProgram* cbCreateProgram_(cbArena *memory, cbShader *vShader, cbShader *fShader)
{
	cbShaderProgram* result = PushStruct(memory, cbShaderProgram);
	
	return cbCreateProgram_(result, vShader, fShader);
}

inline void cbInvalidateProgram(cbShaderProgram *program)
{
	if(program->IsValid)
		glDeleteProgram(program->ShaderId);
	program->IsValid = false;
}

inline bool cbProgramNeedsHotReload(cbShaderProgram *program)
{
	cbFiletime newVertexFT = Platform.GetLastFileTime(program->VertexShader->Filename);
	if (Platform.CompareFileTime(newVertexFT, program->VertexShader->LastWriteTime) != 0)
	{
		program->VertexShader->LastWriteTime = newVertexFT;
		return true;
	}

	cbFiletime newFragFT = Platform.GetLastFileTime(program->FragmentShader->Filename);
	if (Platform.CompareFileTime(newFragFT, program->FragmentShader->LastWriteTime) != 0)
	{
		program->FragmentShader->LastWriteTime = newFragFT;
		return true;
	}

	return false;
}

inline void cbReloadShader(cbShaderProgram* shader)
{
	cbInvalidateProgram(shader);
	cbCreateProgram_(shader, shader->VertexShader, shader->FragmentShader);
}

inline bool cbUseProgram(cbShaderProgram *program)
{
	if (cbProgramNeedsHotReload(program))
		cbReloadShader(program);
	if (!program->IsValid)
		return false;
	glUseProgram(program->ShaderId);
	return true;
}

inline int cbGetUniformLocation(cbShaderProgram *program, char *name)
{
	if (!program->IsValid)
		return -1;

	Assert(program->CurrentUniformCount <= MAX_UNIFORMS);
	// Check if already queried
	for (uint32 i = 0; i < program->CurrentUniformCount; ++i)
	{
		char *key = &program->UniformKeys[i][0];
		if(cbStrCmp(key, name) == 0)
		{
			return program->UniformLocations[i];
		}
	}

	// Add to List
	Assert(program->CurrentUniformCount < MAX_UNIFORMS);
	char *dest = &program->UniformKeys[program->CurrentUniformCount][0];
	cbStrCopy(dest, name);
	program->UniformLocations[program->CurrentUniformCount] = glGetUniformLocation(program->ShaderId, name);

	return program->UniformLocations[program->CurrentUniformCount++];
}