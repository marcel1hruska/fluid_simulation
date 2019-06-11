#version 430 core
layout(location = 0) in vec3 position;
layout(location = 1) in vec2 height;

// transform matrix
uniform mat4 transform_matrix;

out float terrain_height;
out vec2 texture_pos;

void main(){
	//terrain vertex height
	vec4 pos = vec4(position.x, height.y, position.z,1);
	gl_Position =  transform_matrix * pos;

	texture_pos = vec2(position.x,position.z);
	terrain_height = height.y;
}