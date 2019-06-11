#version 430 core

in vec2 texture_coords;
in vec4 fragment_color;
in vec3 norm;
in vec4 pos;
out vec4 color;

const vec4 light_color = vec4(1,1,1,1);
const float shininess = 128;
//uniform sampler2D reflection_texture;
//uniform sampler2D refraction_texture;

uniform vec3 light_pos;  
uniform vec3 camera_pos;

void main(){
	if (pos.x < -0.995 || pos.x > 0.995 || pos.z < -0.995 || pos.z > 0.995 || fragment_color.a == 0.0)
		color = fragment_color;
	else
	{
		 // ambient
		vec4 ambient_color = light_color * fragment_color;
  	
		// diffuse 
		vec3 norm = -normalize(norm);
		vec3 light_dir = normalize(light_pos - pos.xyz);
		float diff_el = max(dot(norm, light_dir), 0.0);
		vec4 diffuse_color = light_color * diff_el *  fragment_color;
    
		// specular
		vec3 view_dir = normalize(camera_pos - pos.xyz);
		vec3 reflect_dir = reflect(-light_dir, norm);  
		float spec_el = pow(max(dot(view_dir, reflect_dir), 0.0), shininess);
		vec4 specular_color = light_color * spec_el;  
        
		color = ambient_color + diffuse_color + specular_color;
	}
	//vec4 reflect_color = texture(reflection_texture,texture_coords);
	//vec4 refract_color = texture(refraction_texture,texture_coords);
	//color = mix(reflect_color,refract_color,0.5);
}