#include "shallow_water_solver.h"

using namespace simulation;
using namespace glm;

void simulation::swe_solver::initialize()
{
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			//DAM BREAK
			if (x < GRID_SIZE / 4 && y < GRID_SIZE / 4)
				vertices_[x][y].height.water = WATER_HEIGHT + 1;
			else
				vertices_[x][y].height.water = WATER_HEIGHT;
			
			//NORMAL
			//vertices_[x][y].height.water = WATER_HEIGHT;

			//TODO prerendered terrain e.g.
			//TODO not working correctly, buggy, probably tracking of free surface is needed
			/*
			if (x < GRID_SIZE / 4)
				vertices_[x][y].height.terrain = TERRAIN_HEIGHT + 0.2;
			else
				vertices_[x][y].height.terrain = TERRAIN_HEIGHT;
				*/
			// constant 0 terrain height for now
			vertices_[x][y].height.terrain = TERRAIN_HEIGHT;
			// 0 velocities at the beginning
			velocity_x_[x][y] = 0.0;
			velocity_y_[x][y] = 0.0;

			vertices_[x][y].coords = { (GLfloat)(x - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2),(GLfloat)(y - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2) };
		}
	}

	for (size_t x = 0; x < GRID_SIZE; x++)
	{
		velocity_x_[GRID_SIZE][x] = 0.0;
		velocity_y_[x][GRID_SIZE] = 0.0;
	}
}

void simulation::swe_solver::recompute(double delta)
{
	delta_ = delta/2;
	//all advections use semi-lagrangian method
	adv_height_();
	adv_x_velocity_();
	adv_y_velocity_();

	update_height_();
	update_velocities_();

	boundary_conditions_();
}

double simulation::swe_solver::interpolate_(double q11, double q21, double q12, double q22, float xc, float yc, float x, float y)
{
	auto r1 = (xc + 1 - x)*q11 + (x - xc)*q21;
	auto r2 = (xc +1 - x)*q12 + (x - xc)*q22;
	return (yc + 1 - y)*r1 + (y - yc)*r2;
}

void swe_solver::adv_height_() {
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
			reflect_(back_x, back_y, GRID_SIZE - 1, GRID_SIZE - 1);

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

void swe_solver::adv_x_velocity_() {
	auto copy_velocity_x = velocity_x_;
	for (size_t x = 1; x < GRID_SIZE; ++x) {
		for (size_t y = 1; y < GRID_SIZE - 1; ++y) {
			double velocity_x = copy_velocity_x[x][y];
			//average neighbour y velocities
			double velocity_y = (velocity_y_[x][y] + velocity_y_[x][y+1] + velocity_y_[x-1][y] + velocity_y_[x - 1][y + 1]) / 4;

			//more or less same as height advection
			double back_x = (x * step_ - velocity_x * delta_) / step_;
			double back_y = (y * step_ - velocity_y * delta_) / step_;

			reflect_(back_x, back_y, GRID_SIZE, GRID_SIZE - 1);

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

void swe_solver::adv_y_velocity_() {
	auto copy_velocity_y = velocity_y_;
	for (size_t x = 1; x < GRID_SIZE - 1; ++x) {
		for (size_t y = 1; y < GRID_SIZE; ++y) {
			double velocity_x = (velocity_x_[x][y] + velocity_x_[x+1][y] + velocity_x_[x][y-1] + velocity_x_[x + 1][y - 1]) / 4;
			double velocity_y = copy_velocity_y[x][y];

			double back_x = (x * step_ - velocity_x * delta_)/step_;
			double back_y = (y * step_ - velocity_y * delta_)/step_;

			reflect_(back_x, back_y, GRID_SIZE-1, GRID_SIZE);

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

void swe_solver::update_height_() {
	for (size_t i = 1; i < GRID_SIZE - 1; ++i) {
		for (size_t j = 1; j < GRID_SIZE - 1; ++j) {
			//compute divergence
			double divergence = (velocity_x_[i+1][j] - velocity_x_[i][j])/step_ + (velocity_y_[i][j+1] - velocity_y_[i][j])/ step_;
			vertices_[i][j].height.water -= vertices_[i][j].height.water * divergence * delta_;
		}
	}
}

void swe_solver::update_velocities_() {
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

void swe_solver::boundary_conditions_() {
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

void simulation::swe_solver::reflect_(double & x, double & y, int bound_x, int bound_y)
{
	x = (x < 0) ? -x : (x > bound_x) ? (bound_x - (x - bound_x)) : x;
	y = (y < 0) ? -y : (y > bound_y) ? (bound_y - (y - bound_y)) : y;
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
			vec2 intersection(pos.x + dir.x * ((vertices_[x][y].get_total_height() -pos.y)/ dir.y), pos.z + dir.z * ((vertices_[x][y].get_total_height() -pos.y)/ dir.y));
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
		int radius = GRID_SIZE / 10;
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
					vertices_[x][y].height.water += 0.01;
				}
			}
		}
	}
}

float simulation::swe_solver::water_mass()
{
	float mass = 0;
	for (size_t y = 0; y < GRID_SIZE; y++)
	{
		for (size_t x = 0; x < GRID_SIZE; x++)
		{
			mass += vertices_[x][y].height.water;
		}
	}
	return mass;
}

vertex_grid & simulation::swe_solver::get_vertices()
{
	return vertices_;
}

float simulation::vertex_info::get_total_height()
{
	return height.terrain + height.water;
}
