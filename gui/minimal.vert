// Vertex Shader - file "minimal.vert"

#version 130
uniform mat4 viewMatrix;
uniform mat4 modelMatrix;
in  vec3 in_Position;
in  vec3 in_Color;
out vec3 ex_Color;

void main(void) {
	ex_Color = in_Color;
	gl_Position = viewMatrix*vec4(in_Position, 1.0);
}
