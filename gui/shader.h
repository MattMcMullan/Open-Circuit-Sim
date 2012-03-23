#ifndef __shader_h__
#define __shader_h__
#include "libs.h"
#include <iostream>

// a basic structure for storing a shader's source code and its length
struct ShaderSource {
	ShaderSource(): length(0), source(0) {}
	ShaderSource(char *s);
	ShaderSource(char *s, GLint l);
	GLint length;
	char *source;
};

// stores the callbacks required by the Shader Class
struct ShaderCallbacks {
	ShaderCallbacks() {
		bindAttributeLocations = 0;
	}
	ShaderCallbacks(void  (*attrib)(GLuint)) {
		bindAttributeLocations = attrib;
	}
	void  (*bindAttributeLocations)(GLuint);
};

class Shader {
private:
	GLuint program;
	GLuint fragment;
	GLuint vertex;
	ShaderSource vertError, fragError, progError, genError;
	ShaderCallbacks callbacks;
	// helper functions
	bool loadFile(const char *fName, ShaderSource &out);
	GLuint compileShader(const ShaderSource &source, GLuint type);
public:
	// constructors
	Shader(): program(0), fragment(0), vertex(0) {};
	Shader(const ShaderCallbacks &c): program(0), fragment(0), vertex(0) {
		callbacks = c;
	}
	~Shader();
	void setCallbacks(const ShaderCallbacks &c) { callbacks = c; }
	// loading shaders from various sources
	bool loadShaderFiles(const char *vertFile, const char *fragFile);
	bool loadShaderSource(const char *vertSource, const char *fragSource);
	bool loadShaderSource(const ShaderSource &vertSource, const ShaderSource &fragSource);
	// send the error text to a stream somewhere
	void getErrortext(std::ostream &ostr) const;
	// make this shader active or inactive
	void bind() const;
	void unbind() const;
	// todo: find a better way
	GLuint id() const { return program; }
};

#endif
