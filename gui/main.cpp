#include "libs.h"
#include <stdlib.h>
#include <fstream>
#include <iostream>
#include <vector>
#include "shader.h"
#include "textureLoader.h"
#include "matrixStack.h"
#include "context.h"
using std::cout; using std::endl;

// global variables to store the scene geometry
GLuint vertexArrayID;
GLuint bufferIDs[2];
// shader program
Shader shader;
// mvp stack
matrixStack mvpStack;
// rendering data
std::vector<GLuint> textures;
context screen;
// screen width and height
unsigned int SCW = 800, SCH = 600;
context layout,menu;

// initialize the scene geometry
void initGeometry() {
	static const GLfloat rawVertexData[12] = { -1.0f,1.0f,0.0f, -1.0f,-1.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,-1.0f,0.0f };
	static const GLfloat rawTexCoordData[12] = {0.0f,1.0f,0.0f,  0.0f, 0.0f,0.0f,  1.0f,1.0f,0.0f,  1.0f,0.0f,0.0f };
	// create a new renderable object and set it to be active
	glGenVertexArrays(1,&vertexArrayID);
	glBindVertexArray(vertexArrayID);
	// create buffers for associated data
	glGenBuffers(2,bufferIDs);
	
	// set a buffer to be active and shove vertex data into it
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[0]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), rawVertexData, GL_STATIC_DRAW);
	// bind that data to shader variable 0
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);
	
	// set a buffer to be active and shove color data into it
	glBindBuffer(GL_ARRAY_BUFFER, bufferIDs[1]);
	glBufferData(GL_ARRAY_BUFFER, 12*sizeof(GLfloat), rawTexCoordData, GL_STATIC_DRAW);
	// bind that data to shader variable 1
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);
}
void bindShaderAttribLocations(GLuint p) {
	// assign numerical IDs to the variables that we pass to the shaders 
	glBindAttribLocation(p,0, "in_Position");
	glBindAttribLocation(p,1, "in_ColorTexCoords");
}
void initShaders() {
	shader = Shader(ShaderCallbacks(bindShaderAttribLocations));
	if (!shader.loadShaderFiles("textured.vert","textured.frag")) {
		shader.getErrortext(cout);
		exit(1);
	}
	shader.bind();
}
void createSubBox(Eigen::Affine3f &target, int parent_w, int parent_h, int child_w, int child_h, int off_x, int off_y) {
	target = Eigen::Affine3f::Identity();
	#define FDIV(a,b) (((float)a)/((float)b))
	float rw_scale = FDIV(child_w,parent_w);
	float rh_scale = FDIV(child_h,parent_h);
	float mw = ((FDIV(1.0,child_w))*((float)off_x));
	float mh = (FDIV(1.0,child_h)*((float)off_y));
	target.scale(Eigen::Vector3f(rw_scale,rh_scale,0.0f));
	target.translate(Eigen::Vector3f(mw,mh,0.0f));
	#undef FDIV
}
context *castMouseRay(context* scene, vectorStack &mouse, vectorStack &tl, vectorStack &br) {
	context *intersection = 0;
	// calculate all the cool new shit
	mouse.push(); *mouse = scene->transformation * (*mouse);
	tl.push(); *tl = scene->transformation * (*tl);
	br.push(); *br = scene->transformation * (*br);
	//cout << scene->transformation.matrix() << endl;
	//cout << *mouse << endl << *tl  << endl << *br << endl;
	
	// placed in a block to mimimize recursive stack space
	{
		float mx = mouse.peek()[0];
		float my = mouse.peek()[1];
		float minx = (*tl)[0];
		float miny = (*br)[1];
		float maxx = (*br)[0];
		float maxy = (*tl)[1];
		float tmp;
		if (minx > maxx) {
			tmp = minx; minx = maxx; maxx = tmp;
		}
		if (miny > maxy) {
			tmp = miny; miny = maxy; maxy = tmp;
		}
		if (mx>=minx && mx <=maxx && my>=miny && my<=maxy) {
			cout << "true" << endl;
			intersection = scene;
		} else { 
			cout << "false" << endl;
		}
	}
	if (intersection!=0) {
		context *tmp;
		for (std::list<context*>::reverse_iterator it = scene->subcontexts.rbegin(); it!=scene->subcontexts.rend(); ++it) {
			tmp = castMouseRay(*it, mouse, tl, br);
			if (tmp!=0) {
				intersection = tmp;
				break;
			}
		}
	}
	// drop the new shit
	mouse.pop();
	tl.pop();
	br.pop();
	
	return intersection;
}
int mouse_x = 0, mouse_y = 0;
bool dragging = false;
int dragOriginX = 0;
int dragOriginY = 0;
int layoutViewOffsetX = 0;
int layoutViewOffsetY = 0;
float layoutViewScaleX = 1.0f;
float layoutViewScaleY = 1.0f;
int nativeLayoutX = 6900; int layoutW = 6900;
int nativeLayoutY = 6900; int layoutH = 6900;
void reformatLayout() {
	// layout view scale bounds
	if (layoutViewScaleX < .3) layoutViewScaleX = .3;
	if (layoutViewScaleY < .3) layoutViewScaleY = .3;
	// layout view offset bounds
	int maxOffsetX = (int)(layoutW*2-SCW*2) - (int)(layoutW-layoutViewScaleX*layoutW);
	int maxOffsetY = (int)(layoutH*2-SCH*2) - (int)(layoutH-layoutViewScaleY*layoutH);
	int minOffsetX = (int)(layoutW-layoutViewScaleX*layoutW);
	int minOffsetY = (int)(layoutH-layoutViewScaleY*layoutH);
	if (layoutViewOffsetX < minOffsetX) layoutViewOffsetX = minOffsetX;
	if (layoutViewOffsetX > maxOffsetX) layoutViewOffsetX = maxOffsetX;
	if (layoutViewOffsetY < minOffsetY) layoutViewOffsetY = minOffsetY;
	if (layoutViewOffsetY > maxOffsetY) layoutViewOffsetY = maxOffsetY;
	cout << layoutViewOffsetX <<  ": " << layoutViewOffsetY << endl;
	// create the box transformation
	createSubBox(layout.transformation, SCW, SCH, (int)(layoutViewScaleX*layoutW),(int)(layoutViewScaleY*layoutH), layoutW-SCW-layoutViewOffsetX, SCH-layoutH+6+layoutViewOffsetY);
}
void GLFWCALL mousePos( int x, int y) {
	mouse_x = x;
	mouse_y = y;
	if (dragging) {
		//cout << mouse_x-dragOriginX<< ": " << layoutViewOffsetX << endl;
		layoutViewOffsetX += 2*(dragOriginX-mouse_x);
		dragOriginX = mouse_x;
		layoutViewOffsetY += 2*(dragOriginY-mouse_y);
		dragOriginY = mouse_y;
		reformatLayout();
	}
}
void GLFWCALL mouseButton( int button, int action ) {
	float source_x = (((float)mouse_x)/((float)SCW))*2.0f-1.0f;
	float source_y = 1.0f-(((float)mouse_y)/((float)SCH))*2.0f;
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
		cout << "(" << mouse_x << ", " << mouse_y << ") = (" << source_x << ", " << source_y << ")" << endl;
		vectorStack mouseVec = Eigen::Vector4f(source_x,source_y,0.0f,1.0f); mouseVec.push();
		vectorStack tl = Eigen::Vector4f(-1.0f,1.0f,0.0f,1.0f); tl.push();
		vectorStack br = Eigen::Vector4f(1.0f,-1.0f,0.0f,1.0f); br.push();
		
		context *hit = castMouseRay(&screen, mouseVec, tl, br);
		if (hit == &layout) {
			dragging = true;
			dragOriginX = mouse_x;
			dragOriginY = mouse_y;
		}
	}
	if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE && dragging == true) {
		dragging = false;
	}
}
void GLFWCALL mouseWheel(int pos) {
	static int last = -99999;
	if (last == -99999) last = pos;
	int direction = pos-last;
	cout << direction << endl;
	cout << layoutViewScaleX << endl;
	last = pos;
	layoutViewScaleX += ((float)direction)*.1f;
	layoutViewScaleY += ((float)direction)*.1f;
	reformatLayout();
}
// Callback for when the window is resized
void GLFWCALL windowResize( int width, int height ) {
	SCW = width;
	SCH = height;
	glViewport(0,0,(GLsizei)width,(GLsizei)height);
	createSubBox(menu.transformation, SCW, SCH, 200,SCH, SCW-200, 0);
	reformatLayout();
}
void recursiveRender(context *scene) {// bind the texture
	mvpStack.push();
	mvpStack->matrix() *= scene->transformation.matrix();
	if (scene->texture != 0) {
		glUniform1i(glGetUniformLocation(shader.id(),"ColorTex"), 0);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D,textures[scene->texture]);

		// draw the texture
		glBindVertexArray(vertexArrayID);
		glUniformMatrix4fv(glGetUniformLocation(shader.id(), "mvp"), 1, GL_FALSE, mvpStack->data());
		glDrawArrays(GL_TRIANGLE_STRIP,0,4);
	}
	for (std::list<context*>::iterator it = scene->subcontexts.begin(); it!=scene->subcontexts.end(); ++it) {
		recursiveRender(*it);
	}
	mvpStack.pop();
}

