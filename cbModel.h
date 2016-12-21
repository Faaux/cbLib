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
	glEnableVertexAttribArray(2);
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
				*meshData++ = -pUv->y;
			}
			else
			{
				Assert(false);
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

inline void cbRenderModel(cbShaderProgram* program, cbModel *model)
{
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);
	Assert(model->IsValid);

	// Just for now, debug, load textures here and check rendering
	

	static GLuint albedoId = 12345678;
	if(albedoId == 12345678)
	{
		int width, height;
		uint8 *albedo = Platform.cbLoadImage("res\\models\\sub\\model\\textures\\Sphere_albedo.jpg", width, height);

		glGenTextures(1, &albedoId);
		glBindTexture(GL_TEXTURE_2D, albedoId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, albedo);
		// can free temp_bitmap at this point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		Platform.cbFreeImage(albedo);

	}

	static GLuint metallicId = 12345678;
	if (metallicId == 12345678)
	{
		int width, height;
		uint8 *metallic = Platform.cbLoadImage("res\\models\\sub\\model\\textures\\Sphere_metallic.jpg", width, height);

		glGenTextures(1, &metallicId);
		glBindTexture(GL_TEXTURE_2D, metallicId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, metallic);
		// can free temp_bitmap at this point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		Platform.cbFreeImage(metallic);

	}

	static GLuint roughnessId = 12345678;
	if (roughnessId == 12345678)
	{
		int width, height;
		uint8 *roughness = Platform.cbLoadImage("res\\models\\sub\\model\\textures\\Sphere_roughness.jpg", width, height);

		glGenTextures(1, &roughnessId);
		glBindTexture(GL_TEXTURE_2D, roughnessId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, roughness);
		// can free temp_bitmap at this point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		Platform.cbFreeImage(roughness);
	}

	static GLuint iblbrdfId = 12345678;
	if (iblbrdfId == 12345678)
	{
		int width, height;
		uint8 *normal = Platform.cbLoadImage("res\\models\\sub\\model\\textures\\Sphere_normal.jpg", width, height);

		glGenTextures(1, &iblbrdfId);
		glBindTexture(GL_TEXTURE_2D, iblbrdfId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, normal);
		// can free temp_bitmap at this point
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		Platform.cbFreeImage(normal);
	}

	GLuint texLoc = cbGetUniformLocation(program, "albedoTex");
	glUniform1i(texLoc, 0);

	texLoc = cbGetUniformLocation(program, "metallicTex");
	glUniform1i(texLoc, 1);

	texLoc = cbGetUniformLocation(program, "roughnessTex");
	glUniform1i(texLoc, 2);

	texLoc = cbGetUniformLocation(program, "iblbrdf");
	glUniform1i(texLoc, 3);


	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, albedoId);

	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, metallicId);

	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, roughnessId);

	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, iblbrdfId);

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
