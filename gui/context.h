#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <set>
#include <list>
#include "libs.h"

class context {
public:
	std::list<context*> subcontexts;
	Eigen::Affine3f transformation;
	GLuint texture;
	context() {
		transformation = Eigen::Affine3f::Identity();
		texture = 0;
	}
};
