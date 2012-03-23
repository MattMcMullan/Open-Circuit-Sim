#pragma once
#include <Eigen/Dense>
#include <Eigen/Geometry>
#include <vector>
#include <cstddef>
#include <iostream>

// you can use this like both a stack and a pointer to a value_type (Eigen::Affine3f)
template<class t> class evaluationStack {
public:
	typedef t                 value_type;
	typedef value_type &      reference;
	typedef const value_type &const_reference;
	typedef value_type *      pointer;
	typedef const value_type *const_pointer;
	typedef size_t            size_type;
	typedef ptrdiff_t         difference_type;
private:
	std::vector<value_type> data;
public:
	evaluationStack() {
		data.push_back(value_type::Identity());
	}
	evaluationStack(const_reference o) {
		data.push_back(o);
	}
	reference operator=(const_reference o) {
		return (data.back() = o);
	}
	void push() {
		data.push_back(data.back());
	}
	void pop() {
		if (data.size()>1) {
			data.pop_back();
		}
	}
	reference top() {
		return data.back();
	}
	const_reference top() const {
		return data.back();
	}
	reference peek() {
		return *(++data.rbegin());
	}
	const_reference peek() const {
		return *(++data.rbegin());
	}
	reference operator*() {
		return data.back();
	}
	const_reference operator*() const {
		return data.back();
	}
	pointer operator->() {
		return &data.back();
	}
	const_pointer operator->() const {
		return &data.back();
	}
	size_type size() const {
		return data.size()-1;
	}
	bool empty() const {
		return (data.size()-1)==0;
	}
	void clear() {
		data.clear();
		data.push_back(value_type::Identity());
	}
};

typedef evaluationStack<Eigen::Affine3f> matrixStack;
typedef evaluationStack<Eigen::Vector4f> vectorStack;
