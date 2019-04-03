#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include <utility>
#include <map>

#define MOUSE_SENSITIVITY 0.05f
#define KEYBOARD_SENSITIVITY 4.0f

#define WINDOW_WIDTH 1024
#define WINDOW_HEIGHT 768

namespace utils {
	class camera
	{
	public:
		camera() {}
		void initialise(GLuint matrix_id, GLFWwindow* window, double * delta_time);
		void reposition();
		GLuint matrix_id;
		glm::mat4 transform_matrix;
		glm::mat4 view;
		glm::mat4 projection;
		glm::vec3 pos_ = glm::vec3(0.0f, 3.0f, 5.0f);
		glm::vec3 front_ = glm::vec3(0.0f, 0.0f, 1.0f);
	private:
		//mouse variables
		double last_x_, last_y_, x_, y_;
		float yaw_ = -90.0f;
		float pitch_ = -30.0f;
		//keyboard variables
		double * delta_time_;
		//camera variables
		const glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
		//window variables
		GLFWwindow* window_;
		void add_cursor_values_();
		void add_keyboard_values_();
	};
}
#endif