#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:

	unsigned int ID;

	Shader(const char* vertexPath, const char* fragmentPath) {

		std::string vertexCode, fragmentCode;
		std::ifstream vShaderFile, fShaderFile;

		vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

		try {
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);

			std::stringstream vertexShaderStream, fragmentShaderStream;

			vertexShaderStream << vShaderFile.rdbuf();
			fragmentShaderStream << fShaderFile.rdbuf();

			vShaderFile.close();
			fShaderFile.close();

			vertexCode = vertexShaderStream.str();
			fragmentCode = fragmentShaderStream.str();
		}
		catch (std::ifstream::failure e) {
			std::cout << "ERROR : FAILED to read the Shader files." << '\n';
		}
		const char* vShaderCode = vertexCode.c_str();
		const char* fShaderCode = fragmentCode.c_str();

		unsigned int vertex, fragment;
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		compilationCheck(vertex, "VERTEX");

		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		compilationCheck(fragment, "FRAGMENT");

		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glLinkProgram(ID);
		compilationCheck(ID, "PROGRAM");

		glDeleteShader(vertex);
		glDeleteShader(fragment);
	}

	void use() {
		glUseProgram(ID);
	}

	void setBool(const std::string& name, bool value) {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
	}
	void setInt(const std::string& name, int value) {
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat(const std::string& name, float value) {
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}
	void setFloat4(const std::string& name, float value1,float value2, float value3) {
		glUniform4f(glGetUniformLocation(ID, name.c_str()), value1,value2,value3,1.0f);
	}
	
	void setMat4(const std::string& name, glm::mat4 value) {
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1,GL_FALSE,&value[0][0]);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const {
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}

private:

	void compilationCheck(unsigned int shader, std::string type) {
		int success;
		char infolog[1024];

		if (type == "PROGRAM") {
			glGetProgramiv(ID, GL_LINK_STATUS, &success);
			if (!success) {
				glGetProgramInfoLog(ID, 1024, NULL, infolog);
				std::cout << "ERROR : Failed to Link the Shaders. " << infolog << '\n';
			}
		}
		else {
			glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

			if (!success) {
				glGetShaderInfoLog(shader, 1024, NULL, infolog);
				std::cout << "ERROR : Compilation of "<< type<< " Shader Failed. " << infolog << '\n';
			}
		}
	}
};


#endif // !SHADER_H
