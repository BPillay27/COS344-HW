#include <stdio.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
using namespace std;

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>

#include "shader.hpp"

GLuint LoadShaders(const char * vertex_file_path,const char * fragment_file_path){

	// Open log file for diagnostics
	std::ofstream logFile("shader_log.txt", std::ios::app);
	auto log = [&](const std::string &s){
		printf("%s\n", s.c_str());
		if (logFile.is_open()) logFile << s << "\n";
	};


	// Create the shaders
	GLuint VertexShaderID = glCreateShader(GL_VERTEX_SHADER);
	GLuint FragmentShaderID = glCreateShader(GL_FRAGMENT_SHADER);

	// Read the Vertex Shader code from the file
	std::string VertexShaderCode;
	std::ifstream VertexShaderStream(vertex_file_path, std::ios::in);
	if(VertexShaderStream.is_open()){
		std::stringstream sstr;
		sstr << VertexShaderStream.rdbuf();
		VertexShaderCode = sstr.str();
		VertexShaderStream.close();
		log(std::string("Loaded vertex shader: ") + vertex_file_path);
	}else{
		log(std::string("Impossible to open vertex shader: ") + vertex_file_path);
		getchar();
		return 0;
	}

	// Read the Fragment Shader code from the file
	std::string FragmentShaderCode;
	std::ifstream FragmentShaderStream(fragment_file_path, std::ios::in);
	if(FragmentShaderStream.is_open()){
		std::stringstream sstr;
		sstr << FragmentShaderStream.rdbuf();
		FragmentShaderCode = sstr.str();
		FragmentShaderStream.close();
		log(std::string("Loaded fragment shader: ") + fragment_file_path);
	}
	else{
		log(std::string("Impossible to open fragment shader: ") + fragment_file_path);
		getchar();
		return 0;
	}

	GLint Result = GL_FALSE;
	int InfoLogLength;


	// Compile Vertex Shader
	log(std::string("Compiling vertex shader: ") + vertex_file_path);

	// Strip UTF-8 BOM if present
	if (VertexShaderCode.size() >= 3 && (unsigned char)VertexShaderCode[0] == 0xEF && (unsigned char)VertexShaderCode[1] == 0xBB && (unsigned char)VertexShaderCode[2] == 0xBF) {
		VertexShaderCode.erase(0,3);
	}
	char const * VertexSourcePointer = VertexShaderCode.c_str();
	glShaderSource(VertexShaderID, 1, &VertexSourcePointer , NULL);
	glCompileShader(VertexShaderID);

	// Check Vertex Shader
	glGetShaderiv(VertexShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(VertexShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	printf("Vertex shader compile status: %d\n", Result);
	if ( InfoLogLength > 0 ){
		std::vector<char> VertexShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(VertexShaderID, InfoLogLength, NULL, &VertexShaderErrorMessage[0]);
		log(std::string("Vertex shader log: ") + &VertexShaderErrorMessage[0]);
	}



	// Compile Fragment Shader
	log(std::string("Compiling fragment shader: ") + fragment_file_path);
	if (FragmentShaderCode.size() >= 3 && (unsigned char)FragmentShaderCode[0] == 0xEF && (unsigned char)FragmentShaderCode[1] == 0xBB && (unsigned char)FragmentShaderCode[2] == 0xBF) {
		FragmentShaderCode.erase(0,3);
	}
	char const * FragmentSourcePointer = FragmentShaderCode.c_str();
	glShaderSource(FragmentShaderID, 1, &FragmentSourcePointer , NULL);
	glCompileShader(FragmentShaderID);

	// Check Fragment Shader
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	glGetShaderiv(FragmentShaderID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	printf("Fragment shader compile status: %d\n", Result);
	if ( InfoLogLength > 0 ){
		std::vector<char> FragmentShaderErrorMessage(InfoLogLength+1);
		glGetShaderInfoLog(FragmentShaderID, InfoLogLength, NULL, &FragmentShaderErrorMessage[0]);
		log(std::string("Fragment shader log: ") + &FragmentShaderErrorMessage[0]);
	}
	// If compilation failed, return 0 to indicate error
	glGetShaderiv(FragmentShaderID, GL_COMPILE_STATUS, &Result);
	if (Result != GL_TRUE) {
		printf("Fragment shader failed to compile.\n");
		return 0;
	}



	// Link the program
	log("Linking program");
	GLuint ProgramID = glCreateProgram();
	glAttachShader(ProgramID, VertexShaderID);
	glAttachShader(ProgramID, FragmentShaderID);
	glLinkProgram(ProgramID);

	// Check the program
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	glGetProgramiv(ProgramID, GL_INFO_LOG_LENGTH, &InfoLogLength);
	printf("Program link status: %d\n", Result);
	if ( InfoLogLength > 0 ){
		std::vector<char> ProgramErrorMessage(InfoLogLength+1);
		glGetProgramInfoLog(ProgramID, InfoLogLength, NULL, &ProgramErrorMessage[0]);
		log(std::string("Program link log: ") + &ProgramErrorMessage[0]);
	}
	glGetProgramiv(ProgramID, GL_LINK_STATUS, &Result);
	if (Result != GL_TRUE) {
		printf("Shader program failed to link.\n");
		return 0;
	}

	
	glDetachShader(ProgramID, VertexShaderID);
	glDetachShader(ProgramID, FragmentShaderID);
	
	glDeleteShader(VertexShaderID);
	glDeleteShader(FragmentShaderID);

	return ProgramID;
}


