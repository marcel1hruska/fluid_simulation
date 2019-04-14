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
#define GRID_SIZE 100
#define GRAVITY 10

namespace simulation
{
	struct grid_height
	{
		float water;
		float terrain;
	};

	struct vertex_info
	{
		glm::vec2 coords;
		grid_height height;
		float get_total_height();
	};

	typedef std::array<std::array<vertex_info,GRID_SIZE>,GRID_SIZE> vertex_grid;
	typedef std::array<std::array<float, GRID_SIZE>, GRID_SIZE+1> x_velocity_grid;
	typedef std::array<std::array<float, GRID_SIZE+1>, GRID_SIZE> y_velocity_grid;

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
		swe_solver(){ };
		void initialize();
		//recomputes velocity/acceleration and pressure
		void recompute(double delta);
		//add extra water on button click
		void add_water(glm::vec3 pos, glm::vec3 dir);
		//show mass of water
		float water_mass();
		//get vertex info
		vertex_grid & get_vertices();
	private:
		//staggered grid - pressure in the middle of the cell, velocities at the sides
		/*     
			---↑vy-----
			|         |
			|    h    →vx
			|         |
			-----------
		*/
		//grid vertex info
		vertex_grid vertices_;
		//velocities between vertices
		x_velocity_grid velocity_x_;
		y_velocity_grid velocity_y_;
		//time delta
		double delta_;
		//step size
		double step_ = 0.03;
		//
		double interpolate_(double q11, double q21, double q12, double q22, float xc, float yc, float x, float y);
		//SWE Solver Algorithm:
		//	1, advect water height
		//	2, advect x velocity
		//	3, advect y velocity
		//	4, update height
		//	5, update velocities
		//	6, apply reflecting boundary conditions
		// implementation source: https://pdfs.semanticscholar.org/c902/c4f2c61734cbf4ec7ee8b792ccb01644943d.pdf
		//advections
		void adv_height_();
		void adv_x_velocity_();
		void adv_y_velocity_();
		//unknowns updates
		void update_height_();
		void update_velocities_();
		//apply boundary conditions
		void boundary_conditions_();
		//apply reflections - needed for occasional big deltas
		void reflect_(double & x, double & y, int bound_x, int bound_y);
	};
}

#endif SWE_H