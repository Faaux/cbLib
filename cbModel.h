#pragma once
#include "cbInclude.h"
#include "cbGame.h"
#include "cbBasic.h"
#include "cbMemory.h"
#include <GL/glew.h>
#include <assimp/cimport.h>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

struct cbMesh
{
	bool HasUV;
	uint32 IndiceCount;
	GLuint VAO;
	GLuint VBO;
	GLuint EAB;
};

struct cbModel
{
	bool IsValid;
	uint32 meshCount;
	cbMesh *meshes;
};

#define MAX_MODELS 64
struct cbModelTable
{
	uint32 UsedModels;
	char *ModelNames[MAX_MODELS];
	cbModel *Models[MAX_MODELS];
};

inline cbMesh *cbUploadMesh(cbMesh *mesh, void *data, mem_size size, uint32 *indices)
{
	
	glGenVertexArrays(1, &mesh->VAO);
	glBindVertexArray(mesh->VAO);

	glGenBuffers(1, &mesh->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(12));
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), BUFFER_OFFSET(24));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenBuffers(1, &mesh->EAB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EAB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh->IndiceCount * sizeof(uint32), indices, GL_STATIC_DRAW);

	return mesh;
}

inline void cbDeleteMesh(cbMesh *mesh)
{
	glDeleteVertexArrays(1, &mesh->VAO);
	glDeleteBuffers(1, &mesh->VBO);
}

#define StartRecursiveMeshLoad(scene,rootNode,model) {uint32 recurseiveMeshIndex = 0; RecursiveMeshLoad_(scene,rootNode,model,recurseiveMeshIndex);}
inline void RecursiveMeshLoad_(const aiScene *scene, aiNode *rootNode, cbModel *model, uint32 &meshIndex)
{
	TIMED_FUNCTION();
	for (uint32 n = 0; n < rootNode->mNumMeshes; ++n)
	{
		aiMesh* aiMesh = scene->mMeshes[rootNode->mMeshes[n]];

		cbMesh *mesh = &model->meshes[meshIndex++];		

		// Get Texture Data
		aiMaterial *mat = scene->mMaterials[aiMesh->mMaterialIndex];
		aiString str;
		mat->GetTexture(aiTextureType_DIFFUSE, 0, &str);
		mat->GetTexture(aiTextureType_AMBIENT, 0, &str);
		mat->GetTexture(aiTextureType_SPECULAR, 0, &str);

		aiColor3D color(0.f, 0.f, 0.f);
		mat->Get(AI_MATKEY_COLOR_DIFFUSE, color);
		mat->Get(AI_MATKEY_COLOR_SPECULAR, color);
		mat->Get(AI_MATKEY_COLOR_AMBIENT, color);

		// Get Vertex Data
		// 3 Pos, 3 Normal, 2 UV
		uint32 numVerts = aiMesh->mNumVertices;
		mem_size meshSize = sizeof(float) * (3 + 3 + 2) * numVerts;
		float *meshDataOrigin = (float *)malloc(meshSize);
		float *meshData = meshDataOrigin;

		for(uint32 i = 0; i < aiMesh->mNumVertices; ++i)
		{
			const aiVector3D* pPos = &aiMesh->mVertices[i];
			const aiVector3D* pNormal = &aiMesh->mNormals[i];
			const aiVector3D* pUv = &aiMesh->mTextureCoords[0][i];

			*meshData++ = pPos->x;
			*meshData++ = pPos->y;
			*meshData++ = pPos->z;


			if (pNormal)
			{
				*meshData++ = pNormal->x;
				*meshData++ = pNormal->y;
				*meshData++ = pNormal->z;
			}
			else
			{
				Assert(false);
				*meshData++ = 1.0;
				*meshData++ = 1.0;
				*meshData++ = 1.0;
			}

			if (aiMesh->mTextureCoords[0])
			{
				*meshData++ = pUv->x;
				*meshData++ = pUv->y;
			}
			else
			{
				mesh->HasUV = false;
				*meshData++ = 1.0;
				*meshData++ = 1.0;
			}
		}

		// Get Indices Data
		uint32 numIndices = aiMesh->mNumFaces * 3;
		uint32 *indexDataOrigin = (uint32 *)malloc(numIndices * sizeof(uint32));
		uint32 *indexData = indexDataOrigin;
		for (uint32 j = 0; j < aiMesh->mNumFaces; ++j)
		{
			const aiFace* face = &aiMesh->mFaces[j];
			
			Assert(face->mNumIndices == 3);
			*indexData++ = face->mIndices[0];
			*indexData++ = face->mIndices[1];
			*indexData++ = face->mIndices[2];
		}
		
		// Set state for mesh
		mesh->IndiceCount = numIndices;
		
		cbUploadMesh(mesh, meshDataOrigin, meshSize, indexDataOrigin);
		
		free(meshDataOrigin);
		free(indexDataOrigin);
	}

	for (uint32 n = 0; n < rootNode->mNumChildren; ++n)
	{
		RecursiveMeshLoad_(scene, rootNode->mChildren[n], model, meshIndex);
	}
}

#define cbLoadModel(file) cbLoadModel_(&TransStorage->ModelArena, file)
inline cbModel *cbLoadModel_(cbArena *memory, char* fileName)
{
	TIMED_FUNCTION();
	cbModelTable *table = (cbModelTable*)memory->Base;
	for (uint32 i = 0; i < table->UsedModels; ++i)
	{
		if(cbStrCmp(fileName,table->ModelNames[i])==0)
		{
			return table->Models[i];
		}
	}

	Assert(table->UsedModels < MAX_MODELS);
	cbModel *model = PushStruct(memory, cbModel);
	table->ModelNames[table->UsedModels] = fileName;
	table->Models[table->UsedModels] = model;
	table->UsedModels++;

	aiLogStream stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, 0);
	aiAttachLogStream(&stream);

	const aiScene *scene = aiImportFile(fileName, aiProcessPreset_TargetRealtime_MaxQuality);
	Assert(scene);

	model->meshCount = scene->mNumMeshes;
	model->meshes = PushArray(memory, model->meshCount, cbMesh);

	StartRecursiveMeshLoad(scene, scene->mRootNode, model);

	aiReleaseImport(scene);
	aiDetachAllLogStreams();

	model->IsValid = true;
	return model;
}

inline void cbDeleteModel(cbModel *model)
{
	model->IsValid = false;
	for (uint32 i = 0; i < model->meshCount; i++)
	{
		cbDeleteMesh(&model->meshes[i]);
	}
}

inline void cbRenderModel(cbModel *model)
{
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	Assert(model->IsValid);
	for (uint32 i = 0; i < model->meshCount; i++)
	{
		cbMesh* current = &model->meshes[i];

		glBindVertexArray(current->VAO);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, current->EAB);

		// Draw the triangles !
		glDrawElements(
			GL_TRIANGLES,      // mode
			current->IndiceCount,    // count
			GL_UNSIGNED_INT,   // type
			0           // element array buffer offset
		);
		glBindVertexArray(0);
	}

}

inline void cbInitModelTable(cbArena *memory)
{
	PushStruct(memory, cbModelTable);
}
