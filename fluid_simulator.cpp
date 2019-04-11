#include "fluid_simulator.h"
#include <iostream>


using namespace simulation;
using namespace glm;
using namespace std;


void fluid_simulator::simulate()
{
	if (!initialise_openGL_())
		return;

	create_vertices_();

	adjust_grid_();

	initialise_buffers_();

	c.initialise(glGetUniformLocation(shader_id_,"transform_matrix"), window_, &delta_time_);

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

		c.reposition();

		glUseProgram(shader_id_);

		double delta;
		//check for too big deltas
		while (delta_time_ > 0.f)
		{
			delta = delta_time_ > 0.01 ? 0.01 : delta_time_;
			delta_time_ -= delta;

			recompute_grid_();
		}

		// Compute height of each vertex
		adjust_grid_();

		draw_();
		
		//check for right mouse button -> add pressure to water on its position when pressed
		if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS && !right_button_pressed_)
		{
			right_button_pressed_ = true;
		}
		else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
		{
			h.draw_crosshair();
		}
		else if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_RELEASE && right_button_pressed_)
		{
			add_water_pressure_();
			right_button_pressed_ = false;
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
	
	return true;
}

void simulation::fluid_simulator::initialise_buffers_()
{
	//create vertex array
	glGenVertexArrays(1, &array_id_);
	glBindVertexArray(array_id_);

	shader_id_ = utils::process_shaders("shaders/standard_vertex_shader.vs", "shaders/standard_fragment_shader.fs");

	//create vertex buffer
	glGenBuffers(1, &vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);

	//create color buffer
	glGenBuffers(1, &color_id_);
	glBindBuffer(GL_ARRAY_BUFFER, color_id_);

	// a buffer for the indices
	glGenBuffers(1, &element_id_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices_.size() * sizeof(unsigned int), &indices_[0], GL_STATIC_DRAW);
}

void simulation::fluid_simulator::delete_buffers_()
{
	glDeleteBuffers(1, &vertex_id_);
	glDeleteBuffers(1, &color_id_);
	glDeleteBuffers(1, &element_id_);
	glDeleteProgram(shader_id_);
	glDeleteVertexArrays(1, &array_id_);
}

void simulation::fluid_simulator::draw_()
{
	// send tranformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data_), vertex_buffer_data_, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data_), color_buffer_data_, GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, indices_.size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}

void simulation::fluid_simulator::create_vertices_()
{
	size_t current_vertex;
	// create vertex/color buffers and vertex info array
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			//3 array indices for one vertex - for each coordinate
			current_vertex = 3 * (y * GRID_SIZE + x);

			//initialize to -1 - 1
			vertex_buffer_data_[current_vertex] = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			vertex_buffer_data_[current_vertex + 1] = 0;
			vertex_buffer_data_[current_vertex + 2] = (GLfloat)(y - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);

			//initialize to light blue
			color_buffer_data_[current_vertex] = color_buffer_data_[current_vertex + 1] = 0.5;
			color_buffer_data_[current_vertex + 2] = 1.0;

			//no pressure and velocity at the beginning
			vertices_[x][y].pressure = 0.0;
			vertices_[x][y].velocity = { 0.0,0.0 };
			vertices_[x][y].coords = { vertex_buffer_data_[current_vertex],vertex_buffer_data_[current_vertex + 2] };
		}
	}

	//create 2 triangles for each 4 points
	for (size_t y = 0; y < GRID_SIZE - 1; y++)
	{
		for (size_t x = 0; x < GRID_SIZE - 1; x++)
		{
			//first triangle
			indices_.push_back(y * GRID_SIZE + x);
			indices_.push_back(y * GRID_SIZE + x + 1);
			indices_.push_back((y + 1) * GRID_SIZE + (x+1));
			//second triangle
			indices_.push_back(y * GRID_SIZE + x);
			indices_.push_back((y + 1)  * GRID_SIZE + x);
			indices_.push_back((y + 1) * GRID_SIZE + (x + 1));
		}
	}
}

void simulation::fluid_simulator::adjust_grid_()
{
	size_t current_vertex;
	//adjust z for pressure for each vertex
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			current_vertex = 3 * (y*GRID_SIZE + x);
			//
			vertex_buffer_data_[current_vertex + 1] = (float)(vertices_[x][y].pressure/50);
			//color accouring to Z, the higher the water the lighter the color
			color_buffer_data_[current_vertex] = color_buffer_data_[current_vertex + 1] = (vertex_buffer_data_[current_vertex + 1] + 0.1)*5;
		}
	}
}

void simulation::fluid_simulator::recompute_grid_()
{
	double step = delta_time_ * SPEED;

	// recompute velocity
	for (int x = 0; x < GRID_SIZE; x++)
	{
		int next = (x + 1) % GRID_SIZE;
		for (int y = 0; y < GRID_SIZE; y++)
		{
			//velocity is difference of pressures
			vertices_[x][y].velocity.x += (vertices_[x][y].pressure - vertices_[next][y].pressure) * step;
			vertices_[y][x].velocity.y += (vertices_[y][x].pressure - vertices_[y][next].pressure) * step;
		}
	}

	// new pressure from velocity
	for (int x = 1; x < GRID_SIZE; x++)
	{
		for (int y = 1; y < GRID_SIZE; y++)
		{
			vertices_[x][y].pressure += (vertices_[x - 1][y].velocity.x - vertices_[x][y].velocity.x + vertices_[x][y - 1].velocity.y - vertices_[x][y].velocity.y) * step;
		}
	}
}

void simulation::fluid_simulator::add_water_pressure_()
{
	vertex_info * min_vertex;
	float min_distance = 100;
	for (size_t x = 1; x < GRID_SIZE-1; x++)
	{
		for (size_t y = 1; y < GRID_SIZE-1; y++)
		{
			//compute intersection of a ray from the centre of the camera with a parallel xz plane
			vec2 intersection(c.pos_.x + c.front_.x * (-c.pos_.y / c.front_.y), c.pos_.z + c.front_.z * (-c.pos_.y / c.front_.y));
			//out of bounds check
			if (intersection.x < -1 || intersection.x > 1 || intersection.y > 1 || intersection.y < -1)
				return;
			auto current_distance = distance(intersection, vertices_[x][y].coords);
			//find the closest vertex to the intersection
			if (current_distance < min_distance)
			{
				min_vertex = &vertices_[x][y];
				min_distance = current_distance;
			}
		}
	}
	min_vertex->pressure += -cos((PI / (double)(GRID_SIZE * 4))) * 30.0;
}