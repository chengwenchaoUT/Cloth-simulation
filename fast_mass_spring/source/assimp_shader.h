#pragma once
#ifndef _ASSIMP_SHADER_H_
#define _ASSIMP_SHADER_H_

#include "math_headers.h"
#include "opengl_headers.h"
#include "camera.h"

#include <string>
#include <iostream>
#include <fstream>
#include <sstream>


using namespace std;

class Shader
{
public:
	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath);

	void use();
	void deactive();

	void setBool(const string& name, bool value) const;
	void setInt(const string& name, int value) const;
	void setFloat(const string& name, float value) const;
	//void setMat4(const string& name, mat4 value) const;

private:

};

#endif