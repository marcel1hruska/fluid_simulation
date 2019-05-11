#version 430 core
#define GRID_SIZE 1024
layout(location = 0) in vec3 position;
layout(location = 1) uniform vec4 color;

// transform matrix
uniform mat4 transform_matrix;

out vec4 fragment_color;
void main(){
	fragment_color = color;
	gl_Position =  transform_matrix * vec4(position,1);
}