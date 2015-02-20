#version 150
uniform samplerCube cubeTex;
uniform vec3 cameraPos;
uniform vec4 FogColour;
uniform int addFog;

in Vertex {
	vec3 normal;
float visibility;
} IN;

out vec4 FragColor;

void main(void){
	int a = addFog;

	if(a==1)
	{
		vec4 colour = texture(cubeTex, normalize(IN.normal));
		FragColor = mix(FogColour, colour, IN.visibility);
	}
	else
	FragColor = texture(cubeTex, normalize(IN.normal));
	
	
}