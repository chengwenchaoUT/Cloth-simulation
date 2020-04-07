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

#ifndef _IO_MESH_H_
#define _IO_MESH_H_

#include <vector>
#include <fstream>
#include <iostream>
#include <string>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include "glm.hpp"

using namespace Assimp;
using namespace std;

class MeshLoader{
public:
    
    struct Face{
        unsigned int id1,id2,id3;
        Face() {}
        Face(int a, int b, int c) : id1(a), id2(b), id3(c){}
        void IDMinusMinus() {id1--; id2--; id3--;}
    };

    struct Tet{
        unsigned int id1,id2,id3,id4;
        Tet() {}
        Tet(int a, int b, int c, int d) : id1(a), id2(b), id3(c), id4(d){}
        void IDMinusMinus() {id1--; id2--; id3--; id4--;}
    };

    MeshLoader();
    MeshLoader(char* filename, float scale = 10.0f, glm::vec3 translate = glm::vec3(0.0f, 4.0f, 0.0f));
    virtual ~MeshLoader();

    inline bool Info() {return load_success;}

    //Vertices, edges, and faces information
    std::vector<glm::vec3> m_vertices;
    std::vector<Face> m_faces;
    std::vector<Tet> m_tets;
    
    bool load_success;
};

class Model {
public:
	Model(const char* path, float scale = 0.1f, glm::vec3 translate = glm::vec3(0.0f, -0.5f, 0.0f));

	struct AssimpMesh {
		AssimpMesh() {}
		AssimpMesh(vector<glm::vec3> vertices, vector<aiFace> faces, vector<glm::vec3> normals) : m_vertices(vertices), m_faces(faces), m_normals(normals){}

		~AssimpMesh() {
			m_vertices.clear();
			m_faces.clear();
			m_normals.clear();
		}
		//Vertices, edges, and faces information
		vector<glm::vec3> m_vertices;
		vector<aiFace> m_faces;
		vector<glm::vec3> m_normals;
	};
	
	vector<AssimpMesh> meshes;

	string directory;
	const aiScene* m_scene;
	Importer importer;

	void loadModel(string path, float scale, glm::vec3 translate);
	void processNode(aiNode* node, const aiScene* scene, float scale, glm::vec3 translate);
	AssimpMesh processMesh(aiMesh* mesh, const aiScene* scene, float scale, glm::vec3 translate);
};

#endif