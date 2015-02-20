#version 150 core

uniform sampler2D diffuseTex;

in Vertex	{
	vec4 colour;
	vec2 texCoord;
} IN;

out vec4 FragColor;

void main(void)	{
	FragColor = vec4(0.5,0.5,0.5,0.5);
}
