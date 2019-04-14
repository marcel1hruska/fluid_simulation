#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;

// transform matrix
uniform  mat4 transform_matrix;

out vec2 texture_coords;
out vec4 fragment_color;
void main(){
	texture_coords = vec2(position.x,position.z);
	fragment_color = color;
	gl_Position =  transform_matrix * vec4(position,1);
}