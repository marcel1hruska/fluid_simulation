#ifndef HUD_H
#define HUD_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>


namespace utils
{
	class hud
	{
	public:
		void draw_crosshair();
		void initialize();
		void destroy();
	private:
		GLuint buff_id_;
		GLuint shader_id_;
		//triangle in the middle
		const GLfloat vertex_values_[9] = 
		                   {-0.02f, -0.02f, 0.0f,
							0.02f, -0.02f, 0.0f,
							0.0f,  0.02f, 0.0f};
	};
}

#endif