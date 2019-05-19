#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec4 color;
layout(location = 2) in vec2 height;

// transform matrix
uniform  mat4 transform_matrix;

out vec2 texture_coords;
out vec4 fragment_color;
void main(){
	texture_coords = vec2(position.x,position.z);

	//total vertex height as a result of terrain height + water height
	vec4 pos = vec4(position.x, height.x + height.y, position.z,1);

	//hide water with too low height values
	float alpha = color.w;
	if (height.x < 0.001)
		alpha = 0.0;
	//color accouring to Z, the higher the water the lighter the color
	fragment_color = vec4(height.x*5,height.x*5,color.z,alpha);

	gl_Position =  transform_matrix * pos;
}