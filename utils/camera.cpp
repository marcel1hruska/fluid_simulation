#include "camera.h"

using namespace utils;

void utils::camera::initialise(GLuint matrix_id, GLFWwindow * window, double * delta_time)
{
	window_ = window;
	delta_time_ = delta_time;
	glfwSetCursorPos(window_, WINDOW_WIDTH / 2, WINDOW_HEIGHT / 2);
	last_x_ = x_ = WINDOW_WIDTH / 2;
	last_y_ = y_ = WINDOW_HEIGHT / 2;
	camera::matrix_id = matrix_id;
}

void camera::reposition()
{
	//add cursor values to the camera front
	add_cursor_values_();
	//add keyboard values to the camera position
	add_keyboard_values_();
	//compute final transform matrix
	view = glm::lookAt(pos_, pos_ + front_, up_);
	projection = glm::perspective(glm::radians(45.0f), WINDOW_WIDTH / (float)WINDOW_HEIGHT, 0.1f, 100.0f);
	transform_matrix = projection * view;
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

void camera::add_keyboard_values_()
{
	//delta time/camera speed
	float speed = KEYBOARD_SENSITIVITY * (*delta_time_);

	if (glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS)
		pos_ += speed * front_;
	if (glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS)
		pos_ -= speed * front_;
	if (glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS)
		pos_ -= glm::normalize(glm::cross(front_, up_)) * speed;
	if (glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS)
		pos_ += glm::normalize(glm::cross(front_, up_)) * speed;
}