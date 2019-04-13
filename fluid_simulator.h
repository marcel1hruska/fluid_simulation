#ifndef FLUIDSIM_H
#define FLUIDSIM_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/hud.h"
#include "shallow_water_solver.h"

namespace simulation
{
	class fluid_simulator
	{
	public:
		void simulate();
	private:
		utils::camera c;
		utils::hud h;
		swe_solver s;
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
	};
}

#endif