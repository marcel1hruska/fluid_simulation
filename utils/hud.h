#ifndef HUD_H
#define HUD_H

#include <GL/glew.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <GLFW/glfw3.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include <sstream>
#include <map>
#include <vector>
#define CONTROLS_HEIGHT 600
#define CONTROLS_WIDTH 300

namespace utils
{
	enum class terrain_mode { crater, plain, ground };
	enum class water_mode { pond, nothing, free, dam };

	struct character {
		GLuint texture; 
		glm::ivec2 size; 
		glm::ivec2 bearing;
		GLuint advance;
	};

	struct settings {
		double delta;
		terrain_mode terrain;
		water_mode water;
		float speed;
		bool running;
	};

	struct box {
		box(glm::ivec2 pos, int size) : pos(pos), size(size) {}
		glm::ivec2 pos;
		int size;
	};

	class hud
	{
	public:
		void draw_crosshair();
		bool display_hud(settings& s);
		bool initialize(GLFWwindow * window);
		void destroy();
	private:
		void render_text_(std::string text, GLfloat x, GLfloat y, GLfloat scale, glm::vec3 color);
		std::stringstream fps_;
		double timer_ = 1;
		GLuint crosshair_buff_id_, shader_id_, vertex_array_id_, char_buff_id_, crosshair_shader_id_, box_buffer_id_;
		GLFWwindow * controls_;
		std::map<GLchar, character> characters_;
		bool initialize_freetype_();
		//triangle in the middle
		std::vector<GLfloat> crosshair_vertex_values_;
		static const std::map<std::string, box> choices_;
		void draw_box_(box b);
		int window_width, window_height;
		bool change_state_(double x, double y, settings & s);
	};
}

#endif