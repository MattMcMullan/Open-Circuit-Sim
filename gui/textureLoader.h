#ifndef __textureLoader_h__
#define __textureLoader_h__
#include "libs.h"

class textureLoader {
public:
	static void init();
	static void denit();
	static GLuint loadFromFile(const char *fname);
};

#endif
