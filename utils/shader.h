#ifndef SHADER_H
#define SHADER_H

#include <string>

namespace utils {
	GLuint process_shaders(const std::string & vertex_shader_path, const std::string & fragment_shader_path);

	GLint read_and_compile(const std::string & shader_path, GLuint id);
}

#endif
