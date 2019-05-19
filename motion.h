#ifndef SWE_H
#define SWE_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <array>
#include "utils/hud.h"
#include "utils/perlin.h"

#define WATER_HEIGHT 0.5
#define TERRAIN_HEIGHT 0.5
#define GRID_SIZE 1024
#define PARTICLES GRID_SIZE*GRID_SIZE

namespace simulation
{
	struct height
	{
		float water;
		float terrain;
	};

	struct velocity
	{
		float x;
		float y;
	};

	struct flux
	{
		float left;
		float right;
		float top;
		float bottom;
	};

	struct add_info
	{
		glm::vec3 pos;
		float delta;
	};

	/*
		IMPORTANT
		!!!
		Currently implemented but not using
		Simple pipeline model proved to be less time-consuming, more stable and provide sufficient visual results
		To use the SWE, uncomment parts starting with SWE and comment out FLOW
		!!!

		Water Simulation, virtual pipe model:

		2 passes (2 compute shaders)
		1. compute outflows for each cell in grid
		2. use these outflows to compute new water heights


		NOT USED
		Shallow Water Equations basic version:

		∂η/∂t+(∇η)v = −η∇v
		∂v/∂t+(∇v)v = a∇h

		where:
			- h - total height (of water above zero)
			- g - terrain height (above zero)
			- η - water height (above terrain, h-g)
			- v - horizontal velocity
			- a - acceleration of fluid due to gravity
		solver computer velocity and height of water over time:

		∂η/∂t +(∇η)v = −η∇v
		∂v1/∂t +(∇v1)v = a∇h
		∂v2/∂t +(∇v2)v = a∇h

		left side of equations - advection within velocity field
		right side - additional acceleration/gravity
		source: https://pdfs.semanticscholar.org/c902/c4f2c61734cbf4ec7ee8b792ccb01644943d.pdf
	*/
	class motion
	{
	public:
		motion() 
		{ 
			utils::perlin_noise p;
			perlin_noise_ = new float[PARTICLES];
			for (size_t y = 0; y < GRID_SIZE; y++)
			{
				for (int x = 0; x < GRID_SIZE; x++)
				{
					auto current = coords(x, y);
					perlin_noise_[x + y * GRID_SIZE] = p.make_some_noise(8 * (current.x + 1) / 2, 8 * (current.y + 1) / 2, TERRAIN_HEIGHT, 5);
				}
			}
		}
		~motion() { delete[] perlin_noise_; }
		GLuint initialize(utils::settings * s);
		//recomputes heights + adds water if necessary
		void recompute(glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 dir = glm::vec3(0,0,0));
		//returns heights
		height * get_heights();
		GLuint flux_id();
		GLuint heights_id();
		//destroy solver
		void destroy();
		void initialize_heights();
	private:
		GLuint compute_shader_id_, height_ssbo_id_, vel_ssbo_id_, flux_ssbo_id_, add_ssbo_id_, flow_shader_id_, update_shader_id_;
		//keep heights and additional info buffers
		height* heights_;
		add_info * info_;
		flux * fl_;
		//time delta
		double delta_;
		glm::vec2 coords(int x, int y);
		utils::settings * s_;
		float * perlin_noise_;
	};
}

#endif SWE_H