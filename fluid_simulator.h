#include <GL/glew.h>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "utils/shader.h"
#include "utils/camera.h"

namespace simulation
{
	class fluid_simulator
	{
	public:
		int simulate();
	};
}