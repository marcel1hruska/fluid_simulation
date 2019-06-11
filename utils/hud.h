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
#define CONTROLS_HEIGHT 700
#define CONTROLS_WIDTH 300
#define UNDERLINE_SIZE 100

namespace utils
{
	enum class terrain_mode { crater, plain, ground };
	enum class water_mode { pond, nothing, free, dam };
	enum interaction_mode { manual, rain, waterfall, touch, object };

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
		interaction_mode mode;
		bool running;
		glm::vec3 camera_pos;
	};

	class hud
	{
	public:
		void draw_crosshair();
		void draw_object(glm::mat4 projection, glm::vec3 camera_pos, glm::vec3 camera_dir,float radius);
		bool display_hud(settings& s);
		bool initialize(GLFWwindow * window);
		void destroy();
	private:
		void render_text_(std::string text, GLfloat x, GLfloat y);
		std::stringstream fps_;
		double timer_ = 1;
		GLuint crosshair_buff_id_, hud_shader_id_, vertex_array_id_, char_buff_id_, object_shader_id_, box_buffer_id_, object_buffer_id_,object_element_id_;
		GLFWwindow * controls_;
		std::map<GLchar, character> characters_;
		bool initialize_freetype_();
		//triangle in the middle
		std::vector<GLfloat> crosshair_vertex_values_;
		std::vector<GLfloat> object_vertex_values_;
		std::vector<int>  object_elements_;
		static const std::map<std::string, glm::ivec2> choices_;
		void draw_box_(glm::ivec2 b);
		int window_width, window_height;
		bool change_state_(double x, double y, settings & s);
	};
}

#endif