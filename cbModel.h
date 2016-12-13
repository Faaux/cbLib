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

inline cbMesh *cbUploadMesh(cbMesh *mesh, void *data, mem_size size, uint32 numIndices, uint32 *indices)
{
	mesh->IndiceCount = numIndices;
	glGenVertexArrays(1, &mesh->VAO);
	glBindVertexArray(mesh->VAO);

	glGenBuffers(1, &mesh->VBO);
	glBindBuffer(GL_ARRAY_BUFFER, mesh->VBO);

	glBufferData(GL_ARRAY_BUFFER, size, data, GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(0));
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), BUFFER_OFFSET(12));

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);
	glBindVertexArray(0);

	glGenBuffers(1, &mesh->EAB);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh->EAB);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, numIndices * sizeof(uint32), indices, GL_STATIC_DRAW);

	return mesh;
}

inline void cbDeleteMesh(cbMesh *mesh)
{
	glDeleteVertexArrays(1, &mesh->VAO);
	glDeleteBuffers(1, &mesh->VBO);
}

inline void RecursiveMeshLoad(const aiScene *scene, aiNode *rootNode, cbModel *model, uint32 meshIndex)
{
	for (uint32 n = 0; n < rootNode->mNumMeshes; ++n)
	{
		aiMesh* mesh = scene->mMeshes[rootNode->mMeshes[n]];

		// 3 Pos, 3 Normal
		uint32 numVerts = mesh->mNumVertices;
		mem_size meshSize = sizeof(float) * (3 + 3) * numVerts;
		float *meshDataOrigin = (float *)malloc(meshSize);
		float *meshData = meshDataOrigin;

		uint32 counter = 0;

		for(uint32 i = 0; i < mesh->mNumVertices; ++i)
		{
			const aiVector3D* pPos = &mesh->mVertices[i];
			const aiVector3D* pNormal = &mesh->mNormals[i];

			counter += 3;
			*meshData++ = pPos->x;
			*meshData++ = pPos->y;
			*meshData++ = pPos->z;


			if (pNormal)
			{
				counter += 3;
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
		}
		Assert(counter == numVerts * 6);


		uint32 numIndices = mesh->mNumFaces * 3;
		uint32 *indexDataOrigin = (uint32 *)malloc(numIndices * sizeof(uint32));
		uint32 *indexData = indexDataOrigin;
		counter = 0;
		for (uint32 j = 0; j < mesh->mNumFaces; ++j)
		{
			const aiFace* face = &mesh->mFaces[j];
			
			Assert(face->mNumIndices == 3);
			*indexData++ = face->mIndices[0];
			*indexData++ = face->mIndices[1];
			*indexData++ = face->mIndices[2];
			counter += 3;
		}

		Assert(counter == numIndices);

		// Upload mesh to Graphics Card
		// Save in some struct to render later on with needed info
		cbUploadMesh(&model->meshes[meshIndex++], meshDataOrigin, meshSize, numIndices, indexDataOrigin);
		
		free(meshDataOrigin);
		free(indexDataOrigin);
	}

	for (uint32 n = 0; n < rootNode->mNumChildren; ++n)
	{
		RecursiveMeshLoad(scene, rootNode->mChildren[n], model, meshIndex);
	}
}

#define cbLoadModel(file) cbLoadModel_(&TransStorage->ModelArena, file)
inline cbModel *cbLoadModel_(cbArena *memory, char* fileName)
{
	cbModel *model = PushStruct(memory, cbModel);

	aiLogStream stream = aiGetPredefinedLogStream(aiDefaultLogStream_STDOUT, 0);
	aiAttachLogStream(&stream);

	const aiScene *scene = aiImportFile("res\\wt_teapot.obj", aiProcessPreset_TargetRealtime_MaxQuality);
	Assert(scene);

	model->meshCount = scene->mNumMeshes;
	model->meshes = PushArray(memory, model->meshCount, cbMesh);

	RecursiveMeshLoad(scene, scene->mRootNode, model, 0);

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
	glEnable(GL_DEPTH_TEST);
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
