#version 330 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;

// transform matrix
uniform mat4 transform_matrix;

out vec3 fragment_color;

void main(){	
	fragment_color = color;
	gl_Position =  transform_matrix * vec4(position,1);
}