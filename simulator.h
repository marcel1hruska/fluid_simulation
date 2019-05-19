#ifndef FLUIDSIM_H
#define FLUIDSIM_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shader.h"
#include "utils/camera.h"
#include "utils/hud.h"
#include "motion.h"

namespace simulation
{
	class simulator
	{
	public:
		void simulate();
	private:
		utils::camera c;
		utils::hud h;
		motion m;
		GLFWwindow * window_;
		//run simulations
		bool was_space_pressed_ = false;
		//delta count
		double last_frame_ = 0.0f;
		//intialise openGL libraries
		bool initialise_openGL_();
		void initialise_buffers_();
		void delete_buffers_();
		//draw water scene
		void draw_water_();
		//draw terrain scene
		void draw_terrain_();
		//buffer IDs
		GLuint terrain_shader_id_, water_shader_id_, mesh_element_id_, water_vertex_id_, terrain_vertex_id_, water_color_id_, terrain_color_id_, array_id_, heights_id_;
		GLuint location_reflection_texture_, location_refraction_texture_;

		size_t mesh_size = PARTICLES * 6 * sizeof(unsigned int);
		void push_indices_(size_t * indices, size_t x, size_t y);
	};
}

#endif