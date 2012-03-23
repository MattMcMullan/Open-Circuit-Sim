#include "libs.h"
#include "textureLoader.h"
#include <iostream>
using std::cout; using std::endl;
// helper functions to init and denit OpenIL
bool DevIL_Initialized = false;
void DevIL_Init() {
	if (DevIL_Initialized)
		return;
	// init DevIL
	if (ilGetInteger(IL_VERSION_NUM) < IL_VERSION ||
		iluGetInteger(ILU_VERSION_NUM) < ILU_VERSION ||
		ilutGetInteger(ILUT_VERSION_NUM) < ILUT_VERSION) {
		cout << "DevIL library is out of date! Please upgrade"<<endl;
		return;
	}
	DevIL_Initialized = true;
	// Needed to initialize DevIL.
	ilInit ();
	iluInit();
	// GL cannot use palettes anyway, so convert early.
	ilEnable (IL_CONV_PAL);
	// Gets rid of dithering on some nVidia-based cards.
	ilutEnable (ILUT_OPENGL_CONV);
}
void DevIL_Denit() {
	if (!DevIL_Initialized)
		return;
	DevIL_Initialized = false;
	ilShutDown();
}
void HandleDevILErrors () {
	ILenum error = ilGetError ();
	if (error != IL_NO_ERROR) {
		do {
			cout << "\n\n" << iluErrorString (error) << endl;	
		} while ((error = ilGetError ()));
	}
}

void textureLoader::init() {
	DevIL_Init();
}
void textureLoader::denit() {
	DevIL_Denit();
}
GLuint textureLoader::loadFromFile(const char *fname) {
	GLuint retval;
	// IL image ID
	ILuint ImgId = 0;
	
	// Generate the main image name to use.
	ilGenImages (1, &ImgId);

	// Bind this image name.
	ilBindImage (ImgId);

	// Loads the image specified by File into the ImgId image.
	if (!ilLoadImage (fname)) {
		HandleDevILErrors ();
		return 0;
	}
	// Lets ILUT know to use its OpenGL functions.
	ilutRenderer (ILUT_OPENGL);
	// Goes through all steps of sending the image to OpenGL.
	retval = ilutGLBindTexImage();
	// We're done with our image, so we go ahead and delete it.
	ilDeleteImages(1, &ImgId);
	
	return retval;
}
