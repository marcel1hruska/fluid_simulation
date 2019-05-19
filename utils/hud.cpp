#include "hud.h"
#include "shader.h"
#include <vector>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

using namespace utils;
using namespace glm;

#define UNDERLINE_SIZE 100

//all option texts
const std::map<std::string,box> hud::choices_ = 
{
	{"FPS: ",box(glm::ivec2(25.0f, CONTROLS_HEIGHT - 25.0f),UNDERLINE_SIZE)},
	{"Water settings: ",box(glm::ivec2(25.0f, CONTROLS_HEIGHT - 75.0f),UNDERLINE_SIZE) },
	{"Dam Break",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 100.0f),UNDERLINE_SIZE) },
	{"Pond",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 125.0f),UNDERLINE_SIZE) },
	{"Free Surface",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 150.0f),UNDERLINE_SIZE) },
	{"Nothing",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 175.0f),UNDERLINE_SIZE) },
	{"Terrain settings: ",box(glm::ivec2(25.0f, CONTROLS_HEIGHT - 225.0f),UNDERLINE_SIZE) },
	{"Plain",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 250.0f),UNDERLINE_SIZE) },
	{"Ground",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 275.0f),UNDERLINE_SIZE) },
	{"Crater",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 300.0f),UNDERLINE_SIZE) },
	{"Speed:",box(glm::ivec2(25.0f, CONTROLS_HEIGHT - 350.0f),UNDERLINE_SIZE) },
	{"0.05",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 375.0f),UNDERLINE_SIZE) },
	{"0.1",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 400.0f),UNDERLINE_SIZE) },
	{"0.15",box(glm::ivec2(50.0f, CONTROLS_HEIGHT - 425.0f),UNDERLINE_SIZE) },
	{"State: ",box(glm::ivec2(25.0f, CONTROLS_HEIGHT - 500.0f),UNDERLINE_SIZE) }
};

void hud::draw_crosshair()
{
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_buff_id_);
	glBufferData(GL_ARRAY_BUFFER, crosshair_vertex_values_.size()*sizeof(GLfloat), &crosshair_vertex_values_[0], GL_STATIC_DRAW);

	// use HUD shader
	glUseProgram(crosshair_shader_id_);
	glm::mat4 projection = glm::ortho(0.0f, (float)window_width, 0.0f, (float)window_height);
	glUniformMatrix4fv(glGetUniformLocation(crosshair_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_buff_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw triangle
	glDrawArrays(GL_LINES, 0, 4);

	glDisableVertexAttribArray(0);
}

void utils::hud::render_text_(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color) 
{
	// Iterate through all characters
	std::string::const_iterator c;
	for (c = text.begin(); c != text.end(); c++)
	{
		character ch = characters_[*c];

		GLfloat xpos = x + ch.bearing.x * scale;
		GLfloat ypos = y - (ch.size.y - ch.bearing.y) * scale;

		GLfloat w = ch.size.x * scale;
		GLfloat h = ch.size.y * scale;
		// Update char_buff_id_ for each character
		GLfloat vertices[6][4] = {
			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos,     ypos,       0.0, 1.0 },
			{ xpos + w, ypos,       1.0, 1.0 },

			{ xpos,     ypos + h,   0.0, 0.0 },
			{ xpos + w, ypos,       1.0, 1.0 },
			{ xpos + w, ypos + h,   1.0, 0.0 }
		};
		// Render glyph texture over quad
		glBindTexture(GL_TEXTURE_2D, ch.texture);

		glBindBuffer(GL_ARRAY_BUFFER, char_buff_id_);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices,GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, char_buff_id_);
		glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), (void*)0);

		glDrawArrays(GL_TRIANGLES, 0, 6);
		glDisableVertexAttribArray(0);

		x += (ch.advance >> 6) * scale; // Bitshift by 6 to get value in pixels
	}
}

