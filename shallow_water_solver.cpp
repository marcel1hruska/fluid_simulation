#include "shallow_water_solver.h"

using namespace simulation;
using namespace glm;

void simulation::swe_solver::initialize()
{
	create_vertices_();
	adjust_grid();
}

void simulation::swe_solver::create_vertices_()
{
	size_t current_vertex;
	// create vertex/color buffers and vertex info array
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			//3 array indices for one vertex - for each coordinate
			current_vertex = 3 * (y * GRID_SIZE + x);

			//initialize to (-1,1)
			vertex_buffer_data_[current_vertex] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			vertex_buffer_data_[current_vertex + 1] = 1;
			vertex_buffer_data_[current_vertex + 2] = (GLfloat)(y - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);

			//initialize to light blue
			color_buffer_data_[current_vertex] = color_buffer_data_[current_vertex + 1] = 0.5;
			color_buffer_data_[current_vertex + 2] = 1.0;

			//TODO start/HUD modes
			/* 
			//DAM BREAK
			if (x < GRID_SIZE / 6 && y < GRID_SIZE / 6)
				vertices_[x][y].height.water = PLANE_HEIGHT + 1;
			else
				vertices_[x][y].height.water = PLANE_HEIGHT;
			*/
			//TSUNAMI
			//NORMAL
			vertices_[x][y].height.water = PLANE_HEIGHT;

			//TODO prerendered terrain e.g.
			/*
			if (x < GRID_SIZE / 6 && y < GRID_SIZE / 6)
				vertices_[x][y].height.terrain = 1;
			else
				vertices_[x][y].height.terrain = 0;
			*/

			// constant 0 terrain height for now
			vertices_[x][y].height.terrain = 0;
			// 0 velocities at the beginning
			velocity_x_[x][y] = 0.0;
			velocity_y_[x][y] = 0.0;

			vertices_[x][y].coords = { vertex_buffer_data_[current_vertex],vertex_buffer_data_[current_vertex + 2] };
		}
	}

	for (size_t x = 0; x < GRID_SIZE; x++)
	{
		velocity_x_[GRID_SIZE][x] = 0.0;
		velocity_y_[x][GRID_SIZE] = 0.0;
	}

	//create 2 triangles for each 4 points
	for (size_t y = 0; y < GRID_SIZE - 1; y++)
	{
		for (size_t x = 0; x < GRID_SIZE - 1; x++)
		{
			//first triangle
			indices_.push_back(y * GRID_SIZE + x);
			indices_.push_back(y * GRID_SIZE + x + 1);
			indices_.push_back((y + 1) * GRID_SIZE + (x + 1));
			//second triangle
			indices_.push_back(y * GRID_SIZE + x);
			indices_.push_back((y + 1)  * GRID_SIZE + x);
			indices_.push_back((y + 1) * GRID_SIZE + (x + 1));
		}
	}
}

void simulation::swe_solver::adjust_grid()
{
	size_t current_vertex;
	//adjust z for pressure for each vertex
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			current_vertex = 3 * (y*GRID_SIZE + x);
			//total vertex height as a result of terrain height + water height
			auto total_height = vertices_[x][y].height.water + vertices_[x][y].height.terrain;
			vertex_buffer_data_[current_vertex + 1] = total_height;
			//color accouring to Z, the higher the water the lighter the color
			color_buffer_data_[current_vertex] = color_buffer_data_[current_vertex + 1] = (vertices_[x][y].height.water + 0.1 - PLANE_HEIGHT) * 5;
		}
	}
}

void simulation::swe_solver::recompute(double delta)
{
	delta_ = delta;

	//all advections use semi-lagrangian method
	adv_height();
	adv_x_velocity();
	adv_y_velocity();

	update_height();
	update_velocities();

	boundary_conditions();
}

double simulation::swe_solver::interpolate_(double q11, double q21, double q12, double q22, float xc, float yc, float x, float y)
{
	auto r1 = (xc + 1 - x)*q11 + (x - xc)*q21;
	auto r2 = (xc +1 - x)*q12 + (x - xc)*q22;
	return (yc + 1 - y)*r1 + (y - yc)*r2;
}

