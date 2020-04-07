#include "assimp_shader.h"

Shader::Shader(const char* vertexPath, const char* fragmentPath)
{
	string vertexCode;
	string fragmentCode;
	ifstream vertexFile;
	ifstream fragmentFile;

	vertexFile.exceptions(ifstream::badbit);
	fragmentFile.exceptions(ifstream::badbit);
	try
	{
		//open files
		vertexFile.open(vertexPath);
		fragmentFile.open(fragmentPath);
		stringstream vertexStream, fragmentStream;

		//read files
		vertexStream << vertexFile.rdbuf();
		fragmentStream << fragmentFile.rdbuf();

		//close files
		vertexFile.close();
		fragmentFile.close();

		vertexCode = vertexStream.str();
		fragmentCode = fragmentStream.str();

	}
	catch (ifstream::failure e)
	{
		cout << "ERROR: Shader read fails." << endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();



	//Shader
	unsigned int vertex, fragment;
	//vertex shader
	vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vShaderCode, NULL);
	glCompileShader(vertex);

	//fragment shader
	fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fShaderCode, NULL);
	glCompileShader(fragment);

	//shader program
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	glLinkProgram(ID);
	glDeleteShader(vertex);
	glDeleteShader(fragment);


}

void Shader::use() {
	glUseProgram(ID);
}

void Shader::deactive()
{
	glUseProgram(0);
}

void Shader::setBool(const string& name, bool value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
}
void Shader::setInt(const string& name, int value) const
{
	glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
}
void Shader::setFloat(const string& name, float value) const
{
	glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
}