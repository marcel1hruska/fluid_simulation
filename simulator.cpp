#include "simulator.h"
#include <iostream>

using namespace simulation;
using namespace glm;
using namespace std;


void simulator::simulate()
{
	if (!initialise_openGL_())
		return;

	//create vertex array
	glGenVertexArrays(1, &array_id_);
	glBindVertexArray(array_id_);

	//initialize compute shader
	heights_id_ = m.initialize();

	//initialize own buffers
	initialise_buffers_();

	//TODO water textures
	//location_reflection_texture_ = glGetUniformLocation(shader_id_,"reflection_texture");
	//location_refraction_texture_ = glGetUniformLocation(shader_id_, "refraction_texture");

	c.initialise(glGetUniformLocation(water_shader_id_, "transform_matrix"), window_, WATER_HEIGHT + TERRAIN_HEIGHT);

	h.initialize();

	last_frame_ = glfwGetTime() - 0.01;

	//end on escape
	while (!glfwWindowShouldClose(window_) && glfwGetKey(window_,GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		auto current_frame = glfwGetTime();
		delta_time_ = current_frame - last_frame_;
		last_frame_ = current_frame;

		// clear colors and depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		c.reposition(delta_time_);
		
		//draw meshes
		draw_terrain_();

		draw_water_();
		
		//start simulation on space
		if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS && !was_space_pressed_)
		{
			run_ = !run_;
			was_space_pressed_ = true;
		}
		else if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_RELEASE)
		{
			was_space_pressed_ = false;
		}

		if (run_)
		{
			//check for right mouse button -> add water to the middle pressed
			if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				h.draw_crosshair();
				m.recompute(delta_time_, c.get_pos(), c.get_dir());
			}
			else
				//recompute advection and update unknowns
				m.recompute(delta_time_);
		}

		glfwSwapBuffers(window_);
		glfwPollEvents();
	}

	delete_buffers_();

	h.destroy();
	m.destroy();

	glfwTerminate();
}

bool simulation::simulator::initialise_openGL_()
{
	// initialize GLFW
	if (!glfwInit())
	{
		cerr << "Cannot initialize GLFW" << endl;
		return false;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
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

void simulation::simulator::initialise_buffers_()
{
	water_shader_id_ = utils::process_shaders("shaders/water_vertex_shader.vert", "shaders/water_fragment_shader.frag");
	terrain_shader_id_ = utils::process_shaders("shaders/terrain_vertex_shader.vert", "shaders/terrain_fragment_shader.frag");

	auto heights = m.get_heights();

	//create water vertex buffer
	glGenBuffers(1, &water_vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, water_vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES*sizeof(vec3), NULL, GL_STATIC_DRAW);

	//initialize water vertices
	vec3 * water_vertices = (vec3 *)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLES * sizeof(vec3), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			//initialize water to (-1,1)
			water_vertices[x + y*GRID_SIZE].x = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			water_vertices[x + y * GRID_SIZE].y = heights[y*GRID_SIZE + x].water;
			water_vertices[x + y * GRID_SIZE].z = (GLfloat)(y - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//create terrain vertex buffer
	glGenBuffers(1, &terrain_vertex_id_);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_id_);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(vec3), NULL, GL_STATIC_DRAW);

	//initialize terrain vertices
	vec3 * terrain_vertices = (vec3 *)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLES * sizeof(vec3), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			terrain_vertices[y*GRID_SIZE + x].x = (GLfloat)(x - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
			terrain_vertices[y*GRID_SIZE + x].y = heights[y*GRID_SIZE + x].terrain;
			terrain_vertices[y*GRID_SIZE + x].z = (GLfloat)(y - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//create water color buffer
	glGenBuffers(1, &water_color_id_);
	glBindBuffer(GL_ARRAY_BUFFER, water_color_id_);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(vec4), NULL, GL_STATIC_DRAW);

	//initialize color buffer
	vec4 * water_colors = (vec4 *)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLES * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			water_colors[y*GRID_SIZE + x].r = water_colors[y*GRID_SIZE + x].g = 0.7;
			water_colors[y*GRID_SIZE + x].b = 0.7;
			//half transparent
			water_colors[y*GRID_SIZE + x].a = 0.5;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//create terrain color buffer
	glGenBuffers(1, &terrain_color_id_);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_color_id_);
	glBufferData(GL_ARRAY_BUFFER, PARTICLES * sizeof(vec4), NULL, GL_STATIC_DRAW);

	//initialize color buffer
	vec4 * terrain_colors = (vec4 *)glMapBufferRange(GL_ARRAY_BUFFER, 0, PARTICLES * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			terrain_colors[y*GRID_SIZE + x].r = 0.3 + heights[y*GRID_SIZE + x].terrain*0.6;
			terrain_colors[y*GRID_SIZE + x].g = 0.2 + heights[y*GRID_SIZE + x].terrain*0.3;
			terrain_colors[y*GRID_SIZE + x].b = 0;
			//half transparent
			terrain_colors[y*GRID_SIZE + x].a = 1.0;
		}
	}
	glUnmapBuffer(GL_ARRAY_BUFFER);

	//buffer for the water//terrain indices
	glGenBuffers(1, &mesh_element_id_);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, mesh_size, NULL, GL_STATIC_DRAW);

	size_t * mesh = (size_t *)glMapBufferRange(GL_ELEMENT_ARRAY_BUFFER, 0, mesh_size, GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int y = 0; y < GRID_SIZE - 1; y++)
	{
		for (int x = 0; x < GRID_SIZE - 1; x++)
		{
			//create triangles for terrain/water
			push_indices_(mesh, x, y);
		}
	}
	glUnmapBuffer(GL_ELEMENT_ARRAY_BUFFER);
}

void simulation::simulator::delete_buffers_()
{
	glDeleteBuffers(1, &water_vertex_id_);
	glDeleteBuffers(1, &terrain_vertex_id_);
	glDeleteBuffers(1, &water_color_id_);
	glDeleteBuffers(1, &terrain_color_id_);
	glDeleteBuffers(1, &mesh_element_id_);
	glDeleteProgram(water_shader_id_);
	glDeleteProgram(terrain_shader_id_);
	glDeleteVertexArrays(1, &array_id_);
}

void simulation::simulator::draw_water_()
{
	glUseProgram(water_shader_id_);

	// send transformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, water_vertex_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, water_color_id_);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// height
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m.heights_id());
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// height
	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, m.flux_id());
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_size, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void simulation::simulator::draw_terrain_()
{
	auto color_id = glGetUniformLocation(terrain_shader_id_, "color");

	glUseProgram(terrain_shader_id_);
	// send transformation to the shader
	glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// vertices
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_color_id_);
	glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_size, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
}

void simulation::simulator::push_indices_(size_t * indices, size_t x, size_t y)
{
	//first triangle
	indices[y*GRID_SIZE*6 + x*6] = y * GRID_SIZE + x;
	indices[y*GRID_SIZE*6 + x*6 + 1] = y * GRID_SIZE + x + 1;
	indices[y*GRID_SIZE*6 + x*6 + 2] = (y + 1) * GRID_SIZE + (x + 1);
	//second triangle
	indices[y*GRID_SIZE*6 + x*6 + 3] = y * GRID_SIZE + x;
	indices[y*GRID_SIZE*6 + x*6 + 4] = (y + 1)  * GRID_SIZE + x;
	indices[y*GRID_SIZE*6 + x*6 + 5] = (y + 1) * GRID_SIZE + (x + 1);
}
