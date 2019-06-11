#include "hud.h"
#include "loaders.h"
#include <vector>
#include <iostream>
#include <glm/gtc/type_ptr.hpp>
#include <iomanip>

using namespace utils;
using namespace glm;

#define PI 3.14

//all option texts
const std::map<std::string,ivec2> hud::choices_ = 
{
	{"FPS: ",ivec2(25.0f, CONTROLS_HEIGHT - 25.0f)},
	{"Water settings: ",ivec2(25.0f, CONTROLS_HEIGHT - 75.0f) },
	{"Dam Break",ivec2(50.0f, CONTROLS_HEIGHT - 100.0f) },
	{"Pond",ivec2(50.0f, CONTROLS_HEIGHT - 125.0f) },
	{"Free Surface",ivec2(50.0f, CONTROLS_HEIGHT - 150.0f) },
	{"Nothing",ivec2(50.0f, CONTROLS_HEIGHT - 175.0f) },
	{"Terrain settings: ",ivec2(25.0f, CONTROLS_HEIGHT - 225.0f) },
	{"Plain",ivec2(50.0f, CONTROLS_HEIGHT - 250.0f) },
	{"Ground",ivec2(50.0f, CONTROLS_HEIGHT - 275.0f) },
	{"Crater",ivec2(50.0f, CONTROLS_HEIGHT - 300.0f) },
	{"Speed:",ivec2(25.0f, CONTROLS_HEIGHT - 350.0f) },
	{"0.25",ivec2(50.0f, CONTROLS_HEIGHT - 375.0f) },
	{"0.5",ivec2(50.0f, CONTROLS_HEIGHT - 400.0f) },
	{"1",ivec2(50.0f, CONTROLS_HEIGHT - 425.0f) },
	{"Interaction:",ivec2(25.0f, CONTROLS_HEIGHT - 475.0f) },
	{"Manual",ivec2(50.0f, CONTROLS_HEIGHT - 500.0f) },
	{"Rain",ivec2(50.0f, CONTROLS_HEIGHT - 525.0f) },
	{"Waterfall",ivec2(50.0f, CONTROLS_HEIGHT - 550.0f) },
	{"Touch",ivec2(50.0f, CONTROLS_HEIGHT - 575.0f) },
	{"Object",ivec2(50.0f, CONTROLS_HEIGHT - 600.0f) },
	{"State: ",ivec2(25.0f, CONTROLS_HEIGHT - 650.0f) },
	{"Camera: ",ivec2(25.0f, CONTROLS_HEIGHT - 675.0f)}
};

