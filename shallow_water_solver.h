#ifndef SWE_H
#define SWE_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <array>

#define WATER_HEIGHT 0
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

	struct add_info
	{
		glm::vec3 pos;
		float delta;
	};

	/* Shallow Water Equations basic version:

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
	class swe_solver
	{
	public:
		GLuint initialize();
		//recomputes heights and velocities + adds water if necessary
		void recompute(double delta, glm::vec3 pos = glm::vec3(0,0,0), glm::vec3 dir = glm::vec3(0,0,0));
		//returns heights
		height * get_heights();
		//destroy solver
		void destroy();
	private:
		GLuint compute_shader_id_, height_ssbo_id_, vel_ssbo_id_, add_ssbo_id_;
		//keep heights and additional info buffers
		height* heights_;
		add_info * info_;
		//time delta
		double delta_;
	};
}

#endif SWE_H