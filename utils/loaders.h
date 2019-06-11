#ifndef LOADERS_H
#define LOADERS_H

#include <string>

namespace utils {
	GLuint process_shaders(const std::string & vertex_shader_path, const std::string & fragment_shader_path);
	GLuint process_compute_shader(const std::string & compute_shader_path);
	GLint read_and_compile(const std::string & shader_path, GLuint id);
	// load textures from BMP
	GLuint loadBMP(char* path);
}

#endif
