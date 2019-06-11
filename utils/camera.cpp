#include "camera.h"

using namespace utils;

void utils::camera::initialise(GLuint water_shader_id, GLuint terrain_shader_id, GLFWwindow * window, int height)
{
	window_ = window;
	glfwSetCursorPos(window_, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
	last_x_ = x_ = WINDOW_WIDTH / 2;
	last_y_ = y_ = WINDOW_HEIGHT / 2;
	glUseProgram(terrain_shader_id);
	terrain_matrix_id = glGetUniformLocation(terrain_shader_id, "transform_matrix");
	glUseProgram(water_shader_id);
	water_matrix_id = glGetUniformLocation(water_shader_id, "transform_matrix");
	pos_ = glm::vec3(-1.0f, height + 3.0f, 3.0f);
}

void camera::reposition(double delta)
{
	//add cursor values to the camera front
	add_cursor_values_();
	//add keyboard values to the camera position
	add_keyboard_values_(delta);
	//compute final transform matrix
	view_ = glm::lookAt(pos_, pos_ + front_, up_);
	projection_ = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
	transform_matrix = projection_ * view_;
}

glm::vec3 utils::camera::get_pos()
{
	return pos_;
}

glm::vec3 utils::camera::get_dir()
{
	return front_;
}

void camera::add_cursor_values_()
{
	glfwGetCursorPos(window_, &x_, &y_);

	yaw_ += (x_ - last_x_) * MOUSE_SENSITIVITY;
	pitch_ += (last_y_ - y_) * MOUSE_SENSITIVITY;

	last_x_ = x_;
	last_y_ = y_;

	//maximum pitch values
	if (pitch_ > 85.0f)
		pitch_ = 85.0f;
	else if (pitch_ < -85.0f)
		pitch_ = -85.0f;

	//compute new front
	front_ = glm::normalize(glm::vec3(
		cos(glm::radians(yaw_)) * cos(glm::radians(pitch_)), 
		sin(glm::radians(pitch_)), 
		sin(glm::radians(yaw_)) * cos(glm::radians(pitch_)) ));
}

void camera::add_keyboard_values_(double delta)
{
	//delta time/camera speed
	float speed = KEYBOARD_SENSITIVITY * delta;

	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		pos_ += speed * front_;
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		pos_ -= speed * front_;
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		pos_ -= glm::normalize(glm::cross(front_, up_)) * speed;
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		pos_ += glm::normalize(glm::cross(front_, up_)) * speed;
}