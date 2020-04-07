// ---------------------------------------------------------------------------------//
// Copyright (c) 2013, Regents of the University of Pennsylvania                    //
// All rights reserved.                                                             //
//                                                                                  //
// Redistribution and use in source and binary forms, with or without               //
// modification, are permitted provided that the following conditions are met:      //
//     * Redistributions of source code must retain the above copyright             //
//       notice, this list of conditions and the following disclaimer.              //
//     * Redistributions in binary form must reproduce the above copyright          //
//       notice, this list of conditions and the following disclaimer in the        //
//       documentation and/or other materials provided with the distribution.       //
//     * Neither the name of the <organization> nor the                             //
//       names of its contributors may be used to endorse or promote products       //
//       derived from this software without specific prior written permission.      //
//                                                                                  //
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND  //
// ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED    //
// WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE           //
// DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY               //
// DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES       //
// (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;     //
// LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND      //
// ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT       //
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS    //
// SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.                     //
//                                                                                  //
// Contact Tiantian Liu (ltt1598@gmail.com) if you have any questions.              //
//----------------------------------------------------------------------------------//

#include "io_mesh.h"

MeshLoader::MeshLoader()
{
    m_vertices.clear();
    m_faces.clear();
    m_tets.clear();
    load_success = false;
}

// courtesy to Yusuf 
MeshLoader::MeshLoader(char* filename, float scale, glm::vec3 translate)
{
    m_vertices.clear();
    m_faces.clear();
    m_tets.clear();
    load_success = false;

    std::cout << "MeshLoader initializing (to " << filename << ").." << std::endl;

    std::ifstream infile(filename);
    if(!infile.is_open())
    {
        std::cout << "cannot read " << filename << std::endl;
        return;
    }

    char buffer[256];
    glm::vec3 pos;
    Face face; 
    Tet tet; 
    char ignore[256];
    while(!infile.eof())
    {
        infile >> buffer;
        if (strcmp(buffer, "Vertices") == 0)
        {
            int num_vertices;
            infile >> num_vertices;
            for (int i = 0; i < num_vertices; i++)
            {
                infile >> pos.x >> pos.y >> pos.z >> ignore;
                m_vertices.push_back(pos * scale + translate);
            }
            if (m_vertices.size() != num_vertices)
            {
                std::cout << "Init MeshLoader: error on reading vertices." << std::endl;
                return;
            }
        }
        else if (strcmp(buffer, "Triangles") == 0)
        {
            int num_face;
            infile >> num_face;
            for (int i = 0; i < num_face; i++)
            {
                infile >> face.id1 >> face.id2 >> face.id3 >> ignore;
                face.IDMinusMinus();
                m_faces.push_back(face);
            }
            if (m_faces.size() != num_face)
            {
                std::cout << "Init MeshLoader: error on reading faces." << std::endl;
                return;
            }
        }
        else if (strcmp(buffer, "Tetrahedra") == 0)
        {
            int num_tet;
            infile >> num_tet;
            for (int i = 0; i < num_tet; i++)
            {
                infile >> tet.id1 >> tet.id2 >> tet.id3 >> tet.id4 >> ignore;
                tet.IDMinusMinus();
                m_tets.push_back(tet);
            }
            if (m_tets.size() != num_tet)
            {
                std::cout << "Init MeshLoader: error on reading tets." << std::endl;
                return;
            }
        }
    }

    load_success = true;
}

MeshLoader::~MeshLoader()
{
    m_vertices.clear();
    m_faces.clear();
    m_tets.clear();
}



//--------------------------------------- Model Class -----------------------------------------------//
Model::Model(const char* path, float scale, glm::vec3 translate)
{
	this->loadModel(path, scale, translate);
}

void Model::loadModel(string path, float scale, glm::vec3 translate)
{
	importer.SetPropertyInteger(AI_CONFIG_PP_RVC_FLAGS, aiComponent_TEXCOORDS);
	m_scene = importer.ReadFile(path, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_JoinIdenticalVertices
	| aiProcess_RemoveComponent);
	if (!m_scene || m_scene->mFlags == AI_SCENE_FLAGS_INCOMPLETE || !m_scene->mRootNode)
	{
		cout << "ERROR::ASSIMP::" << importer.GetErrorString() << endl;
		return;
	}

	this->directory = path.substr(0, path.find_last_of('/'));

	this->processNode(m_scene->mRootNode, m_scene, scale, translate);
}

void Model::processNode(aiNode* node, const aiScene* scene, float scale, glm::vec3 translate)
{
	for (int i = 0; i < node->mNumMeshes; i++)
	{
		aiMesh* mesh = scene->mMeshes[node->mMeshes[i]];
		this->meshes.push_back(processMesh(mesh, scene, scale, translate));
	}

	for (int i = 0; i < node->mNumChildren; i++)
	{
		this->processNode(node->mChildren[i], scene, scale, translate);
	}
}

Model::AssimpMesh Model::processMesh(aiMesh* mesh, const aiScene* scene, float scale, glm::vec3 translate)
{
	vector<glm::vec3>vertices;
	vector<aiFace> faces;
	vector<glm::vec3>normals;

	// vertices and normals
	for (int i = 0; i < mesh->mNumVertices; i++)
	{
		glm::vec3 position;
		position.x = mesh->mVertices[i].x;
		position.y = mesh->mVertices[i].y;
		position.z = mesh->mVertices[i].z;
		vertices.push_back(position * scale + translate);

		glm::vec3 normal;
		normal.x = mesh->mNormals[i].x;
		normal.y = mesh->mNormals[i].y;
		normal.z = mesh->mNormals[i].z;
		normals.push_back(normal);
	}

	// vertices indices(faces)
	for (int i = 0; i < mesh->mNumFaces; i++)
	{
		aiFace face = mesh->mFaces[i];
		faces.push_back(face);
	}

	return AssimpMesh(vertices, faces, normals);
}
