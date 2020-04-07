#include "act_mesh.h"

ActMesh::ActMesh(const GLchar* path)
{
	this->loadModel(path);
	this->current_vertices = this->meshes[0].vertices;
}


void ActMesh::BoneTransform(float TimeInSeconds, vector<aiMatrix4x4>& Transforms)
{
	aiMatrix4x4 Identity;
	SetIdentity(Identity);

	float TicksPerSecond = (float)(m_scene->mAnimations[0]->mTicksPerSecond != 0 ? m_scene->mAnimations[0]->mTicksPerSecond : 25.0);
	float TimeInTicks = TimeInSeconds * TicksPerSecond;

	// current animation time
	float AnimationTime = fmod(TimeInTicks, (float)m_scene->mAnimations[0]->mDuration);

	ReadNodeHeirarchy(AnimationTime, m_scene->mRootNode, Identity);

	uint numOfBones = getBoneNumber();
	Transforms.resize(numOfBones);

	for (int i = 0; i < numOfBones; i++)
	{
		Transforms[i] = bone_infos[i].FinalTransform;
	}
}

void ActMesh::setVertices(vector<Vertex> vertices)
{
	this->current_vertices = vertices;
}

void ActMesh::DrawAnim(Shader* shader, float currentTime, Camera* camera)
{
	// bones
	vector<aiMatrix4x4> transforms;
	BoneTransform(currentTime, transforms);
	for (int i = 0; i < transforms.size(); i++) {
		char UniformName[128];
		memset(UniformName, 0, sizeof(UniformName));
		snprintf(UniformName, sizeof(UniformName), "gBones[%d]", i);
		uint boneTransLocation = glGetUniformLocation(shader->ID, UniformName);

		const GLfloat* trans = ConvertToOpenGL(transforms[i]);
		glUniformMatrix4fv(boneTransLocation, 1, GL_FALSE, trans);
	}

	// meshes
	for (int i = 0; i < this->meshes.size(); i++)
	{
		glUniform1i(glGetUniformLocation(shader->ID, "isCloth"), false);
		this->meshes[i].Draw(shader, camera);

		// update current vertices
	    // UpdateCurrentPosition(this->meshes[i], transforms);
	}
}

void ActMesh::UpdateCurrentPosition(AssimpMesh mesh, vector<aiMatrix4x4> transforms)
{
	vector<BoneData> bones = mesh.bones;
	const int bone_num = 4;
	for (int i = 0; i < this->current_vertices.size(); i++) {
		mat4 gBones[bone_num];
		for (int j = 0; j < bone_num; j++)
		{
			gBones[j] = ConvertToGLM(transforms[bones[i].bone_id[j]]);
		}
		mat4 BoneTransform = gBones[0] * bones[i].bone_weights[0];
		BoneTransform += gBones[1] * bones[i].bone_weights[1];
		BoneTransform += gBones[2] * bones[i].bone_weights[2];
		BoneTransform += gBones[3] * bones[i].bone_weights[3];

		vec3 position = mesh.vertices[i].Position;
		vec4 tmp = mesh.ai_projection * mesh.ai_view * mesh.ai_model * BoneTransform 
			* vec4(position, 1.0f);
		position = vec3(tmp.x, tmp.y, tmp.z);
		this->current_vertices[i].Position = position;
	}
}


void ActMesh::loadModel(string path)
{
	m_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices
		| aiProcess_RemoveComponent);
	if (!m_scene || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}

	this->directory = path.substr(0, path.find_last_of('/'));
	this->GlobalInverseTransfrom = m_scene->mRootNode->mTransformation;
	GlobalInverseTransfrom.Inverse();


	this->processNode(m_scene->mRootNode, m_scene);
}


void ActMesh::processNode(aiNode* node, const aiScene* scene)
{

	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(processMesh(mesh, scene));
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene);
	}
}

