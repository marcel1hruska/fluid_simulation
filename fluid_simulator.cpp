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

	initialise_buffers_();

	c.initialise(glGetUniformLocation(shader_id_,"transform_matrix"), window_, PLANE_HEIGHT);

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

		glUseProgram(shader_id_);

		//recompute advection and update unknowns
		//time should be machine dependent?
		s.recompute(0.01);

		// Compute height of each vertex
		s.adjust_grid();
		
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
			s.add_water(c.get_pos(),c.get_dir());
			right_button_pressed_ = false;
		}
		
		glfwSwapBuffers(window_);
		glfwPollEvents();

		std::cout << "Total water mass: " << s.water_mass() << std::endl;
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
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, s.indices().size() * sizeof(unsigned int), &s.indices()[0], GL_STATIC_DRAW);
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(s.vertex_buffer()), s.vertex_buffer(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// colors
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, color_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(s.color_buffer()), s.color_buffer(), GL_DYNAMIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, s.indices().size(), GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
}