bool utils::hud::initialize_freetype_()
{
	FT_Library ft;
	if (FT_Init_FreeType(&ft)) {
		fprintf(stderr, "Could not init freetype library\n");
		return false;
	}

	//initialize face with free sans
	FT_Face face;
	if (FT_New_Face(ft, "resources/FreeSans.ttf", 0, &face)) {
		fprintf(stderr, "Could not open font\n");
		return false;
	}
	FT_Set_Pixel_Sizes(face, 0, 48);

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1); // Disable byte-alignment restriction

	//stores ASCII textures
	for (GLubyte c = 0; c < 128; c++)
	{
		// Load character glyph 
		if (FT_Load_Char(face, c, FT_LOAD_RENDER))
		{
			std::cout << "ERROR::FREETYTPE: Failed to load Glyph" << std::endl;
			continue;
		}
		// Generate texture
		GLuint texture;
		glGenTextures(1, &texture);
		glBindTexture(GL_TEXTURE_2D, texture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_R8, face->glyph->bitmap.width, face->glyph->bitmap.rows, 0, GL_RED, GL_UNSIGNED_BYTE, face->glyph->bitmap.buffer);
		// Set texture options
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		// Now store character for later use
		character character = {
			texture,
			glm::ivec2(face->glyph->bitmap.width, face->glyph->bitmap.rows),
			glm::ivec2(face->glyph->bitmap_left, face->glyph->bitmap_top),
			face->glyph->advance.x
		};
		characters_.insert(std::pair<GLchar, utils::character>(c, character));
	}

	//release ft resources
	FT_Done_Face(face);
	FT_Done_FreeType(ft);

	return true;
}

void utils::hud::draw_box_(box b)
{	
	GLfloat box_vertices[6] = {
		b.pos.x, b.pos.y -3, 0.0f, b.pos.x + b.size, b.pos.y - 3,0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(box_vertices), box_vertices, GL_STATIC_DRAW);
	
	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw triangle
	glDrawArrays(GL_LINES, 0, 2);

	glDisableVertexAttribArray(0);
}

bool utils::hud::change_state_(double x, double y,settings & s)
{
	y = CONTROLS_HEIGHT - y;
	bool changed = false;
	for (auto it = choices_.begin(); it != choices_.end(); it++)
	{
		if (x >= it->second.pos.x && x <= it->second.pos.x + 100 && y >= it->second.pos.y &&  y <= it->second.pos.y + 10)
		{
			if (it->first == "0.05" || it->first == "0.1" || it->first == "0.15")
			{
				s.speed = std::stof(it->first);
				break;
			}
			changed = true;
			if (it->first == "Dam Break")
				s.water = water_mode::dam;
			else if (it->first == "Pond")
				s.water = water_mode::pond;
			else if (it->first == "Free Surface")
				s.water = water_mode::free;
			else if (it->first == "Nothing")
				s.water = water_mode::nothing;
			else if (it->first == "Plain")
				s.terrain = terrain_mode::plain;
			else if (it->first == "Crater")
				s.terrain = terrain_mode::crater;
			else if (it->first == "Ground")
				s.terrain = terrain_mode::ground;
			break;
		}
	}
	return changed;
}

