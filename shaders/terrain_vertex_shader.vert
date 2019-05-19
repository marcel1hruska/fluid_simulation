#version 430 core
#define GRID_SIZE 1024
layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 height;

// transform matrix
uniform mat4 transform_matrix;

out vec4 fragment_color;
void main(){
	fragment_color = color;
	//total vertex height
	vec4 pos = vec4(position.x, height.y, position.z,1);
	gl_Position =  transform_matrix * pos;
}