AssimpMesh ActMesh::processMesh(aiMesh* mesh, const aiScene* scene)
{
	vector<Vertex>vertices;
	vector<uint>indices;
	vector<Texture>textures;
	vector<BoneData>bones;

	// vertices
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		Vertex vertex;

		vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertex.Position = position;

		vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		vertex.Normal = normal;

		if (mesh->mTextureCoords[0]) {
			vec2 texCoords;
			texCoords.x = mesh->mTextureCoords[0][i].x;
			texCoords.y = mesh->mTextureCoords[0][i].y;
			vertex.TexCoords = texCoords;
		}
		else
		{
			vertex.TexCoords = vec2(0.0, 0.0);
		}

		vertices.push_back(vertex);
	}

	// bones
	bones.resize(vertices.size());
	for (int i = 0; i < mesh->mNumBones; i++) {
		uint bone_index;
		string bone_name(mesh->mBones[i]->mName.data);
		if (bone_map.find(bone_name) == bone_map.end()) {
			// new bone
			bone_index = getBoneNumber();
			BoneInfo bi;
			bone_infos.push_back(bi);
			bone_infos[bone_index].BoneOffset = mesh->mBones[i]->mOffsetMatrix;
			bone_map[bone_name] = bone_index;
		}
		else
		{
			bone_index = bone_map[bone_name];
		}

		int num = mesh->mBones[i]->mNumWeights;
		for (int j = 0; j < mesh->mBones[i]->mNumWeights; j++)
		{
			uint vertex_id = mesh->mBones[i]->mWeights[j].mVertexId;
			float weight = mesh->mBones[i]->mWeights[j].mWeight;
			bones[vertex_id].AddBoneData(bone_index, weight);
		}
	}

	// vertices indices(faces)
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		for (int j = 0; j < face.mNumIndices; j++)
		{
			indices.push_back(face.mIndices[j]);
		}
	}

	// material
	if (mesh->mMaterialIndex >= 0)
	{
		aiMaterial* material = scene->mMaterials[mesh->mMaterialIndex];

		vector<Texture> diffuseMaps = loadTextures(material, aiTextureType_DIFFUSE, "texture_diffuse");
		textures.insert(textures.end(), diffuseMaps.begin(), diffuseMaps.end());

		vector<Texture> specularMaps = loadTextures(material, aiTextureType_SPECULAR, "texture_specular");
		textures.insert(textures.end(), specularMaps.begin(), specularMaps.end());
	}

	return AssimpMesh(vertices, indices, textures, bones);
}

vector<Texture> ActMesh::loadTextures(aiMaterial* mat, aiTextureType type, string typeName)
{
	vector<Texture> textures;

	for (int i = 0; i < mat->GetTextureCount(type); i++)
	{
		aiString path;
		mat->GetTexture(type, i, &path);

		bool loaded = false;
		for (int j = 0; j < textures_loaded.size(); j++)
		{
			if (textures_loaded[j].path == path)
			{
				textures.push_back(textures_loaded[j]);
				loaded = true;
				break;
			}
		}
		if (!loaded)
		{
			Texture texture;
			texture.id = TextureFromFile(path.C_Str(), this->directory);
			texture.type = typeName;
			texture.path = path;

			textures.push_back(texture);
			textures_loaded.push_back(texture);
		}
	}

	return textures;
}

uint ActMesh::getBoneNumber()
{
	return bone_map.size();
}

void ActMesh::ReadNodeHeirarchy(float AnimationTime, const aiNode* pNode, const aiMatrix4x4& ParentTransform)
{
	string NodeName(pNode->mName.data);

	const aiAnimation* pAnimation = m_scene->mAnimations[0];

	aiMatrix4x4 NodeTransfrom(pNode->mTransformation);

	const aiNodeAnim* pNodeAnim = FindNodeAnim(pAnimation, NodeName);

	if (pNodeAnim) {
		// Interpolate scaling
		aiVector3D Scaling;
		CalcInterpolatedScaling(Scaling, AnimationTime, pNodeAnim);
		aiMatrix4x4 ScalingMatrix;
		SetScaling(ScalingMatrix, Scaling);

		// Interpolate rotation
		aiQuaternion Rotation;
		CalcInterpolatedRotation(Rotation, AnimationTime, pNodeAnim);
		aiMatrix4x4 RotationMatrix;
		SetRotation(RotationMatrix, Rotation);

		// Interpolate translation
		aiVector3D Translation;
		CalcInterpolatedTranslation(Translation, AnimationTime, pNodeAnim);
		aiMatrix4x4 TransMatrix;
		SetTranslation(TransMatrix, Translation);

		NodeTransfrom = TransMatrix * RotationMatrix * ScalingMatrix;
	}

	aiMatrix4x4 GlobalTransform = ParentTransform * NodeTransfrom;

	if (bone_map.find(NodeName) != bone_map.end()) {
		uint BoneIndex = bone_map[NodeName];
		bone_infos[BoneIndex].FinalTransform = GlobalInverseTransfrom * GlobalTransform * bone_infos[BoneIndex].BoneOffset;
	}

	for (int i = 0; i < pNode->mNumChildren; i++)
	{
		ReadNodeHeirarchy(AnimationTime, pNode->mChildren[i], GlobalTransform);
	}
}

