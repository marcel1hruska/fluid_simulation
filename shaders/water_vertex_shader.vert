#version 430 core

layout(location = 0) in vec3 position;
layout(location = 1) in vec2 height;
layout(location = 2) in vec4 normal;
// transform matrix
uniform mat4 transform_matrix;

out vec2 texture_coords;
out vec4 fragment_color;
out vec3 norm;
out vec4 pos;

const vec3 water_color = vec3(0.653,0.78,0.954);

void main(){
	texture_coords = vec2(position.x,position.z);
	//total vertex height as a result of terrain height + water height
	pos = vec4(position.x, height.x + height.y, position.z,1);

	//normal.w constains info whether the vertex should be drawn or not (from normals compute shader)
	float alpha;

	if (normal.w == 0)
		alpha = 0.0;
	else
		alpha = 0.4;
	//color accouring to Z, the higher the water the lighter the color
	fragment_color = vec4(water_color,alpha);

	gl_Position =  transform_matrix * pos;
	norm = normal.xyz;
}