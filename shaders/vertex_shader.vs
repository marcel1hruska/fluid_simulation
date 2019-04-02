#version 330 core

// Input data
layout(location = 0) in vec3 pos;

// transform matrix
uniform mat4 transform_matrix;

void main(){	
	// output position as result of model position and transform matrix
	gl_Position =  transform_matrix * vec4(pos,1);

}