void hud::draw_crosshair()
{
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_buff_id_);
	glBufferData(GL_ARRAY_BUFFER, crosshair_vertex_values_.size()*sizeof(GLfloat), &crosshair_vertex_values_[0], GL_STATIC_DRAW);

	// use HUD shader
	glUseProgram(object_shader_id_);
	glm::mat4 projection = glm::ortho(0.0f, (float)window_width, 0.0f, (float)window_height);
	glUniformMatrix4fv(glGetUniformLocation(object_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, crosshair_buff_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw triangle
	glDrawArrays(GL_LINES, 0, 4);

	glDisableVertexAttribArray(0);
}

void utils::hud::draw_object(mat4 projection, vec3 camera_pos, vec3 camera_dir, float radius)
{
	// object distance from camera
	camera_dir *= 0.8;
	vec3 center = camera_pos + camera_dir;

	//object is a cube
	object_vertex_values_ = {
		center.x - radius, center.y - radius, center.z - radius, 
		center.x - radius, center.y + radius, center.z - radius,
		center.x + radius, center.y + radius, center.z - radius,
		center.x + radius, center.y - radius, center.z - radius,
		center.x + radius, center.y + radius, center.z - radius,
		center.x - radius, center.y - radius, center.z - radius,

		center.x + radius, center.y - radius, center.z - radius,
		center.x + radius, center.y + radius, center.z - radius,
		center.x + radius, center.y - radius, center.z + radius,
		center.x + radius, center.y - radius, center.z + radius,
		center.x + radius, center.y + radius, center.z + radius,
		center.x + radius, center.y + radius, center.z - radius,

		center.x - radius, center.y - radius, center.z - radius,
		center.x - radius, center.y + radius, center.z - radius,
		center.x - radius, center.y - radius, center.z + radius,
		center.x - radius, center.y - radius, center.z + radius,
		center.x - radius, center.y + radius, center.z + radius,
		center.x - radius, center.y + radius, center.z - radius,

		center.x - radius, center.y - radius, center.z + radius,
		center.x - radius, center.y + radius, center.z + radius,
		center.x + radius, center.y + radius, center.z + radius,
		center.x + radius, center.y - radius, center.z + radius,
		center.x + radius, center.y + radius, center.z + radius,
		center.x - radius, center.y - radius, center.z + radius,

		center.x - radius, center.y - radius, center.z - radius,
		center.x - radius, center.y - radius, center.z + radius,
		center.x + radius, center.y - radius, center.z + radius,
		center.x - radius, center.y - radius, center.z - radius,
		center.x + radius, center.y - radius, center.z + radius,
		center.x + radius, center.y - radius, center.z - radius,

		center.x - radius, center.y + radius, center.z - radius,
		center.x - radius, center.y + radius, center.z + radius,
		center.x + radius, center.y + radius, center.z + radius,
		center.x - radius, center.y + radius, center.z - radius,
		center.x + radius, center.y + radius, center.z + radius,
		center.x + radius, center.y + radius, center.z - radius
	};
	glBindBuffer(GL_ARRAY_BUFFER, object_buffer_id_);    
	glBufferData(GL_ARRAY_BUFFER,               
		object_vertex_values_.size() * sizeof(float), 
		&object_vertex_values_[0],  
		GL_STATIC_DRAW);
	// common object shader
	glUseProgram(object_shader_id_);
	glUniformMatrix4fv(glGetUniformLocation(object_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, object_buffer_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glDrawArrays(GL_TRIANGLES, 0, 3*2*6);

	glDisableVertexAttribArray(0);
}

void utils::hud::render_text_(std::string text, GLfloat x, GLfloat y) 
{
	float scale = 0.3f;
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

		x += (ch.advance >> 6) * scale;
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

	glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

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
		// store character for later use
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

void utils::hud::draw_box_(ivec2 b)
{	
	// only underline
	GLfloat box_vertices[6] = {
		b.x, b.y -3, 0.0f, b.x + UNDERLINE_SIZE, b.y - 3,0.0f
	};
	glBufferData(GL_ARRAY_BUFFER, sizeof(box_vertices), box_vertices, GL_STATIC_DRAW);
	
	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw line
	glDrawArrays(GL_LINES, 0, 2);

	glDisableVertexAttribArray(0);
}

bool utils::hud::change_state_(double x, double y,settings & s)
{
	y = CONTROLS_HEIGHT - y;
	//whether something changed
	bool changed = false;
	// go through all choices and change settings accordingly
	for (auto it = choices_.begin(); it != choices_.end(); it++)
	{
		if (x >= it->second.x && x <= it->second.x + 100 && y >= it->second.y &&  y <= it->second.y + 10)
		{
			if (it->first == "0.25" || it->first == "0.5" || it->first == "1")
			{
				s.speed = std::stof(it->first);
				break;
			}
			if (it->first == "Manual")
			{
				s.mode = interaction_mode::manual;
				break;
			}
			else if (it->first == "Rain")
			{
				s.mode = interaction_mode::rain;
				break;
			}	
			else if (it->first == "Waterfall")
			{
				s.mode = interaction_mode::waterfall;
				break;
			}
			else if (it->first == "Touch")
			{
				s.mode = interaction_mode::touch;
				break;
			}
			else if (it->first == "Object")
			{
				s.mode = interaction_mode::object;
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

	// clear
	glClearColor(0.3f, 0.3f, 0.3f, 0.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	// start program
	glBindVertexArray(vertex_array_id_);
	glUseProgram(hud_shader_id_);

	// simple projection
	glm::mat4 projection = glm::ortho(0.0f, (float)CONTROLS_WIDTH, 0.0f, (float)CONTROLS_HEIGHT);
	glUniformMatrix4fv(glGetUniformLocation(hud_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
	glUniform3f(glGetUniformLocation(hud_shader_id_, "textColor"), 0.5, 0.8f, 0.2f);
	// unbind textures
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
		render_text_(it->first, it->second.x, it->second.y);
	}
	//draw values
	//fps
	render_text_(fps_.str(), 75.0f, choices_.at("FPS: ").y);

	//running
	if (s.running)
		render_text_("RUNNING", 75.0f, choices_.at("State: ").y);
	else
		render_text_("PAUSED", 75.0f, choices_.at("State: ").y);

	//camera
	std::stringstream camera_pos_txt;
	camera_pos_txt << std::fixed << std::setprecision(2) << s.camera_pos.x << "  " << s.camera_pos.y << "  " << s.camera_pos.z;
	render_text_(camera_pos_txt.str(), 100.0f, choices_.at("Camera: ").y);
	glBindTexture(GL_TEXTURE_2D, 0);

	//chosen values, boxes
	glUseProgram(object_shader_id_);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glUniformMatrix4fv(glGetUniformLocation(object_shader_id_, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
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
	// interaction modes
	switch (s.mode)
	{
	case interaction_mode::manual:
		draw_box_(choices_.at("Manual"));
		break;
	case interaction_mode::rain:
		draw_box_(choices_.at("Rain"));
		break;
	case interaction_mode::waterfall:
		draw_box_(choices_.at("Waterfall"));
		break;
	case interaction_mode::touch:
		draw_box_(choices_.at("Touch"));
		break;
	case interaction_mode::object:
		draw_box_(choices_.at("Object"));
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
	// remember main window sizes
	glfwGetWindowSize(window, &window_width, &window_height);
	//crosshair vertices
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

	hud_shader_id_ = process_shaders("shaders/HUD_vertex_shader.vert", "shaders/HUD_fragment_shader.frag");

	//initialize free type
	if (!initialize_freetype_())
		return false;

	//create buffers for controls
	glGenVertexArrays(1, &vertex_array_id_);
	glBindVertexArray(vertex_array_id_);

	glGenBuffers(1, &char_buff_id_);
	glBindBuffer(GL_ARRAY_BUFFER, char_buff_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 6 * 4, NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &box_buffer_id_);
	glBindBuffer(GL_ARRAY_BUFFER, box_buffer_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 3 * 4, NULL, GL_DYNAMIC_DRAW);

	glBindVertexArray(0);

	// crosshair and object buffers and shader
	glGenBuffers(1, &crosshair_buff_id_);
	glGenBuffers(1, &object_buffer_id_);
	glGenBuffers(1, &object_element_id_);
	object_shader_id_ = process_shaders("shaders/object_vertex_shader.vert", "shaders/object_fragment_shader.frag");

	//return context back
	glfwMakeContextCurrent(window);

	return true;
}

void utils::hud::destroy()
{
	glDeleteBuffers(1, &crosshair_buff_id_);
	glDeleteBuffers(1, &box_buffer_id_);
	glDeleteBuffers(1, &char_buff_id_);
	glDeleteBuffers(1, &object_buffer_id_);
	glDeleteBuffers(1, &object_element_id_);
	glDeleteProgram(hud_shader_id_);
	glDeleteProgram(object_shader_id_);
	glDeleteVertexArrays(1, &vertex_array_id_);
}