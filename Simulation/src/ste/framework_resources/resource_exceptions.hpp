// StE 
// � Shlomi Steinberg, 2015-2016 

#pragma once 

#include <stdexcept> 

namespace StE {
namespace Resource {

class resource_exception : public std::runtime_error {
	using Base = std::runtime_error;

public:
	using Base::Base;
	resource_exception() : Base("") {}
};

class resource_io_error : public resource_exception {
public:
	using resource_exception::resource_exception;
};

}
}