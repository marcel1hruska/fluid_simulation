#ifndef FLUIDSIM_H
#define FLUIDSIM_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/loaders.h"
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
		// other entities
		utils::camera c;
		utils::hud h;
		motion m;
		GLFWwindow * window_;
		// ligh position
		glm::vec3 light_position_ = { 1.0, 2.0, 1.0 };
		//run simulations
		bool was_space_pressed_ = false;
		//delta count
		double last_frame_ = 0.0f;
		//buffer IDs
		GLuint terrain_shader_id_, water_shader_id_, mesh_element_id_, water_vertex_id_, terrain_vertex_id_, array_id_;
		//GLuint reflection_texture_id_, refraction_texture_id_, reflection_depth_buffer_id_, refraction_depth_texture_id_, reflection_frame_buffer_id_, refraction_frame_buffer_id_, location_reflection_texture_, location_refraction_texture_;
		// terrain textures
		GLuint sand_texture_, rocks_texture_, grass_texture_, forest_texture_;
		size_t mesh_size = PARTICLES * 6 * sizeof(unsigned int);

		//intialise openGL libraries
		bool initialise_openGL_();
		bool initialise_buffers_();
		void delete_buffers_();
		//draw water scene
		void draw_water_();
		//draw terrain scene
		void draw_terrain_();
		void render_scene_();
		void push_indices_(size_t * indices, size_t x, size_t y);

	};
}

#endif