#include "simulator.h"
#include <iostream>
#include <cstdio>

using namespace simulation;
using namespace glm;
using namespace std;

void simulator::simulate()
{
	if (!initialise_openGL_())
		return;

	// program seetings
	utils::settings s = { 0, utils::terrain_mode::crater, utils::water_mode::pond, 1,  utils::interaction_mode::manual, false };

	//initialize compute shaders (flow, update, normals)
	m.initialize(&s);

	//initialize own buffers
	if (!initialise_buffers_())
		return;

	//initialize camera
	c.initialise(water_shader_id_, terrain_shader_id_, window_, WATER_HEIGHT + TERRAIN_HEIGHT);

	//initialize control panel
	if (!h.initialize(window_))
	{
		return;
	};

	last_frame_ = glfwGetTime() - 0.01;

	// initialize normals as well
	m.compute_normals();

	//end on escape
	while (!glfwWindowShouldClose(window_) && glfwGetKey(window_,GLFW_KEY_ESCAPE) != GLFW_PRESS)
	{
		glfwMakeContextCurrent(window_);

		auto current_frame = glfwGetTime();
		s.delta = current_frame - last_frame_;
		last_frame_ = current_frame;

		// clear colors and depth
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// move camera and save its position
		c.reposition(s.delta);
		s.camera_pos = c.get_pos();
		/*
		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, reflection_frame_buffer_id_);

		//create reflection texture
		render_scene_();

		//render reflection to texture
		glBindTexture(GL_TEXTURE_2D, reflection_texture_id_);
		//glCopyTexSubImage2D copies the frame buffer
		//to the bound texture
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);


		glBindTexture(GL_TEXTURE_2D, 0);
		glBindFramebuffer(GL_FRAMEBUFFER, refraction_frame_buffer_id_);

		//create refraction texture
		render_scene_();

		//render color buffer to texture
		glBindTexture(GL_TEXTURE_2D, refraction_texture_id_);
		glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);

		//render depth to texture
		glBindTexture(GL_TEXTURE_2D, refraction_depth_texture_id_);
		glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, 0);

		glBindFramebuffer(GL_FRAMEBUFFER, 0);
		*/

		//start simulation on space
		if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_PRESS && !was_space_pressed_) 
		{
			s.running = !s.running;
			was_space_pressed_ = true;
		}
		else if (glfwGetKey(window_, GLFW_KEY_SPACE) == GLFW_RELEASE)
		{
			was_space_pressed_ = false;
		}

		if (s.running)
		{
			// action on right mouse button
			if (glfwGetMouseButton(window_, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS)
			{
				m.recompute(c.get_pos(), c.get_dir(), true);
				if (s.mode == utils::interaction_mode::object)
					h.draw_object(c.transform_matrix, c.get_pos(), c.get_dir(),CUBE_SIZE/(float)GRID_SIZE);
				else if (s.mode == utils::interaction_mode::manual || s.mode == utils::interaction_mode::touch)
					h.draw_crosshair();
			}
			else
			{
				//recompute without using action
				m.recompute(c.get_pos(), c.get_dir(), false);
			}
		}

		// render scene and water
		render_scene_();
		draw_water_();

		// display control panel
		if (h.display_hud(s))
		{
			// if true, reinitialize
			s.running = false;
			m.initialize_heights();
			m.compute_normals();
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
	// turn off vsync
	glfwSwapInterval(0);

	// Initialize GLEW
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		cerr << "Cannot initialize GLEW" << endl;
		glfwTerminate();
		return false;
	}

	// FPS mode	
	glfwSetInputMode(window_, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	//white background
	glClearColor(0.5f, 0.5f, 0.5f, 0.5f);

	// depth enabled
	glEnable(GL_DEPTH_TEST);
	glDepthFunc(GL_LESS);

	//blending
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	
	//create vertex array
	glGenVertexArrays(1, &array_id_);
	glBindVertexArray(array_id_);

	return true;
}

bool simulation::simulator::initialise_buffers_()
{
	//shaders for water and terrain
	water_shader_id_ = utils::process_shaders("shaders/water_vertex_shader.vert", "shaders/water_fragment_shader.frag");
	terrain_shader_id_ = utils::process_shaders("shaders/terrain_vertex_shader.vert", "shaders/terrain_fragment_shader.frag");

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
			water_vertices[x + y * GRID_SIZE].y = 0;
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
			terrain_vertices[y*GRID_SIZE + x].y = 0;
			terrain_vertices[y*GRID_SIZE + x].z = (GLfloat)(y - GRID_SIZE / 2) / (GLfloat)(GRID_SIZE / 2);
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

	/*
	//reflection
	glGenFramebuffers(1, &reflection_frame_buffer_id_);
	glBindFramebuffer(GL_FRAMEBUFFER, reflection_frame_buffer_id_);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &reflection_texture_id_);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, reflection_texture_id_);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, reflection_texture_id_, 0);

	glGenRenderbuffers(1, &reflection_depth_buffer_id_);
	glBindRenderbuffer(GL_RENDERBUFFER, reflection_depth_buffer_id_);
	glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, WINDOW_WIDTH, WINDOW_HEIGHT);
	glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, reflection_depth_buffer_id_);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;


	//refraction
	glGenFramebuffers(1, &refraction_frame_buffer_id_);
	glBindFramebuffer(GL_FRAMEBUFFER, refraction_frame_buffer_id_);
	glDrawBuffer(GL_COLOR_ATTACHMENT0);

	glGenTextures(1, &refraction_texture_id_);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, refraction_texture_id_);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, refraction_texture_id_, 0);

	glGenTextures(1, &refraction_depth_texture_id_);
	// "Bind" the newly created texture : all future texture functions will modify this texture
	glBindTexture(GL_TEXTURE_2D, refraction_depth_texture_id_);
	// Give an empty image to OpenGL ( the last "0" )
	glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT32, WINDOW_WIDTH, WINDOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, 0);
	// Poor filtering. Needed !
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	// Set "renderedTexture" as our colour attachement #0
	glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, refraction_depth_texture_id_, 0);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
		return false;
	*/
	glUseProgram(water_shader_id_);
	//glUniform1i(glGetUniformLocation(water_shader_id_, "reflection_texture"),0);
	//glUniform1i(glGetUniformLocation(water_shader_id_, "refraction_texture"),1);
	glUniform3fv(glGetUniformLocation(water_shader_id_, "light_pos"), 1, &light_position_[0]);
	glUniform3fv(glGetUniformLocation(water_shader_id_, "camera_pos"), 1, &c.get_pos()[0]);

	//initialise terrain textures
	forest_texture_ = utils::loadBMP("resources/forest.bmp");
	grass_texture_ = utils::loadBMP("resources/grass.bmp");
	rocks_texture_ = utils::loadBMP("resources/rocks.bmp");
	sand_texture_ = utils::loadBMP("resources/sand.bmp");
	
	glUseProgram(terrain_shader_id_);
	glUniform1i(glGetUniformLocation(terrain_shader_id_, "sand"), 0);
	glUniform1i(glGetUniformLocation(terrain_shader_id_, "grass"), 1);
	glUniform1i(glGetUniformLocation(terrain_shader_id_, "rock"), 2);
	glUniform1i(glGetUniformLocation(terrain_shader_id_, "forest"), 3);

	return true;
}

void simulation::simulator::delete_buffers_()
{
	glDeleteTextures(1, &forest_texture_);
	glDeleteTextures(1, &sand_texture_);
	glDeleteTextures(1, &rocks_texture_);
	glDeleteTextures(1, &grass_texture_);
	glDeleteBuffers(1, &water_vertex_id_);
	glDeleteBuffers(1, &terrain_vertex_id_);
	glDeleteBuffers(1, &mesh_element_id_);
	glDeleteProgram(water_shader_id_);
	glDeleteProgram(terrain_shader_id_);
	glDeleteVertexArrays(1, &array_id_);
	//glDeleteTextures(1, &reflection_texture_id_);
	//glDeleteTextures(1, &refraction_texture_id_);
	//glDeleteTextures(1, &refraction_depth_texture_id_);
	//glDeleteFramebuffers(1, &reflection_frame_buffer_id_);
	//glDeleteFramebuffers(1, &refraction_frame_buffer_id_);
	//glDeleteRenderbuffers(1, &reflection_depth_buffer_id_);
}

void simulation::simulator::draw_water_()
{
	/*
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, reflection_texture_id_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, refraction_texture_id_);
	*/
	glUseProgram(water_shader_id_);

	// send transformation to the shader
	glUniformMatrix4fv(c.water_matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// update camera position
	glUniform3fv(glGetUniformLocation(water_shader_id_, "camera_pos"), 1, &c.get_pos()[0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, water_vertex_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// height
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m.heights_id());
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// normals
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, m.normals_id());
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);

	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_size, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void simulation::simulator::draw_terrain_()
{
	// textures
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, sand_texture_);
	glActiveTexture(GL_TEXTURE1);
	glBindTexture(GL_TEXTURE_2D, grass_texture_);
	glActiveTexture(GL_TEXTURE2);
	glBindTexture(GL_TEXTURE_2D, rocks_texture_);
	glActiveTexture(GL_TEXTURE3);
	glBindTexture(GL_TEXTURE_2D, forest_texture_);

	glUseProgram(terrain_shader_id_);

	// send transformation to the shader
	glUniformMatrix4fv(c.terrain_matrix_id, 1, GL_FALSE, &c.transform_matrix[0][0]);

	// vertices
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, terrain_vertex_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// height
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, m.heights_id());
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mesh_element_id_);
	
	// draw mesh
	glDrawElements(GL_TRIANGLES, mesh_size, GL_UNSIGNED_INT, (void*)0);

	glDisableVertexAttribArray(0);
}

void simulation::simulator::render_scene_()
{
	draw_terrain_();
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
