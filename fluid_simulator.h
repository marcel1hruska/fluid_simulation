#ifndef FLUIDSIM_H
#define FLUIDSIM_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/hud.h"
#include <vector>

#define SPEED 10.0
#define PI 	3.14159265358979323846

#define GRID_SIZE 50

namespace simulation
{
	struct vertex_info
	{
		double pressure;
		glm::vec2 velocity;
		glm::vec2 coords;
	};

	class fluid_simulator
	{
	public:
		void simulate();
	private:
		utils::camera c;
		utils::hud h;
		GLFWwindow * window_;
		//delta count
		double delta_time_ = 0.0f;
		double last_frame_ = 0.0f;
		bool right_button_pressed_ = false;
		//intialise openGL libraries
		bool initialise_openGL_();
		void initialise_buffers_();
		void delete_buffers_();
		//draw scene
		void draw_();
		//buffer IDs
		GLuint shader_id_, element_id_, vertex_id_, color_id_, array_id_;
		//initialise vertex value
		void create_vertices_();
		//adjusts water height according to the pressure
		void adjust_grid_();
		//recomputes velocity/acceleration and pressure
		void recompute_grid_();
		//all vertex info
		vertex_info vertices_[GRID_SIZE][GRID_SIZE];
		//vertex positions buffer
		GLfloat vertex_buffer_data_[GRID_SIZE*GRID_SIZE * 3];
		//vertex colors buffer
		GLfloat color_buffer_data_[GRID_SIZE*GRID_SIZE * 3];
		//triangles matching vertices
		std::vector<unsigned int> indices_;
		//add extra pressure on button click
		void add_water_pressure_();
		//transforms camera coordinate to world
		glm::vec3 screen_to_world_();
	};
}
#endif