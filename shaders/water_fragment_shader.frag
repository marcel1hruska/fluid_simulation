#version 430 core

in vec2 texture_coords;
in vec4 fragment_color;
out vec4 color;

uniform sampler2D reflection_texture;
uniform sampler2D refraction_texture;

void main(){

	vec4 reflect_color = texture(reflection_texture,texture_coords);
	vec4 refract_color = texture(refraction_texture,texture_coords);
	color = fragment_color;
	//color = mix(reflect_color,refract_color,fragment_color.a);
}