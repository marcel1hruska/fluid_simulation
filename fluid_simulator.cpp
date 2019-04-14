#include "fluid_simulator.h"
#include <iostream>

using namespace simulation;
using namespace glm;
using namespace std;


void fluid_simulator::simulate()
{
	if (!initialise_openGL_())
		return;

	s.initialize();

	create_vertices_();
	adjust_grid_();

	initialise_buffers_();

	//TODO water textures
	//location_reflection_texture_ = glGetUniformLocation(shader_id_,"reflection_texture");
	//location_refraction_texture_ = glGetUniformLocation(shader_id_, "refraction_texture");

	c.initialise(glGetUniformLocation(water_shader_id_, "transform_matrix"), window_, WATER_HEIGHT + TERRAIN_HEIGHT);

	h.init_HUD();

	last_frame_ = glfwGetTime() - 0.01;

	//end on escape
	while (!glfwWindowShouldClose(window_) && glfwGetKey(window_,GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		// clear colors and depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		auto current_frame = glfwGetTime();
		delta_time_ = current_frame - last_frame_;
		last_frame_ = current_frame;

		c.reposition(delta_time_);

		//recompute advection and update unknowns
		s.recompute(delta_time_);

		// Compute height of each vertex
		adjust_grid_();
		
		//draw meshes
		draw_terrain_();

		draw_water_();

		draw_boundaries_();

		//check for right mouse button -> add water to the middle pressed
		if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			h.draw_crosshair();
			s.add_water(c.get_pos(), c.get_dir());
		}
		
		glfwSwapBuffers(window_);
		glfwPollEvents();
	}

	delete_buffers_();

	h.delete_HUD();

	glfwTerminate();
}

bool simulation::fluid_simulator::initialise_openGL_()
{
	// initialize GLFW
	if (!glfwInit())
	{
		cerr << "Cannot initialize GLFW" << endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// create window
	window_ = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Fluid Simulation", NULL, NULL);
	if (window_ == NULL) {
		cerr << "Cannot open GLFW window" << endl;
		glfwTerminate();
		return false;
	}

	// create context
	glfwMakeContextCurrent(window_);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Cannot initialize GLEW" << endl;
		glfwTerminate();
		return false;
	}

	// FPS mode	
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	// depth enabled
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	return true;
}

void simulation::fluid_simulator::initialise_buffers_()
{
	//create vertex array
	glGenVertexArrays(1, &array_id_);
	glBindVertexArray(array_id_);

	water_shader_id_ = utils::process_shaders("shaders/water_vertex_shader.vs", "shaders/water_fragment_shader.fs");
	terrain_shader_id_ = utils::process_shaders("shaders/terrain_vertex_shader.vs", "shaders/terrain_fragment_shader.fs");

	//create water vertex buffer
	glGenBuffers(1, &water_vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, water_vertex_id_);

	//create terrain vertex buffer
	glGenBuffers(1, &terrain_vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_array), terrain_buffer_data_, GL_STATIC_DRAW);

	//create color buffer
	glGenBuffers(1, &color_id_);
	glBindBuffer(GL_ARRAY_BUFFER, color_id_);

	//create boundary vertex buffer
	glGenBuffers(1, &bound_vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, bound_vertex_id_);

	//buffer for the water//terrain indices
	glGenBuffers(1, &mesh_element_id_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_indices_.size() * sizeof(unsigned int), &mesh_indices_[0], GL_STATIC_DRAW);

	//buffer for the boundary indices
	glGenBuffers(1, &bound_element_id_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound_element_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, bound_indices_.size() * sizeof(unsigned int), &bound_indices_[0], GL_STATIC_DRAW);
}

void simulation::fluid_simulator::delete_buffers_()
{
	glDeleteBuffers(1, &water_vertex_id_);
	glDeleteBuffers(1, &terrain_vertex_id_);
	glDeleteBuffers(1, &bound_vertex_id_);
	glDeleteBuffers(1, &color_id_);
	glDeleteBuffers(1, &mesh_element_id_);
	glDeleteBuffers(1, &bound_element_id_);
	glDeleteProgram(water_shader_id_);
	glDeleteProgram(terrain_shader_id_);
	glDeleteVertexArrays(1, &array_id_);
}

