#include "motion.h"
#include "utils/shader.h"
#include "utils/perlin.h"

using namespace simulation;
using namespace glm;

//debug
void GLAPIENTRY MessageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

GLuint simulation::motion::initialize()
{
	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	//for terrain
	utils::perlin_noise p;

	//compile compute shader
	//swe_shader_id_ = utils::process_compute_shader("shaders/swe_compute_shader.comp");
	flow_shader_id_ = utils::process_compute_shader("shaders/flow_compute_shader.comp");
	update_shader_id_ = utils::process_compute_shader("shaders/update_compute_shader.comp");

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
			if (distance(coords(x,y),vec2(-1,-1)) < 1.2)
				heights_[x + y*GRID_SIZE].water = WATER_HEIGHT + 0.5;
			else
				heights_[x + y*GRID_SIZE].water = WATER_HEIGHT;
				
			//NORMAL
			//heights_[x + y*GRID_SIZE].water = WATER_HEIGHT;

			//PLATFORM, not very nice
			/*
			if (x < GRID_SIZE / 4 && y < GRID_SIZE / 4)
				heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT + 0.2;
			else
				heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT;
			*/
			// GROUND
			//heights_[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT;
			//PERLIN NOISE
			auto current = coords(x, y);
			heights_[x + y * GRID_SIZE].terrain = p.make_some_noise(16*(current.x + 1)/2, 16*(current.y + 1) / 2, TERRAIN_HEIGHT, 7);
		}
	}

	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	/*
	SWE
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
	*/

	//FLOW
	//flux storage buffer
	glGenBuffers(1, &flux_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flux_ssbo_id_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLES * sizeof(flux), NULL, GL_STATIC_DRAW);

	//init
	fl_ = (flux *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, PARTICLES * sizeof(flux), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int x = 0; x < PARTICLES; x++)
	{
		fl_[x].top = 0.0;
		fl_[x].bottom = 0.0;
		fl_[x].left = 0.0;
		fl_[x].right = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//additional information storage buffer
	glGenBuffers(1, &add_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, add_ssbo_id_);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, 2*sizeof(add_info), NULL, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);

	info_ = (add_info*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, 2*sizeof(add_info), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	return height_ssbo_id_;
}

void simulation::motion::recompute(double delta, vec3 pos, vec3 dir)
{
	info_[0] = { pos, 0.1f };
	info_[1] = { dir,1 };

	/*
	SWE
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, height_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, vel_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, flux_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, add_ssbo_id_);

	//swe compute shader
	glUseProgram(compute_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	*/

	//FLOW
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, height_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flux_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, add_ssbo_id_);

	// flow compute shader
	glUseProgram(flow_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	glUseProgram(update_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}

height * simulation::motion::get_heights()
{
	return heights_;
}

GLuint simulation::motion::flux_id()
{
	return flux_ssbo_id_;
}

GLuint simulation::motion::heights_id()
{
	return height_ssbo_id_;
}


void simulation::motion::destroy()
{
	glDeleteBuffers(1, &height_ssbo_id_);
	//glDeleteBuffers(1, &vel_ssbo_id_);		SWE
	glDeleteBuffers(1, &add_ssbo_id_);
	glDeleteBuffers(1, &flux_ssbo_id_);		//FLOW
	glDeleteProgram(compute_shader_id_);
}

vec2 motion::coords(int x, int y)
{
	return vec2((x - GRID_SIZE / 2) / (float)GRID_SIZE / 2, (y - GRID_SIZE / 2) / (float)GRID_SIZE / 2);
}