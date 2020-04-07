#pragma once
#ifndef _ACT_MESH_H_
#define _ACT_MESH_H_

#include <stb-master/stb_image.h>
#include <map>

#include "assimp_mesh.h"


using namespace Assimp;

uint TextureFromFile(const char* path, string directory);
void SetZero(aiMatrix4x4& mat);
void SetIdentity(aiMatrix4x4& mat);
void SetScaling(aiMatrix4x4& mat, aiVector3D Scaling);
void SetRotation(aiMatrix4x4& mat, aiQuaternion Rotation);
void SetTranslation(aiMatrix4x4& mat, aiVector3D Translation);
const GLfloat* ConvertToOpenGL(aiMatrix4x4& mat);
mat4 ConvertToGLM(aiMatrix4x4& mat);

struct BoneInfo {
	aiMatrix4x4 BoneOffset;
	aiMatrix4x4 FinalTransform;

	BoneInfo() {
		SetZero(BoneOffset);
		SetZero(FinalTransform);
	}
};


class ActMesh
{
public:
	ActMesh(const GLchar* path);

	void BoneTransform(float TimeInSeconds, vector<aiMatrix4x4>& Transforms);
	void setVertices(vector<Vertex> vertices);
	void DrawAnim(Shader* shader, float currentTime, Camera* camera);
	void UpdateCurrentPosition(AssimpMesh mesh, vector<aiMatrix4x4> transforms);

private:
	vector<AssimpMesh> meshes;
	vector<Vertex> current_vertices; 
	vector<Texture> textures_loaded;
	string directory;
	const aiScene* m_scene;
	Importer importer;              


	// ----------------------------------
	// Bones
	map<string, uint>bone_map;
	vector<BoneInfo> bone_infos;
	aiMatrix4x4 GlobalInverseTransfrom;


	void loadModel(string path);
	void processNode(aiNode* node, const aiScene* scene);
	AssimpMesh processMesh(aiMesh* mesh, const aiScene* scene);
	vector<Texture> loadTextures(aiMaterial* mat, aiTextureType type, string typename);


	// Bones Animation
	uint getBoneNumber();
	void ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform);
	const aiNodeAnim* FindNodeAnim(const aiAnimation* pAnimation, const string NodeName);
	void CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	void CalcInterpolatedTranslation(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim);
	uint FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim);
	uint FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim);
	uint FindTranslation(float AnimationTime, const aiNodeAnim* pNodeAnim);


};

#endif
