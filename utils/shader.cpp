#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <algorithm>
#include <sstream>
#include <string.h>

#include <GL/glew.h>

#include "shader.h"

using namespace std;

namespace utils {
	GLuint process_shaders(const std::string & vertex_shader_path, const std::string & fragment_shader_path)
	{
		// initialize
		GLuint vertex_id = glCreateShader(GL_VERTEX_SHADER);
		GLuint fragment_id = glCreateShader(GL_FRAGMENT_SHADER);

		if (read_and_compile(vertex_shader_path, vertex_id) == GL_FALSE)
			return 0;
		if (read_and_compile(fragment_shader_path, fragment_id) == GL_FALSE)
			return 0;

		// link
		GLuint program_id = glCreateProgram();
		glAttachShader(program_id, vertex_id);
		glAttachShader(program_id, fragment_id);
		glLinkProgram(program_id);

		int length;
		// Check the program
		glGetProgramiv(program_id, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char * info = new char[length + 1];
			glGetShaderInfoLog(program_id, length, NULL, info);
			cout << "Link info: " << info << endl;
			delete[] info;
		}

		//remove
		glDetachShader(program_id, vertex_id);
		glDetachShader(program_id, fragment_id);

		glDeleteShader(vertex_id);
		glDeleteShader(fragment_id);

		return program_id;
	}

	GLint utils::read_and_compile(const std::string & shader_path, GLuint id)
	{
		stringstream code;

		// read vertex
		ifstream file(shader_path);
		if (file.is_open())
		{
			code << file.rdbuf();
			file.close();
		}
		else
		{
			cout << "Wrong shader path" << endl;
			return 0;
		}

		// compile vertex
		auto source = code.str();
		auto c_source = source.c_str();
		glShaderSource(id, 1, &c_source, NULL);
		glCompileShader(id);

		int length;
		// compilation results
		glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
		if (length > 0) {
			char * info = new char[length + 1];
			glGetShaderInfoLog(id, length, NULL, info);
			cout << shader_path << " compile info: " << info << endl;
			delete[] info;
		}

		GLint result;
		glGetShaderiv(id, GL_COMPILE_STATUS, &result);

		return result;
	}
}