void swe_solver::adv_height() {
	//make a copy
	auto copy_vertices = vertices_;
	for (size_t x = 1; x < GRID_SIZE - 1; ++x) {
		for (size_t y = 1; y < GRID_SIZE - 1; ++y) {
			//average of velocities to find x and y values on staggered grid
			double velocity_x = (velocity_x_[x][y] + velocity_x_[x+1][y]) / 2;
			double velocity_y = (velocity_y_[x][y] + velocity_y_[x][y+1]) / 2;

			//backtrace to the last grid point
			double back_x = (x * step_ - velocity_x * delta_)/step_;
			double back_y = (y * step_ - velocity_y * delta_)/step_;

			//move from boundaries
			reflect_(back_x, back_y);

			//put back on grid
			size_t new_grid_x = back_x;
			size_t new_grid_y = back_y;
			
			//bilinear interpolation of the water heights between found grid point and the vertices
			vertices_[x][y].height.water = interpolate_(
				copy_vertices[new_grid_x][new_grid_y].height.water,
				copy_vertices[new_grid_x + 1][new_grid_y].height.water,
				copy_vertices[new_grid_x][new_grid_y + 1].height.water,
				copy_vertices[new_grid_x + 1][new_grid_y + 1].height.water,
				new_grid_x,
				new_grid_y,
				back_x,
				back_y);
		}
	}
}

void swe_solver::adv_x_velocity() {
	auto copy_velocity_x = velocity_x_;
	for (size_t x = 1; x < GRID_SIZE; ++x) {
		for (size_t y = 1; y < GRID_SIZE - 1; ++y) {
			double velocity_x = copy_velocity_x[x][y];
			//average neighbour y velocities
			double velocity_y = (velocity_y_[x][y] + velocity_y_[x][y+1] + velocity_y_[x-1][y] + velocity_y_[x - 1][y + 1]) / 4;

			//more or less same as height advection
			double back_x = (x * step_ - velocity_x * delta_) / step_;
			double back_y = (y * step_ - velocity_y * delta_) / step_;

			reflect_(back_x, back_y);

			size_t new_grid_x = back_x;
			size_t new_grid_y = back_y;

			velocity_x_[x][y] = interpolate_(
				copy_velocity_x[new_grid_x][new_grid_y],
				copy_velocity_x[new_grid_x + 1][new_grid_y],
				copy_velocity_x[new_grid_x][new_grid_y + 1],
				copy_velocity_x[new_grid_x + 1][new_grid_y + 1],
				new_grid_x,
				new_grid_y,
				back_x,
				back_y);
		}
	}
}

void swe_solver::adv_y_velocity() {
	auto copy_velocity_y = velocity_y_;
	for (size_t x = 1; x < GRID_SIZE - 1; ++x) {
		for (size_t y = 1; y < GRID_SIZE; ++y) {
			double velocity_x = (velocity_x_[x][y] + velocity_x_[x+1][y] + velocity_x_[x][y-1] + velocity_x_[x + 1][y - 1]) / 4;
			double velocity_y = copy_velocity_y[x][y];

			double back_x = (x * step_ - velocity_x * delta_)/step_;
			double back_y = (y * step_ - velocity_y * delta_)/step_;

			reflect_(back_x, back_y);

			size_t new_grid_x = back_x;
			size_t new_grid_y = back_y;

			velocity_y_[x][y] = interpolate_(
				copy_velocity_y[new_grid_x][new_grid_y],
				copy_velocity_y[new_grid_x + 1][new_grid_y],
				copy_velocity_y[new_grid_x][new_grid_y + 1],
				copy_velocity_y[new_grid_x + 1][new_grid_y + 1],
				new_grid_x,
				new_grid_y,
				back_x,
				back_y);
		}
	}
}

void swe_solver::update_height() {
	for (size_t i = 1; i < GRID_SIZE - 1; ++i) {
		for (size_t j = 1; j < GRID_SIZE - 1; ++j) {
			//compute divergence
			double divergence = (velocity_x_[i+1][j] - velocity_x_[i][j])/step_ + (velocity_y_[i][j+1] - velocity_y_[i][j])/ step_;
			vertices_[i][j].height.water -= vertices_[i][j].height.water * divergence * delta_;
		}
	}
}