int main( int argc, char ** argv ) {
	textureLoader::init();
	
	// Initialize GLFW
	if( !glfwInit() ) {
		exit( EXIT_FAILURE );
	}
	// Open an OpenGL window
	if( !glfwOpenWindow( SCW,SCH, 0,0,0,0,0,0, GLFW_WINDOW ) ) {
		glfwTerminate();
		exit( EXIT_FAILURE );
	}
	
	glfwSetWindowTitle("Open Circuit Sim");
	glfwSetWindowSizeCallback( windowResize );
	glfwSetMouseButtonCallback(mouseButton);
	glfwSetMousePosCallback(mousePos);
	glfwSetMouseWheelCallback( mouseWheel ) ;
	glfwSwapInterval( 1 );
	glewInit();
	glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
	//glClearDepth(1.0f);
	//glEnable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glEnable (GL_BLEND);
	glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glDepthFunc(GL_LEQUAL);
	// load a texture
	textures.push_back(0);
	textures.push_back(textureLoader::loadFromFile("reflectiveSphere.png"));
	textures.push_back(textureLoader::loadFromFile("cloth.png"));
	screen.texture = 0;
	layout.texture = 1;
	menu.texture = 2;
	screen.subcontexts.push_front(&layout);
	screen.subcontexts.push_back(&menu);
	textures.push_back(screen.texture);
	// last of the init
	initGeometry();
	initShaders();
	GLint texUnits;
	glGetIntegerv(GL_MAX_TEXTURE_UNITS,&texUnits);
	cout << "# of tex units: " << texUnits << endl;
	windowResize(SCW,SCH);
	// Main loop
	int running = GL_TRUE;
	while( running ) {
		// OpenGL rendering goes here...
		glClear( GL_COLOR_BUFFER_BIT );
		
		recursiveRender(&screen);
		// Swap front and back rendering buffers
		glfwSwapBuffers();
		// Check if ESC key was pressed or window was closed
		running = !glfwGetKey( GLFW_KEY_ESC ) && glfwGetWindowParam( GLFW_OPENED );
	}
	textureLoader::denit();
	// delete the texture
	glDeleteTextures(textures.size(), &textures[0]);
	// delete the buffers
	glDeleteVertexArrays(1,&vertexArrayID);
	glDeleteBuffers(3,bufferIDs);
	// Close window and terminate GLFW
	glfwTerminate();
	// Exit program
	exit( EXIT_SUCCESS );
}
