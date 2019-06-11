#version 430 core

in float terrain_height;
in vec2 texture_pos;

// textures for terrain
uniform sampler2D sand; 
uniform sampler2D grass; 
uniform sampler2D rock; 
uniform sampler2D forest; 

out vec4 color;

// blend textures together, smoother transmition
vec4 blend_textures(float terrain_height, float min_val, float max_val, vec4 first, vec4 second) {
	float ratio = (terrain_height - min_val) / (max_val-min_val);
	return mix(first, second, ratio);
}

void main(){
	//different textures for different biomes according to height
	vec4 sand_color  = texture(sand, texture_pos); 
	vec4 grass_color = texture(grass, texture_pos); 
	vec4 forest_color = texture(forest, texture_pos);
	vec4 rock_color = texture(rock, texture_pos);
	
	// sand -> sand/grass -> grass -> grass/forest -> forest -> forest/rocks -> rocks
	if (terrain_height < 0.1) {
		color = sand_color;
	} else if (terrain_height < 0.2) {
		color = blend_textures(terrain_height, 0.1, 0.2, sand_color, grass_color);
	} else if (terrain_height < 0.3) {
		color = grass_color;
	} else if (terrain_height < 0.4) {
		color = blend_textures(terrain_height, 0.3, 0.4, grass_color, forest_color);
	} else if (terrain_height < 0.5) {
		color = forest_color;
	} else if (terrain_height < 0.6) {
		color = blend_textures(terrain_height, 0.5, 0.6, forest_color, rock_color);
	} else {
		color = rock_color;
	}
}