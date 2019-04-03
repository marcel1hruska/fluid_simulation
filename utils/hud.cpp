#include "hud.h"
#include "shader.h"
#include <vector>

using namespace utils;
using namespace glm;

void hud::draw_crosshair()
{
	glBindBuffer(GL_ARRAY_BUFFER, buff_id_);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_values_), vertex_values_, GL_STATIC_DRAW);

	// use HUD shader
	glUseProgram(shader_id_);

	// position buffer
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, buff_id_);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);

	// draw triangle
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDisableVertexAttribArray(0);
}

void hud::init_HUD()
{
	glGenBuffers(1, &buff_id_);
	shader_id_ = process_shaders("shaders/HUD_vertex_shader.vs", "shaders/HUD_fragment_shader.fs");
}

void utils::hud::delete_HUD()
{
	glDeleteBuffers(1, &buff_id_);
	glDeleteProgram(shader_id_);
}