void simulation::fluid_simulator::draw_water_()
{
	glUseProgram(water_shader_id_);

	// send transformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, water_vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(grid_array), water_buffer_data_, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_array), color_buffer_data_, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_indices_.size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void simulation::fluid_simulator::draw_terrain_()
{
	auto color_id = glGetUniformLocation(terrain_shader_id_, "color");

	glUseProgram(terrain_shader_id_);
	// send transformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	//send color to the shader
	glUniform4fv(color_id, 1, terrain_color_);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_indices_.size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
}

void simulation::fluid_simulator::draw_boundaries_()
{
	auto color_id = glGetUniformLocation(terrain_shader_id_, "color");

	glUseProgram(terrain_shader_id_);
	// send transformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	//send color to the shader
	glUniform4fv(color_id, 1, glass_color_);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, bound_vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(bound_array), bound_buffer_data_, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, bound_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, bound_indices_.size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
}

void simulation::fluid_simulator::create_vertices_()
{
	size_t current_vertex;
	size_t current_color;
	size_t next_bound_indices = GRID_SIZE * GRID_SIZE;
	size_t next_bound_buffer = next_bound_indices * 3;
	auto vertices = s.get_vertices();

	// create vertex/color buffers and vertex info array
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			current_vertex = 3 * (y * GRID_SIZE + x);
			current_color = 4 * (y * GRID_SIZE + x);

			//initialize boundary front
			bound_buffer_data_[current_vertex] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			bound_buffer_data_[current_vertex + 1] = 0;
			bound_buffer_data_[current_vertex + 2] = 1 - 2 / (GLfloat)GRID_SIZE;

			//initialize boundary back
			bound_buffer_data_[next_bound_buffer + current_vertex] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			bound_buffer_data_[next_bound_buffer + current_vertex + 1] = 0;
			bound_buffer_data_[next_bound_buffer + current_vertex + 2] = -1;

			//initialize boundary right
			bound_buffer_data_[2 * next_bound_buffer + current_vertex] = 1 - 2/(GLfloat)GRID_SIZE;
			bound_buffer_data_[2 * next_bound_buffer + current_vertex + 1] = 0;
			bound_buffer_data_[2*next_bound_buffer + current_vertex + 2] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);

			//initialize boundary left
			bound_buffer_data_[3 * next_bound_buffer + current_vertex] = -1;
			bound_buffer_data_[3 * next_bound_buffer + current_vertex + 1] = 0;
			bound_buffer_data_[3* next_bound_buffer + current_vertex + 2] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);

			//initialize water to (-1,1)
			water_buffer_data_[current_vertex] = (GLfloat)(x - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2);
			water_buffer_data_[current_vertex + 1] = vertices[x][y].height.water;
			water_buffer_data_[current_vertex + 2] = (GLfloat)(y - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2);

			//initialize terrain to (-1,1)
			terrain_buffer_data_[current_vertex] = (GLfloat)(x - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2);
			terrain_buffer_data_[current_vertex + 1] = vertices[x][y].height.terrain;
			terrain_buffer_data_[current_vertex + 2] = (GLfloat)(y - GRID_SIZE/2) / (GLfloat)(GRID_SIZE / 2);

			//initialize to light blue
			color_buffer_data_[current_color] = color_buffer_data_[current_color+ 1] = 0.5;
			color_buffer_data_[current_color + 2] = 1.0;
			//half transparent
			color_buffer_data_[current_color + 3] = 0.5;

			//create 2 triangles for each 4 points
			if (y < GRID_SIZE - 1 && x < GRID_SIZE - 1)
			{
				//create triangles for boundaries
				push_indices_(bound_indices_, 0, x, y);
				push_indices_(bound_indices_, next_bound_indices, x, y);
				push_indices_(bound_indices_, 2 * next_bound_indices, x, y);
				push_indices_(bound_indices_, 3 * next_bound_indices, x, y);

				//create triangles for terrain/water
				push_indices_(mesh_indices_, 0, x, y);
			}
		}
	}
}

void simulation::fluid_simulator::adjust_grid_()
{
	size_t current_vertex;
	size_t current_color;
	auto vertices = s.get_vertices();

	//adjust z for pressure for each vertex
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			current_vertex = 3 * (y*GRID_SIZE + x);
			current_color = 4 * (y*GRID_SIZE + x);
			//total vertex height as a result of terrain height + water height
			water_buffer_data_[current_vertex + 1] = vertices[x][y].get_total_height();
			//color accouring to Z, the higher the water the lighter the color
			color_buffer_data_[current_color] = color_buffer_data_[current_color + 1] = (vertices[x][y].height.water) * 5;

			//update boundary heights
			bound_buffer_data_[current_vertex + 1] = y*(vertices[x][GRID_SIZE - 1].height.water / (GLfloat)GRID_SIZE) + vertices[x][GRID_SIZE - 1].height.terrain;
			bound_buffer_data_[GRID_SIZE*GRID_SIZE * 3 + current_vertex + 1] = y * (vertices[x][0].height.water / (GLfloat)GRID_SIZE) + vertices[x][0].height.terrain;
			bound_buffer_data_[2 * GRID_SIZE*GRID_SIZE * 3 + current_vertex + 1] = y * (vertices[GRID_SIZE - 1][x].height.water / (GLfloat)GRID_SIZE) + vertices[GRID_SIZE - 1][x].height.terrain;
			bound_buffer_data_[3*GRID_SIZE*GRID_SIZE * 3 + current_vertex + 1] = y * (vertices[0][x].height.water / (GLfloat)GRID_SIZE) + vertices[0][x].height.terrain;
		}
	}
}

void simulation::fluid_simulator::push_indices_(std::vector<unsigned int>& indices, size_t move, size_t x, size_t y)
{
	//first triangle terrain
	indices.push_back(move + y * GRID_SIZE + x);
	indices.push_back(move + y * GRID_SIZE + x + 1);
	indices.push_back(move + (y + 1) * GRID_SIZE + (x + 1));
	//second triangle terrain
	indices.push_back(move + y * GRID_SIZE + x);
	indices.push_back(move + (y + 1)  * GRID_SIZE + x);
	indices.push_back(move + (y + 1) * GRID_SIZE + (x + 1));
}
