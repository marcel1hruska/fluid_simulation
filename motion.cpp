#include "motion.h"
#include "utils/loaders.h"

using namespace simulation;
using namespace glm;

//debug
void GLAPIENTRY MessageCallback(GLenum source,GLenum type,GLuint id,GLenum severity,GLsizei length,const GLchar* message,const void* userParam)
{
	fprintf(stderr, "GL CALLBACK: %s type = 0x%x, severity = 0x%x, message = %s\n",
		(type == GL_DEBUG_TYPE_ERROR ? "** GL ERROR **" : ""),
		type, severity, message);
}

void simulation::motion::initialize(utils::settings * s)
{
	//remember settings
	s_ = s;

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(MessageCallback, 0);

	// noise for terrain
	utils::perlin_noise p;

	//compile compute shaders
	//swe_shader_id_ = utils::process_compute_shader("shaders/swe_compute_shader.comp");
	flow_shader_id_ = utils::process_compute_shader("shaders/flow_compute_shader.comp");
	update_shader_id_ = utils::process_compute_shader("shaders/update_compute_shader.comp");
	normals_shader_id_ = utils::process_compute_shader("shaders/normals_compute_shader.comp");

	//height storage buffer
	glGenBuffers(1, &height_ssbo_id_);
	initialize_heights();

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
		vels[x].water = 0.0;
		vels[x].y = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
	*/

	//flux storage buffer
	glGenBuffers(1, &flux_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, flux_ssbo_id_);
	glBufferData(GL_SHADER_STORAGE_BUFFER, PARTICLES * sizeof(flux), NULL, GL_STATIC_DRAW);

	//init
	flux * fl = (flux *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, PARTICLES * sizeof(flux), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int x = 0; x < PARTICLES; x++)
	{
		fl[x].top = 0.0;
		fl[x].bottom = 0.0;
		fl[x].left = 0.0;
		fl[x].right = 0.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//normal storage buffer
	glGenBuffers(1, &normal_ssbo_id_);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, normal_ssbo_id_);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, PARTICLES * sizeof(vec4), NULL, GL_MAP_WRITE_BIT);
	//init
	vec4 * normals = (vec4*)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, PARTICLES * sizeof(vec4), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	for (int x = 0; x < PARTICLES; x++)
	{
		normals[x].x = 0.0;
		normals[x].y = 0.0;
		normals[x].z = 0.0;
		normals[x].w = 1.0;
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

	//additional information uniform buffer, we want to keep this one
	glGenBuffers(1, &add_ubo_id_);
	glBindBuffer(GL_UNIFORM_BUFFER, add_ubo_id_);
	glBufferStorage(GL_UNIFORM_BUFFER, sizeof(add_info), NULL, GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT);
	info_ = (add_info*)glMapBufferRange(GL_UNIFORM_BUFFER, 0, sizeof(add_info), GL_MAP_PERSISTENT_BIT | GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);
	glUnmapBuffer(GL_UNIFORM_BUFFER);
}

void simulation::motion::recompute(vec3 pos, vec3 dir, bool mode_used)
{
	// refresh information for the buffer
	info_->camera_direction = vec4(dir,0);
	info_->camera_position = vec4(pos, 0);
	// mode, 5 - nothing
	info_->mode = (mode_used) ? s_->mode : 5;
	// random position for rain drop
	info_->rand_pos = vec4(std::rand() % GRID_SIZE, std::rand() % GRID_SIZE,0,0);
	//  speed
	info_->delta = max(s_->speed * 0.15,s_->speed * s_->delta);
	info_->grid_size = GRID_SIZE;
	// size of cube object
	info_->cube_size = CUBE_SIZE;

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

	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, height_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, flux_ssbo_id_);
	glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(flow_shader_id_, "settings"), add_ubo_id_);

	// flow compute shader
	glUseProgram(flow_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	// update heights
	glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(update_shader_id_, "settings"), add_ubo_id_);
	glUseProgram(update_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

	compute_normals();
}

GLuint simulation::motion::heights_id()
{
	return height_ssbo_id_;
}

GLuint simulation::motion::normals_id()
{
	return normal_ssbo_id_;
}

void simulation::motion::destroy()
{
	glDeleteBuffers(1, &height_ssbo_id_);
	//glDeleteBuffers(1, &vel_ssbo_id_);		SWE
	glDeleteBuffers(1, &add_ubo_id_);
	glDeleteBuffers(1, &flux_ssbo_id_);	
	glDeleteBuffers(1, &normal_ssbo_id_);
	glDeleteProgram(flow_shader_id_);
	glDeleteProgram(normals_shader_id_);
	glDeleteProgram(update_shader_id_);
}

void simulation::motion::initialize_heights()
{
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, height_ssbo_id_);
	glBufferStorage(GL_SHADER_STORAGE_BUFFER, PARTICLES * sizeof(height), NULL, GL_MAP_WRITE_BIT);
	height * heights = (height *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0, PARTICLES * sizeof(height), GL_MAP_WRITE_BIT | GL_MAP_INVALIDATE_BUFFER_BIT);

	for (size_t y = 0; y < GRID_SIZE; y++)
	{
		for (int x = 0; x < GRID_SIZE; x++)
		{
			auto current = coords(x, y);

			// switch heights according to the modes
			switch (s_->terrain)
			{
			case utils::terrain_mode::ground:
				heights[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT * perlin_noise_[x + y * GRID_SIZE];
				break;
			case utils::terrain_mode::plain:
				heights[x + y * GRID_SIZE].terrain = TERRAIN_HEIGHT - 0.5;
				break;
			case utils::terrain_mode::crater:
				auto dist = distance(current, vec2(0, 0));
				if (dist < 1)
					heights[x + y * GRID_SIZE].terrain = perlin_noise_[x + y * GRID_SIZE]* dist;
				else
					heights[x + y * GRID_SIZE].terrain = perlin_noise_[x + y * GRID_SIZE];

				break;
			}

			//boundaries
			if (x == 0 || x == GRID_SIZE - 1 || y == 0 || y == GRID_SIZE - 1)
			{
				heights[x + y * GRID_SIZE].water = 0;
			}
			else
			{
				switch (s_->water)
				{
				case utils::water_mode::dam:
					if (distance(vec2(0, 0), vec2(int(x), int(y))) < GRID_SIZE / 4)
						heights[x + y * GRID_SIZE].water = WATER_HEIGHT;
					else
						heights[x + y * GRID_SIZE].water = 0;
					break;
				case utils::water_mode::free:
					heights[x + y * GRID_SIZE].water = WATER_HEIGHT;
					break;
				case utils::water_mode::nothing:
					heights[x + y * GRID_SIZE].water = 0;
					break;
				case utils::water_mode::pond:
					if (distance(current, vec2(0, 0)) < 0.5)
						heights[x + y * GRID_SIZE].water = WATER_HEIGHT / 4;
					else
						heights[x + y * GRID_SIZE].water = 0;
					break;
				}
			}
		}
	}
	glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);
}

vec2 motion::coords(int x1, int y1)
{
	return vec2((x1 - float(GRID_SIZE / 2)) / float(GRID_SIZE / 2), (y1 - float(GRID_SIZE / 2)) / float(GRID_SIZE / 2));
}

void simulation::motion::compute_normals()
{
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, height_ssbo_id_);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, normal_ssbo_id_);
	glBindBufferBase(GL_UNIFORM_BUFFER, glGetUniformBlockIndex(normals_shader_id_, "settings"), add_ubo_id_);

	glUseProgram(normals_shader_id_);
	glDispatchCompute(GRID_SIZE / 32, GRID_SIZE / 32, 1);
	glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
}