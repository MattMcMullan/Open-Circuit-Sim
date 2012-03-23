// Vertex Shader - file "minimal.vert"

#version 330 core

in vec3 in_Position;
in vec3 in_ColorTexCoords;
out vec3 ColorTexCoords;
uniform mat4 mvp;

void main(void) {
	ColorTexCoords = in_ColorTexCoords;
	gl_Position = mvp*vec4(in_Position, 1.0);
}
