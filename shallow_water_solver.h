#ifndef SWE_H
#define SWE_H

#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <array>

#define PLANE_HEIGHT 2
#define GRID_SIZE 100
#define GRAVITY 10

typedef GLfloat grid_array[GRID_SIZE*GRID_SIZE * 3];

namespace simulation
{
	struct grid_height
	{
		double water;
		double terrain;
	};

	struct vertex_info
	{
		glm::vec2 coords;
		grid_height height;
	};

	typedef std::array<std::array<vertex_info,GRID_SIZE>,GRID_SIZE> vertex_grid;
	typedef std::array<std::array<double, GRID_SIZE>, GRID_SIZE+1> x_velocity_grid;
	typedef std::array<std::array<double, GRID_SIZE+1>, GRID_SIZE> y_velocity_grid;

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
		//adjusts water height according to the pressure
		void adjust_grid();
		//recomputes velocity/acceleration and pressure
		void recompute(double delta);
		//add extra water on button click
		void add_water(glm::vec3 pos, glm::vec3 dir);
		std::vector<unsigned int> & indices();
		grid_array & vertex_buffer();
		grid_array & color_buffer();
		//show mass of water
		float water_mass();
	private:
		//initialise vertex value
		void create_vertices_();
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
		//vertex positions buffer
		grid_array vertex_buffer_data_;
		//vertex colors buffer
		grid_array color_buffer_data_;
		//triangles matching vertices
		std::vector<unsigned int> indices_;
		//time delta
		double delta_;
		//step size
		double step_ = 0.1;
		//
		double interpolate_(double q11, double q21, double q12, double q22, float xc, float yc, float x, float y);
		//SWE Solver Algorithm:
		//	1, advect water height
		//	2, advect x velocity
		//	3, advect y velocity
		//	4, update height
		//	5, update velocities
		//	6, if necessary, apply reflecting boundary conditions
		// implementation source: https://pdfs.semanticscholar.org/c902/c4f2c61734cbf4ec7ee8b792ccb01644943d.pdf
		//advections
		void adv_height();
		void adv_x_velocity();
		void adv_y_velocity();
		//unknowns updates
		void update_height();
		void update_velocities();
		//apply boundary conditions
		void boundary_conditions();
		//apply reflections
		void reflect_(double & x, double & y);
	};
}

#endif SWE_H