const aiNodeAnim* ActMesh::FindNodeAnim(const aiAnimation* pAnimation, const string NodeName)
{
	for (int i = 0; i < pAnimation->mNumChannels; i++)
	{
		const aiNodeAnim* pNodeAnim = pAnimation->mChannels[i];

		if (string(pNodeAnim->mNodeName.data) == NodeName) {
			return pNodeAnim;
		}
	}
	return NULL;
}

void ActMesh::CalcInterpolatedScaling(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumScalingKeys == 1) {
		Out = pNodeAnim->mScalingKeys[0].mValue;
		return;
	}

	uint ScalingIndex = FindScaling(AnimationTime, pNodeAnim);
	uint NextScalingIndex = ScalingIndex + 1;
	assert(NextScalingIndex < pNodeAnim->mNumScalingKeys);
	float deltaTime = (float)(pNodeAnim->mScalingKeys[NextScalingIndex].mTime -
		pNodeAnim->mScalingKeys[ScalingIndex].mTime);
	float factor = (AnimationTime - (float)pNodeAnim->mScalingKeys[ScalingIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);

	const aiVector3D& start = pNodeAnim->mScalingKeys[ScalingIndex].mValue;
	const aiVector3D& end = pNodeAnim->mScalingKeys[NextScalingIndex].mValue;
	aiVector3D deltaValue = end - start;
	Out = start + factor * deltaValue;
}

void ActMesh::CalcInterpolatedRotation(aiQuaternion& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumRotationKeys == 1) {
		Out = pNodeAnim->mRotationKeys[0].mValue;
		return;
	}

	uint RotationIndex = FindRotation(AnimationTime, pNodeAnim);
	uint NextRotationIndex = RotationIndex + 1;
	assert(NextRotationIndex < pNodeAnim->mNumRotationKeys);
	float deltaTime = (float)(pNodeAnim->mRotationKeys[NextRotationIndex].mTime -
		pNodeAnim->mRotationKeys[RotationIndex].mTime);
	float factor = (AnimationTime - (float)pNodeAnim->mRotationKeys[RotationIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);

	const aiQuaternion& start = pNodeAnim->mRotationKeys[RotationIndex].mValue;
	const aiQuaternion& end = pNodeAnim->mRotationKeys[NextRotationIndex].mValue;
	aiQuaternion::Interpolate(Out, start, end, factor);
	Out = Out.Normalize();
}

void ActMesh::CalcInterpolatedTranslation(aiVector3D& Out, float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	if (pNodeAnim->mNumPositionKeys == 1) {
		Out = pNodeAnim->mPositionKeys[0].mValue;
		return;
	}

	uint PositionIndex = FindTranslation(AnimationTime, pNodeAnim);
	uint NextPositionIndex = PositionIndex + 1;
	assert(NextPositionIndex < pNodeAnim->mNumPositionKeys);
	float deltaTime = (float)(pNodeAnim->mPositionKeys[NextPositionIndex].mTime -
		pNodeAnim->mPositionKeys[PositionIndex].mTime);
	float factor = (AnimationTime - (float)pNodeAnim->mPositionKeys[PositionIndex].mTime) / deltaTime;
	assert(factor >= 0.0f && factor <= 1.0f);

	const aiVector3D& start = pNodeAnim->mPositionKeys[PositionIndex].mValue;
	const aiVector3D& end = pNodeAnim->mPositionKeys[NextPositionIndex].mValue;
	aiVector3D deltaValue = end - start;
	Out = start + factor * deltaValue;
}

