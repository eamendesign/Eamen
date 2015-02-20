#version 150

uniform mat4 modelMatrix;
uniform mat4 viewMatrix;
uniform mat4 projMatrix;
uniform mat4 textureMatrix;

in vec3 position;

out Vertex {
	vec3 normal;
	float visibility;
} OUT;

const float density = 0.003;
const float gradient = 1.5;

void main(void) {
	vec3 tempPos = position - vec3(0,0,1);
	OUT.normal = transpose(mat3(viewMatrix)) * normalize(tempPos);
	vec4 positionRelativeToCam = viewMatrix * modelMatrix * vec4(position, 1.0);
	gl_Position = projMatrix * vec4(tempPos, 1.0);
	
	float distance = length(positionRelativeToCam.xyz);
	OUT.visibility = exp(-pow(distance*density, gradient));
	OUT.visibility = clamp(OUT.visibility, 0.0, 1.0);
}