
#include "stdafx.hpp"
#include "gl_extensions.h"

#include <GLFW/glfw3.h>

#ifdef __cplusplus
extern "C" {
#endif

PFNGLNAMEDBUFFERPAGECOMMITMENTEXT glNamedBufferPageCommitmentEXT = nullptr;

#ifdef __cplusplus
}
#endif

bool init_glext() {
	glNamedBufferPageCommitmentEXT = reinterpret_cast<PFNGLNAMEDBUFFERPAGECOMMITMENTEXT>(glfwGetProcAddress("glNamedBufferPageCommitmentEXT"));
	
	return true;
}
