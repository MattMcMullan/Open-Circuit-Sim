#include "libs.h"
#include "shader.h"
#include <cstring>
#include <fstream>
#include <iostream>

ShaderSource::ShaderSource(char *s) {
	source = s;
	length = strlen(s);
}
ShaderSource::ShaderSource(char *s, GLint l) {
	source = s;
	length = l;
}

Shader::~Shader() {
	if (vertex)
		glDeleteShader(vertex);
	if (fragment)
		glDeleteShader(fragment);
	if (program)
		glDeleteProgram(program);
	if (vertError.source)
		delete[] vertError.source;
	if (fragError.source)
		delete[] fragError.source;
	if (progError.source)
		delete[] progError.source;
	if (genError.source)
		delete[] genError.source;
}

void Shader::bind() const {
	glUseProgram(program);
}
void Shader::unbind() const {
	GLint active;
	glGetIntegerv(GL_CURRENT_PROGRAM,&active);
	if ((GLuint)active==program) {
		glUseProgram(0);
	}
}

bool Shader::loadFile(const char *fName, ShaderSource &out) {
	std::ifstream::pos_type size;
	char * memblock;
	// load the file
	std::ifstream file (fName, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		out.length = (GLuint) size;
		memblock = new char [size];
		file.seekg (0, std::ios::beg);
		file.read (memblock, size);
		file.close();
		out.source = memblock;
		return true;
	}
	// send out a message if it fails
	out.length = 22+strlen(fName);
	out.source = new char[out.length];
	strncpy(out.source,"Unable to open file: ",21);
	strcpy(out.source+21,fName);
	return false;
}

bool Shader::loadShaderFiles(const char *vertFile, const char *fragFile) {
	ShaderSource vertFileS, fragFileS;
	if (!loadFile(vertFile,vertFileS)) return false;
	if (!loadFile(fragFile,fragFileS)) return false;
	bool retval = loadShaderSource(vertFileS,fragFileS);
	delete[] vertFileS.source;
	delete[] fragFileS.source;
	return retval;
}
bool Shader::loadShaderSource(const char *vertSource, const char *fragSource) {
	ShaderSource vert((char*)vertSource), frag((char*)fragSource);
	return loadShaderSource(vertError,fragError);
}
GLuint Shader::compileShader(const ShaderSource &source, GLuint type) {
	GLuint shader;
	GLint length;
	// create a shader ID
	shader = glCreateShader(type);
	if (!shader) {
		genError.length = strlen("Failed to allocate an OpenGL Shader object");
		genError.source = new char[genError.length];
		strncpy(genError.source,"Failed to allocate an OpenGL Shader object",genError.length);
		// todo: parse the OpenGL Errors
		return 0;
	}
	// try to compile it
	glShaderSource(shader, 1, (const GLchar**)&source.source,&source.length);
	GLint compiled;
	glCompileShader(shader);
	// print out errors if they're there
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		// find the error length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length>0) {
			// print the errors
			genError.length = length;
			char *error = new char[length];
			genError.source = error;
			GLint written;
			glGetShaderInfoLog(shader,length, &written, error);
		}
		return 0;
	}
	return shader;
}
bool Shader::loadShaderSource(const ShaderSource &vertSource, const ShaderSource &fragSource) {
	fragment = compileShader(fragSource,GL_FRAGMENT_SHADER);
	if (!fragment) {
		fragError = genError;
		genError.length = 0; genError.source = 0;
	}
	vertex = compileShader(vertSource,GL_VERTEX_SHADER);
	if (!vertex) {
		vertError = genError;
		genError.length = 0; genError.source = 0;
		if (fragment) {
			glDeleteShader(fragment);
			fragment = 0;
		}
		return false;
	}
	if (!fragment) {
		return false;
	}
	// the GLSL program links shaders together to form the render pipeline
	program = glCreateProgram();
	if (!program) {
		progError.length = strlen("Shader program allocation failed");
		progError.source = new char[progError.length];
		strcpy(progError.source,"Shader program allocation failed");
		glDeleteShader(fragment); fragment = 0;
		glDeleteShader(vertex); vertex = 0;
		return false;
	}
	// assign numerical IDs to the variables that we pass to the shaders 
	if (callbacks.bindAttributeLocations!=0) callbacks.bindAttributeLocations(program);
	// bind our shaders to this program
	glAttachShader(program,vertex);
	glAttachShader(program,fragment);
	// link things together and activate the shader
	glLinkProgram(program);
	// check to see if it linked properly
	GLint info;
	glGetProgramiv(program,GL_LINK_STATUS,&info);
	if (info==GL_FALSE) {
		glGetProgramiv(program,GL_INFO_LOG_LENGTH,&info);
		if (info>0) {
			progError.length = info;
			char *error = new char[info];
			progError.source = error;
			glGetProgramInfoLog(program,info, &info, error);
		} else {
			progError.length = strlen("Linking the shader program failed");
			progError.source = new char[progError.length];
			strcpy(progError.source,"Linking the shader program failed");
		}
		glDeleteShader(fragment); fragment = 0;
		glDeleteShader(vertex); vertex = 0;
		return false;
	}
	return true;
}
void Shader::getErrortext(std::ostream &ostr) const {
	ostr << "Shader Errors:\n";
	if (vertError.source!=0) {
		ostr << "Vertex Shader Failed to Load:\n" << vertError.source << "\n\n";
	}
	if (fragError.source!=0) {
		ostr << "Fragment Shader Failed to Load:\n" << fragError.source << "\n\n";
	}
	if (progError.source!=0) {
		ostr << "Shader Program Failed to Create:\n" << progError.source << "\n\n";
	}
	if (genError.source!=0) {
		ostr << "Generic Shader Error:\n" << genError.source << "\n\n";
	}
	if (!vertError.source && !fragError.source && !progError.source && !genError.source) {
		ostr << "Unknown or non-existent error\n";
	}
	ostr << std::flush;
}
