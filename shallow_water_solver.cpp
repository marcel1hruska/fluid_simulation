#include "shallow_water_solver.h"
#include "utils/shader.h"

using namespace simulation;
using namespace glm;

//debug
void GLAPIENTRY MessageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

GLuint simulation::swe_solver::initialize()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	//compile compute shader
	compute_shader_id_ = utils::process_compute_shader("shaders/swe_compute_shader.comp");

	//height storage buffer
	glGenBuffers(1, &height_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, height_ssbo_id_);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, PARTICLES * sizeof(height), NULL, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

	//initialize, keep mapped to heights_
	heights_ = (height *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, PARTICLES * sizeof(height), GL_MAP_WRITE_BIT | GL_MAP_PERSISTENT_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (size_t y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			//DAM BREAK
			if (x < GRID_SIZE / 4 && y < GRID_SIZE / 4)
				heights_[x + y*GRID_SIZE].water = WATER_HEIGHT + 0.5;
			else
				heights_[x + y*GRID_SIZE].water = WATER_HEIGHT;

			//NORMAL
			//heights_[x + y*GRID_SIZE].water = WATER_HEIGHT;

			//TODO prerendered terrain e.g.
			//TODO not working correctly, buggy, probably tracking of free surface is needed
			/*
			if (x < GRID_SIZE / 4 && y < GRID_SIZE / 4)
				heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT + 0.2;
			else
				heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT;
				*/

			// constant 0 terrain height for now
			heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT;
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);


	//velocity storage buffer, do not keep
	glGenBuffers(1, &vel_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, vel_ssbo_id_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, (PARTICLES+1) * sizeof(velocity), NULL, GL_STATIC_DRAW);

	//init
	velocity * vels = (velocity *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, (PARTICLES + 1) * sizeof(velocity), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int x = 0; x < PARTICLES + 1; x++)
	{
		vels[x].x = 0.0;
		vels[x].y = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//additional information storage buffer
	glGenBuffers(1, &add_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, add_ssbo_id_);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, 2*sizeof(add_info), NULL, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

	info_ = (add_info*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 2*sizeof(add_info), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	return height_ssbo_id_;
}

void simulation::swe_solver::recompute(double delta, vec3 pos, vec3 dir)
{
	info_[0] = { pos, (delta > 0.1) ? 0.1f : (float)delta };
	info_[1] = { dir,1 };

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, height_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vel_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, add_ssbo_id_);

	//compute shader
	glUseProgram(compute_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

height * simulation::swe_solver::get_heights()
{
	return heights_;
}


void simulation::swe_solver::destroy()
{
	glDeleteBuffers(1, &height_ssbo_id_);
	glDeleteBuffers(1, &vel_ssbo_id_);
	glDeleteBuffers(1, &add_ssbo_id_);
	glDeleteProgram(compute_shader_id_);
}
