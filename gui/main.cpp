#include <GL/glew.h>
#include <GL/glfw.h>
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <Eigen/Dense>
#include <Eigen/Geometry>
using std::cout; using std::endl;
using namespace Eigen;

// global variables to store the scene geometry
GLuint vertexArrayID;
GLuint bufferIDs[2];
Matrix4f view = Matrix4f::Identity();
Matrix4f rightBox = Matrix4f::Identity();
GLfloat ident[16] = {1,0,0,0, 0,1,0,0, 0,0,1,0, 0,0,0,1};
GLuint shaderProgram;
GLuint initX=400, initY=300, curX, curY;

// initialize the scene geometry
void initGeometry() {
	static const GLfloat rawVertexData[9] = { -1.0f,-1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,-1.0f,0.0f };
	static const GLfloat rawColorData[9]  = {  1.0f,0.0f,0.0f,  0.0f,1.0f,0.0f,  0.0f,0.0f,1.0f };
	// create a new renderable object and set it to be active
	glGenVertexArrays(1,&vertexArrayID);
	glBindVertexArray(vertexArrayID);
	// create buffers for associated data
	glGenBuffers(2,bufferIDs);
	// set a buffer to be active and shove vertex data into it
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), rawVertexData, GL_STATIC_DRAW);
	// bind that data to shader variable 0
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	// set a buffer to be active and shove color data into it
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 9*sizeof(GLfloat), rawColorData, GL_STATIC_DRAW);
	// bind that data to shader variable 1
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}

// loadFile - loads text file into char* fname
// allocates memory - so need to delete after use
// size of file returned in fSize
char* loadFile(const char *fname, GLint &fSize) {
	std::ifstream::pos_type size;
	char * memblock;

	// file read based on example in cplusplus.com tutorial
	std::ifstream file (fname, std::ios::in|std::ios::binary|std::ios::ate);
	if (file.is_open()) {
		size = file.tellg();
		fSize = (GLuint) size;
		memblock = new char [size];
		file.seekg (0, std::ios::beg);
		file.read (memblock, size);
		file.close();
	} else {
		std::cout << "Unable to open " << fname << std::endl;
		exit(1);
	}
	cout << fname << " loaded" << endl;
	return memblock;
}

GLuint compileShader(const char *fname, GLuint type) {
	GLuint shader;
	GLint length;
	// create a shader ID
	shader = glCreateShader(type);
	// load the file into memory and try to compile it
	char *source = loadFile(fname,length);
	glShaderSource(shader, 1, (const GLchar**)&source,&length);
	GLint compiled;
	glCompileShader(shader);
	// print out errors if they're there
	glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);
	if (!compiled) {
		std::cout << fname << " failed to compile" << std::endl;
		// find the error length
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
		if (length>0) {
			// print the errors
			char *error = new GLchar[length];
			GLint written;
			glGetShaderInfoLog(shader,length, &written, error);
			std::cout << "Error log:\n" << error << std::endl;
			delete[] error;
		}
	}
	delete[] source;
	return shader;
}

void initShaders() {
	GLuint p, f, v;
	// get the shader code
	v = compileShader("minimal.vert",GL_VERTEX_SHADER);
	f = compileShader("minimal.frag",GL_FRAGMENT_SHADER);	
	// the GLSL program links shaders together to form the render pipeline
	p = glCreateProgram();
	// assign numerical IDs to the variables that we pass to the shaders 
	glAttachShader(p,v);
	glAttachShader(p,f);
	glBindAttribLocation(p,0, "in_Position");
	glBindAttribLocation(p,1, "in_Color");
	// bind our shaders to this program
	// link things together and activate the shader
	glLinkProgram(p);
	glUseProgram(p);
	shaderProgram = p;
}

void GetRightBoxMatrix(Matrix4f &target, GLuint minWidth, GLuint minHeight) {
	float coordMinWidth = ((float)(minWidth*initX))/((float)curX);
	if (curY>minHeight) minHeight=curY;
	float coordMinHeight = ((float)(minHeight*initY))/((float)curY);
	// TODO scale to min screen
	
}

// Callback for when the window is resized
void GLFWCALL windowResize( int width, int height ) {
	curX=width; curY=height;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	view = Affine3f(Affine3f::Identity()).scale(Vector3f(((float)initX)/((float)width), ((float)initY)/((float)height), 1.)).matrix();
}

int main( void ) {
	// Initialize GLFW
	if( !glfwInit() ) {
		exit( EXIT_FAILURE );
	}
	// Open an OpenGL window
	if( !glfwOpenWindow( initX,initY, 0,0,0,0,0,0, GLFW_WINDOW ) ) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	glfwSetWindowTitle("Open Circuit Simulator");
	glfwSetWindowSizeCallback( windowResize );
	glfwSwapInterval( 1 );
	windowResize(initX,initY);
	glewInit();
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
	glClearDepth(1.0f);
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LEQUAL);
	initGeometry();
	initShaders();
	// Main loop
	int running = GL_TRUE;
	while( running ) {
		// OpenGL rendering goes here...
		glClear( GL_COLOR_BUFFER_BIT );
		// draw the triangle
		glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "viewMatrix"), 1, GL_FALSE, (float*)&view);
		glBindVertexArray(vertexArrayID);
		glDrawArrays(GL_TRIANGLES,0,3);
		// Swap front and back rendering buffers
		glfwSwapBuffers();
		// Check if ESC key was pressed or window was closed
		running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );
	}
	// Close window and terminate GLFW
	glfwTerminate();
	// Exit program
	exit( EXIT_SUCCESS );
}
