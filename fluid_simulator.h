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
	typedef GLfloat grid_array[GRID_SIZE*GRID_SIZE * 3];
	typedef GLfloat color_array[GRID_SIZE*GRID_SIZE * 4];
	typedef GLfloat bound_array[GRID_SIZE*GRID_SIZE * 3 * 4];

	class fluid_simulator
	{
	public:
		void simulate();
		fluid_simulator() 
		{
			//dynamic buffer allocation saves stack capacity
			water_buffer_data_ = new grid_array;
			terrain_buffer_data_ = new grid_array;
			color_buffer_data_ = new color_array;
			bound_buffer_data_ = new bound_array;
		}
		~fluid_simulator()
		{
			delete[] water_buffer_data_;
			delete[] color_buffer_data_;
			delete[] terrain_buffer_data_;
			delete[] bound_buffer_data_;
		}
	private:
		utils::camera c;
		utils::hud h;
		swe_solver s;
		GLFWwindow * window_;
		//run simulations
		bool run_ = false;
		//delta count
		double delta_time_ = 0.0f;
		double last_frame_ = 0.0f;
		//intialise openGL libraries
		bool initialise_openGL_();
		void initialise_buffers_();
		void delete_buffers_();
		//draw water scene
		void draw_water_();
		//draw terrain scene
		void draw_terrain_();
		//draw 4 boundaries between terrain and water
		void draw_boundaries_();
		//buffer IDs
		GLuint terrain_shader_id_, water_shader_id_, mesh_element_id_, water_vertex_id_, terrain_vertex_id_, color_id_, array_id_, bound_element_id_, bound_vertex_id_;
		GLuint location_reflection_texture_, location_refraction_texture_;
		//4 bounds
		float* bound_buffer_data_;
		//water positions buffer
		float* water_buffer_data_;
		//terrain positions buffer
		float* terrain_buffer_data_;
		//vertex colors buffer
		float* color_buffer_data_;
		//triangles matching water/terrain mesh
		std::vector<unsigned int> mesh_indices_;
		//triangles matching boundaries mesh
		std::vector<unsigned int> bound_indices_;
		void create_vertices_();
		//adjusts water height
		void adjust_grid_();
		GLfloat terrain_color_[4] = { 0.5, 0.4, 0.2, 1.0 };
		GLfloat glass_color_[4] = { 0.6, 0.6, 0.8, 0.4 };
		void push_indices_(std::vector<unsigned int> & indices, size_t move, size_t x, size_t y);
	};
}

#endif