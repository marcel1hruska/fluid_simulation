#ifndef CAMERA_H
#define CAMERA_H

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include <utility>

#define MOUSE_SENSITIVITY 0.05f
#define KEYBOARD_SENSITIVITY 4.0f

namespace utils {
	class camera
	{
	public:
		camera(GLuint matrix_id, GLFWwindow* window) : window_(window), matrix_id(matrix_id)
		{
			glfwGetWindowSize(window_, &width_, &height_);
			glfwSetCursorPos(window_, width_ / 2, height_ / 2);
			last_x_ = x_ = width_ / 2;
			last_y_ = y_ = height_ / 2;
		};
		void reposition();
		GLuint matrix_id;
		glm::mat4 transform_matrix;
	private:
		//mouse variables
		double last_x_, last_y_, x_, y_;
		float yaw_ = -90.0f;
		float pitch_ = -30.0f;
		//keyboard variables
		float delta_time_ = 0.0f;
		float last_frame_ = 0.0f;
		//camera variables
		glm::vec3 pos_ = glm::vec3(0.0f, 3.0f, 5.0f);
		glm::vec3 front_ = glm::vec3(0.0f, 0.0f, 1.0f);
		const glm::vec3 up_ = glm::vec3(0.0f, 1.0f, 0.0f);
		//window variables
		int width_, height_;
		GLFWwindow* window_;

		void add_cursor_values_();
		void add_keyboard_values_();
	};
}
#endif