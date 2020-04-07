#pragma once

#ifndef _ASSIMP_MESH_H_
#define _ASSIMP_MESH_H_

#include "assimp_shader.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <vector>


#define NUM_OF_BONES 16
#define INVALID_UNIFORM_LOCATION 0xffffffff

class Shader;
class Camera;

struct Vertex {
	glm::vec3 Position;
	glm::vec3 Normal;
	glm::vec2 TexCoords;
};


// ---------------------------------------------------
// ---------------------------------------------------
// ---------------------------------------------------
struct BoneData
{
	uint bone_id[NUM_OF_BONES];
	float bone_weights[NUM_OF_BONES];

	BoneData() {
		Reset();
	};

	void Reset() {
		memset(bone_id, 0, sizeof(bone_id));
		memset(bone_weights, 0, sizeof(bone_weights));
	}

	void AddBoneData(uint BoneID, float weight) {
		for (uint i = 0; i < NUM_OF_BONES; i++)
		{
			if (bone_weights[i] == 0.0)
			{
				bone_id[i] = BoneID;
				bone_weights[i] = weight;
				return;
			}
		}

		// should never get here
		assert(0);
	}
};

struct Texture
{
	unsigned int id;
	string type;
	aiString path;
};

class AssimpMesh
{
public:
	vector<Vertex> vertices;
	vector<unsigned int> indices;
	vector<Texture> textures;
	vector<BoneData> bones;

	vec3 draw_pos;

	// trans matrix
	mat4 ai_projection;
	mat4 ai_view;
	mat4 ai_model;


	AssimpMesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
		vector<BoneData> bones);

	void setVertices(vector<Vertex> vertices);
	void setTransMat4(mat4 projection, mat4 view, mat4 model);
	void Draw(Shader* shader, Camera* camera);
	unsigned int getVAO();

private:
	unsigned int VAO, VBO_Vertex, VBO_Bone, EBO;
	void setupMesh();
};

#endif

