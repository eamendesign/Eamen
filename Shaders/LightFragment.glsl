#version 150
uniform sampler2D diffuseTex;
uniform vec3 cameraPos;
uniform vec4 lightColour;
uniform vec3 lightPos;
uniform float lightRadius;
uniform vec4 FogColour;
uniform int addFog;


in Vertex {
	vec2 texCoord;
	vec4 colour;
	vec3 normal;
	vec3 worldPos;
float visibility;
	
} IN;

out vec4 FragColor;

void main(void){
	vec4 diffuse = texture(diffuseTex, IN.texCoord);
	
	vec3 incident = normalize(lightPos - IN.worldPos);
	float lambert = max(0.0, dot(incident, IN.normal));
	
	float dist = length(lightPos - IN.worldPos);
	float atten = 1.0 - clamp(dist / lightRadius, 0.0, 1.0);
	
	vec3 viewDir = normalize(cameraPos - IN.worldPos);
	vec3 halfDir = normalize(incident = viewDir);
	
	float rFactor = max(0.0, dot(halfDir, IN.normal));
	float sFactor = pow(rFactor, 50.0);
	
	vec3 colour = (diffuse.rgb * lightColour.rgb);
	colour += (lightColour.rgb * sFactor) * 0.33;

	
	  
	if(addFog==1)
	{
		colour = vec4(colour * atten * lambert, diffuse.a).rgb;
		colour.rgb += (diffuse.rgb * lightColour.rgb) * 0.2;
		FragColor = mix(FogColour, vec4(colour,1.0), IN.visibility);
	}
	else
	{
		FragColor = vec4(colour * atten * lambert, diffuse.a);
	  	FragColor.rgb += (diffuse.rgb * lightColour.rgb) * 0.2;
	}
	
}