void swe_solver::update_velocities() {
	for (size_t j = 1; j < GRID_SIZE - 1; ++j) {
		for (size_t i = 1; i < GRID_SIZE - 1; ++i) {
			if (j >= 2)
			{
				double total_height1 = vertices_[i][j - 1].height.terrain + vertices_[i][j - 1].height.water;
				double total_height2 = vertices_[i][j].height.terrain + vertices_[i][j].height.water;
				//add acceleration
				velocity_y_[i][j] += GRAVITY * (total_height1 - total_height2) / step_ * delta_;
			}
			if (i >= 2)
			{
				double total_height1 = vertices_[i - 1][j].height.terrain + vertices_[i - 1][j].height.water;
				double total_height2 = vertices_[i][j].height.terrain + vertices_[i][j].height.water;
				velocity_x_[i][j] += GRAVITY * (total_height1 - total_height2) / step_ * delta_;
			}
		}
	}
}
void swe_solver::boundary_conditions() {
	//reflecting, TODO absorbing
	//left
	for (size_t y = 0; y < GRID_SIZE; ++y) {
		vertices_[0][y].height.water = vertices_[1][y].height.water + vertices_[1][y].height.terrain - vertices_[0][y].height.terrain;
		velocity_x_[1][y] = 0.0;
		velocity_y_[0][y] = 0.0;
	}
	//down
	for (size_t x = 0; x < GRID_SIZE; ++x) {
		vertices_[x][0].height.water = vertices_[x][1].height.water;
		velocity_x_[x][0] = 0.0;
		velocity_y_[x][1] = 0.0;
	}
	//up
	for (size_t x = 0; x < GRID_SIZE; ++x) {
		vertices_[x][GRID_SIZE - 1].height.water = vertices_[x][GRID_SIZE - 2].height.water + vertices_[x][GRID_SIZE - 2].height.terrain - vertices_[x][GRID_SIZE - 1].height.terrain;
		velocity_x_[x][GRID_SIZE - 1] = 0.0;
		velocity_y_[x][GRID_SIZE - 1] = 0.0;
	}
	//right
	for (size_t y = 0; y < GRID_SIZE; ++y) {
		vertices_[GRID_SIZE - 1][y].height.water = vertices_[GRID_SIZE - 2][y].height.water + vertices_[GRID_SIZE - 2][y].height.terrain - vertices_[GRID_SIZE - 1][y].height.terrain;
		velocity_x_[GRID_SIZE - 1][y] = 0.0;
		velocity_y_[GRID_SIZE - 1][y] = 0.0;
	}
}

void simulation::swe_solver::reflect_(double & x, double & y)
{
	x = (x < 0) ? -x : (x > GRID_SIZE) ? GRID_SIZE - (x - GRID_SIZE) : x;
	y = (y < 0) ? -y : (y > GRID_SIZE) ? GRID_SIZE - (y - GRID_SIZE) : y;
}

void simulation::swe_solver::add_water(vec3 pos, vec3 dir)
{
	std::pair<int,int> min_vertex;
	float min_distance = FLT_MAX;
	for (size_t x = 1; x < GRID_SIZE - 1; x++)
	{
		for (size_t y = 1; y < GRID_SIZE - 1; y++)
		{
			//compute intersection of a ray from the centre of the camera with a parallel xz plane
			vec2 intersection(pos.x + dir.x * ((PLANE_HEIGHT-pos.y)/ dir.y), pos.z + dir.z * ((PLANE_HEIGHT -pos.y)/ dir.y));
			//out of bounds check
			if (intersection.x < -1 || intersection.x > 1 || intersection.y > 1 || intersection.y < -1)
				return;
			auto current_distance = distance(intersection, vertices_[x][y].coords);
			//find the closest vertex to the intersection
			if (current_distance < min_distance)
			{
				min_vertex = { x,y };
				min_distance = current_distance;
			}
		}
	}
	if (min_distance < FLT_MAX)
	{
		//radius of add circle
		int radius = 4;
		for (int x = min_vertex.first - radius; x < min_vertex.first+ radius; x++)
		{
			for (int y = min_vertex.second - radius; y < min_vertex.second + radius; y++)
			{
				if (y < 0 || x < 0 || x >= GRID_SIZE || y >= GRID_SIZE)
					continue;
				//add height to all elements depending on the distance from the center
				auto dist = distance(vec2(x,y), vec2(min_vertex.first,min_vertex.second));
				if (dist < radius)
				{
					if (dist < 1)
						dist = 1;
					vertices_[x][y].height.water += 1 / dist;
				}
			}
		}
	}
}

std::vector<unsigned int>& simulation::swe_solver::indices()
{
	return indices_;
}

grid_array & simulation::swe_solver::vertex_buffer()
{
	return vertex_buffer_data_;
}

grid_array & simulation::swe_solver::color_buffer()
{
	return color_buffer_data_;
}

float simulation::swe_solver::water_mass()
{
	float mass = 0;
	for (size_t y = 0; y < GRID_SIZE; y++)
	{
		for (size_t x = 0; x < GRID_SIZE; x++)
		{
			mass += vertices_[x][y].height.water - vertices_[x][y].height.terrain;
		}
	}
	return mass;
}
