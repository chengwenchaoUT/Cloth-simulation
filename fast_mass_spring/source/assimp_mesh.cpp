#include "assimp_mesh.h"


AssimpMesh::AssimpMesh(vector<Vertex> vertices, vector<unsigned int> indices, vector<Texture> textures,
	vector<BoneData> bones)
{
	this->vertices = vertices;
	this->indices = indices;
	this->textures = textures;
	this->bones = bones;
	this->draw_pos = vec3(0.0, -5.0, 0.0);

	this->setupMesh();
}

void AssimpMesh::setVertices(vector<Vertex> vertices)
{
	this->vertices = vertices;
}

void AssimpMesh::setTransMat4(mat4 projection, mat4 view, mat4 model)
{
	this->ai_projection = projection;
	this->ai_view = view;
	this->ai_model = model;
}

void AssimpMesh::Draw(Shader* shader, Camera* camera)
{
	unsigned int indexOfDiffuse = 1;
	unsigned int indexOfSpecular = 1;

	for (int i = 0; i < this->textures.size(); i++)
	{
		glActiveTexture(GL_TEXTURE0 + i);
		stringstream ss;
		string number;
		string name = this->textures[i].type;

		if (name == "texture_diffuse") {
			ss << indexOfDiffuse++;
		}
		else if (name == "texture_specular")
		{
			ss << indexOfSpecular++;
		}
		number = ss.str();

		glUniform1f(glGetUniformLocation(shader->ID, (name + number).c_str()), i);
		glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
	}

	// view/projection transformations
	mat4 projection = camera->GetProjectionMatrix();
	mat4 view = camera->GetViewMatrix();
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "projection"), 1, GL_FALSE, &projection[0][0]);
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "view"), 1, GL_FALSE, &view[0][0]);

	// world transformation
	glm::mat4 model(1.0f);
	model = glm::translate(model, this->draw_pos);	// Translate it down a bit so it's at the center of the scene
	model = scale(model, glm::vec3(0.1f, 0.1f, 0.1f));	// It's a bit too big for our scene, so scale it down
	glUniformMatrix4fv(glGetUniformLocation(shader->ID, "model"), 1, GL_FALSE, &model[0][0]);

	// set trans matrix
	setTransMat4(projection, view, model);

	glBindVertexArray(this->VAO);
	glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
	glBindVertexArray(0);
}

unsigned int AssimpMesh::getVAO()
{
	return this->VAO;
}



void AssimpMesh::setupMesh()
{
	glGenVertexArrays(1, &this->VAO);
	glGenBuffers(1, &this->VBO_Vertex);
	glGenBuffers(1, &this->VBO_Bone);
	glGenBuffers(1, &this->EBO);

	glBindVertexArray(this->VAO);

	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_Vertex);
	glBufferData(GL_ARRAY_BUFFER, this->vertices.size() * sizeof(Vertex), &this->vertices[0], GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, this->indices.size() * sizeof(unsigned int), &this->indices[0], GL_STATIC_DRAW);

	// vertex coordinate
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

	// normal coordinate
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, Normal));

	// texture coordinate
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)offsetof(Vertex, TexCoords));


	// bones
	glBindBuffer(GL_ARRAY_BUFFER, this->VBO_Bone);
	glBufferData(GL_ARRAY_BUFFER, this->bones.size() * sizeof(BoneData), &this->bones[0], GL_STATIC_DRAW);
	// bone id 
	glEnableVertexAttribArray(3);
	glVertexAttribIPointer(3, 4, GL_INT, sizeof(BoneData), (GLvoid*)0);

	// bone weight
	glEnableVertexAttribArray(4);
	glVertexAttribPointer(4, 4, GL_FLOAT, GL_FALSE, sizeof(BoneData), (GLvoid*)offsetof(BoneData, bone_weights));

	glBindVertexArray(0);
}