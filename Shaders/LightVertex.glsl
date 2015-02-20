#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;
in vec2 texCoord;
in vec4 colour;
in vec3 normal;

out Vertex {
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
	vec3 worldPos;
	float visibility;

} OUT;

const float density = 0.001;
const float gradient = 1.5;

void main(void) {
	OUT.colour = colour;
	OUT.texCoord = texCoord;
	mat3 normalMatrix = transpose(inverse(mat3(modelMatrix)));
	OUT.normal = normalize(normalMatrix * normalize(normal));
	
	vec4 positionRelativeToCam = viewMatrix * modelMatrix * vec4(position, 1.0);

	gl_Position = projMatrix * viewMatrix * modelMatrix * vec4(position, 1.0);

	float distance = length(positionRelativeToCam.xyz);
	OUT.visibility = exp(-pow(distance*density, gradient));
	OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
}