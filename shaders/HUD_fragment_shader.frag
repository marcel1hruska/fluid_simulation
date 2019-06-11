#version 430 core
in vec2 coords;
out vec4 color;

uniform sampler2D text;

void main()
{    
    vec4 sampled = vec4(1.0, 1.0, 1.0, texture(text, coords).r);
    color = vec4(0.5, 0.8, 0.2, 1.0) * sampled;
}  