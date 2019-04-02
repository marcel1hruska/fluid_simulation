#include "fluid_simulator.h"
#include <iostream>

using namespace simulation;
using namespace glm;
using namespace std;

int fluid_simulator::simulate()
{
	GLFWwindow* window;
	// Initialise GLFW
	if (!glfwInit())
	{
		cerr << "Cannot initialize GLFW" << endl;
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window
	window = glfwCreateWindow(1024, 768, "Fluid Simulation", NULL, NULL);
	if (window == NULL) {
		cerr << "Cannot open GLFW window" << endl;
		glfwTerminate();
		return -1;
	}
	// create context
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Cannot initialize GLEW" << endl;
		glfwTerminate();
		return -1;
	}

	// tracking cursor in window	
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	//black background
	glClearColor(0.0f, 0.0f, 0.0f, 0.0f);

	//glfwSetKeyCallback(window, [this](GLFWwindow* window, int key, int scancode, int action, int mods){});

	// depth enabled
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	GLuint array_id;
	glGenVertexArrays(1, &array_id);
	glBindVertexArray(array_id);

	// load the shaders
	GLuint program_id = utils::process_shaders("shaders/vertex_shader.vs", "shaders/fragmet_shader.fs");

	// cube vertices
	static const GLfloat vertex_buffer_data[] = {
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		-1.0f,-1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		-1.0f,-1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f,-1.0f,
		 1.0f,-1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f,-1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f,-1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f, 1.0f, 1.0f,
		-1.0f, 1.0f, 1.0f,
		 1.0f,-1.0f, 1.0f
	};

	//create vertex buffer
	GLuint vertex_buffer;
	glGenBuffers(1, &vertex_buffer);
	glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	//camera setup
	utils::camera c(glGetUniformLocation(program_id, "transform_matrix"), window);

	do {
		c.reposition();

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// use program/shaders
		glUseProgram(program_id);

		// send tranformation to the shader
		glUniformMatrix4fv(c.matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

		// vertices
		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
		glVertexAttribPointer(0,3,GL_FLOAT, GL_FALSE,0,(void*)0);

		// draw cube
		glDrawArrays(GL_TRIANGLES, 0, 12 * 3);

		glDisableVertexAttribArray(0);

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // end drawing
	while (glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS &&
		glfwWindowShouldClose(window) == 0);

	// Cleanup VBO and shader
	glDeleteBuffers(1, &vertex_buffer);
	glDeleteProgram(program_id);
	glDeleteVertexArrays(1, &array_id);

	glfwTerminate();

	return 0;
}