bool utils::hud::display_hud(settings& s)
{
	//change context to options
	glfwMakeContextCurrent(controls_);

	bool changed = false;
	if (glfwGetMouseButton(controls_, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
	{
		double x, y;
		glfwGetCursorPos(controls_, &x, &y);
		changed = change_state_(x, y, s);
	}

	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);
	glBindVertexArray(vertex_array_id_);
	glUseProgram(shader_id_);
	glm::mat4 projection = glm::ortho(0.0f, (float)CONTROLS_WIDTH, 0.0f, (float)CONTROLS_HEIGHT);
	glUniformMatrix4fv(glGetUniformLocation(shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3f(glGetUniformLocation(shader_id_, "textColor"), 0.5, 0.8f, 0.2f);
	glActiveTexture(GL_TEXTURE0);
	//check if fps should be updated
	timer_ += s.delta;
	if (timer_ >= 1)
	{
		fps_ = std::stringstream();
		fps_ << 1 / s.delta;
		timer_ = 0;
	}
	//draw options
	for (auto it = choices_.begin(); it != choices_.end(); it++)
	{
		render_text_(it->first, it->second.pos.x, it->second.pos.y, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	}
	//draw values
	//fps
	render_text_(fps_.str(), 75.0f, choices_.at("FPS: ").pos.y, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));

	//running
	if (s.running)
		render_text_("RUNNING", 75.0f, choices_.at("State: ").pos.y, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	else
		render_text_("PAUSED", 75.0f, choices_.at("State: ").pos.y, 0.3f, glm::vec3(0.5, 0.8f, 0.2f));
	glBindTexture(GL_TEXTURE_2D, 0);

	//chosen values, boxes
	glUseProgram(crosshair_shader_id_);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glUniformMatrix4fv(glGetUniformLocation(crosshair_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	//terrain
	switch (s.terrain)
	{
	case utils::terrain_mode::ground:
		draw_box_(choices_.at("Ground"));
		break;
	case utils::terrain_mode::plain:
		draw_box_(choices_.at("Plain"));
		break;
	case utils::terrain_mode::crater:
		draw_box_(choices_.at("Crater"));
		break;
	}
	//water
	switch (s.water)
	{
	case utils::water_mode::dam:
		draw_box_(choices_.at("Dam Break"));
		break;
	case utils::water_mode::free:
		draw_box_(choices_.at("Free Surface"));
		break;
	case utils::water_mode::nothing:
		draw_box_(choices_.at("Nothing"));
		break;
	case utils::water_mode::pond:
		draw_box_(choices_.at("Pond"));
		break;
	}
	//speed
	std::stringstream float_str;
	float_str << s.speed;
	draw_box_(choices_.at(float_str.str()));

	//swap controls
	glfwSwapBuffers(controls_);

	return changed;
}

bool hud::initialize(GLFWwindow * window)
{
	glfwGetWindowSize(window, &window_width, &window_height);
	crosshair_vertex_values_ = { window_width /2.0f - 10,window_height/2.0f,0.0f,
								window_width / 2.0f + 10,window_height / 2.0f,0.0f,
								window_width / 2.0f,window_height / 2.0f + 10,0.0f,
								window_width / 2.0f,window_height / 2.0f - 10,0.0f };
	//non resizeable options
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);

	//initialize window
	controls_ = glfwCreateWindow(CONTROLS_WIDTH, CONTROLS_HEIGHT, "Controls", NULL, window);
	if (controls_ == NULL) {
		std::cerr << "Cannot open GLFW window" << std::endl;
		glfwTerminate();
		return false;
	}

	glfwMakeContextCurrent(controls_);

	// Initialize GLEW for controls context
	glewExperimental = true;
	if (glewInit() != GLEW_OK) {
		std::cerr << "Cannot initialize GLEW" << std::endl;
		glfwTerminate();
		return false;
	}

	// opengl options
	glEnable(GL_CULL_FACE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	shader_id_ = process_shaders("shaders/HUD_vertex_shader.vert", "shaders/HUD_fragment_shader.frag");

	//initialize free type
	if (!initialize_freetype_())
		return false;

	//create buffers for controls
	glGenVertexArrays(1, &vertex_array_id_);
	glBindVertexArray(vertex_array_id_);

	glGenBuffers(1, &char_buff_id_);
	glBindBuffer(GL_ARRAY_BUFFER, char_buff_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);
	//glEnableVertexAttribArray(0);
	//glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 4 * sizeof(GLfloat), 0);

	glGenBuffers(1, &box_buffer_id_);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, NULL, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);


	//one more buffer for crosshair
	glGenBuffers(1, &crosshair_buff_id_);
	crosshair_shader_id_ = process_shaders("shaders/crosshair_vertex_shader.vert", "shaders/crosshair_fragment_shader.frag");

	//return context back
	glfwMakeContextCurrent(window);

	return true;
}

void utils::hud::destroy()
{
	glDeleteBuffers(1, &crosshair_buff_id_);
	glDeleteBuffers(1, &box_buffer_id_);
	glDeleteBuffers(1, &char_buff_id_);
	glDeleteProgram(shader_id_);
	glDeleteProgram(crosshair_shader_id_);
	glDeleteVertexArrays(1, &vertex_array_id_);
}