uint ActMesh::FindScaling(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumScalingKeys > 0);

	for (int i = 0; i < pNodeAnim->mNumScalingKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mScalingKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

uint ActMesh::FindRotation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumRotationKeys > 0);

	for (int i = 0; i < pNodeAnim->mNumRotationKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mRotationKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}

uint ActMesh::FindTranslation(float AnimationTime, const aiNodeAnim* pNodeAnim)
{
	assert(pNodeAnim->mNumPositionKeys > 0);

	for (int i = 0; i < pNodeAnim->mNumPositionKeys - 1; i++)
	{
		if (AnimationTime < (float)pNodeAnim->mPositionKeys[i + 1].mTime) {
			return i;
		}
	}

	assert(0);

	return 0;
}


uint TextureFromFile(const char* path, string directory)
{
	string filename = string(path);
	filename = directory + '/' + filename;

	uint textureID;
	glGenTextures(1, &textureID);
	int width, height, nrComponents;
	unsigned char* data = stbi_load(filename.c_str(), &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;


		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}

void SetZero(aiMatrix4x4& mat) {
	mat.a1 = 0; mat.a2 = 0; mat.a3 = 0; mat.a4 = 0;
	mat.b1 = 0; mat.b2 = 0; mat.b3 = 0; mat.b4 = 0;
	mat.c1 = 0; mat.c2 = 0; mat.c3 = 0; mat.c4 = 0;
	mat.d1 = 0; mat.d2 = 0; mat.d3 = 0; mat.d4 = 0;
}

void SetIdentity(aiMatrix4x4& mat) {
	mat.a1 = 1; mat.a2 = 0; mat.a3 = 0; mat.a4 = 0;
	mat.b1 = 0; mat.b2 = 1; mat.b3 = 0; mat.b4 = 0;
	mat.c1 = 0; mat.c2 = 0; mat.c3 = 1; mat.c4 = 0;
	mat.d1 = 0; mat.d2 = 0; mat.d3 = 0; mat.d4 = 1;
}

void SetScaling(aiMatrix4x4& mat, aiVector3D Scaling)
{
	mat.a1 = Scaling.x;
	mat.b2 = Scaling.y;
	mat.c3 = Scaling.z;
}

void SetRotation(aiMatrix4x4& mat, aiQuaternion Rotation)
{
	aiMatrix3x3 matRot = Rotation.GetMatrix();
	mat.a1 = matRot.a1; mat.a2 = matRot.a2; mat.a3 = matRot.a3;
	mat.b1 = matRot.b1; mat.b2 = matRot.b2; mat.b3 = matRot.b3;
	mat.c1 = matRot.c1; mat.c2 = matRot.c2; mat.c3 = matRot.c3;
}

void SetTranslation(aiMatrix4x4& mat, aiVector3D Translation)
{
	mat.a4 = Translation.x;
	mat.b4 = Translation.y;
	mat.c4 = Translation.z;
}

const GLfloat* ConvertToOpenGL(aiMatrix4x4& mat)
{
	float m[4][4];

	m[0][0] = mat.a1; m[0][1] = mat.b1; m[0][2] = mat.c1; m[0][3] = mat.d1;
	m[1][0] = mat.a2; m[1][1] = mat.b2; m[1][2] = mat.c2; m[1][3] = mat.d2;
	m[2][0] = mat.a3; m[2][1] = mat.b3; m[2][2] = mat.c3; m[2][3] = mat.d3;
	m[3][0] = mat.a4; m[3][1] = mat.b4; m[3][2] = mat.c4; m[3][3] = mat.d4;

	return &(m[0][0]);
}

mat4 ConvertToGLM(aiMatrix4x4& mat)
{
	mat4 m;

	m[0][0] = mat.a1; m[0][1] = mat.b1; m[0][2] = mat.c1; m[0][3] = mat.d1;
	m[1][0] = mat.a2; m[1][1] = mat.b2; m[1][2] = mat.c2; m[1][3] = mat.d2;
	m[2][0] = mat.a3; m[2][1] = mat.b3; m[2][2] = mat.c3; m[2][3] = mat.d3;
	m[3][0] = mat.a4; m[3][1] = mat.b4; m[3][2] = mat.c4; m[3][3] = mat.d4;

	return m;
}
