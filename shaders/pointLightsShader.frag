#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;
in vec4 fragPosLightSpaceLeft;
in vec4 fragPosLightSpaceRight;
in vec4 fragPosLightSpaceMiddle;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;

uniform vec3 leftPointLightDir;
uniform vec3 leftPointLightColor;

uniform vec3 rightPointLightDir;
uniform vec3 rightPointLightColor;

uniform vec3 middlePointLightDir;
uniform vec3 middlePointLightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap; 

// skybox
uniform samplerCube skybox;

//components
float ambientStrength = 0.45f;
float specularStrength = 0.75f;
float ambientStrengthPointLight = 0.8f;
float shininess = 32.0f;

//attenuation of light
float constant = 0.1f; 
float linear = 0.0045f; 
float quadratic = 0.0035f;

float computeShadow(vec4 fragPosLightSpace, vec3 lightDir) {
	// perform perspective divide 
	vec3 normalizedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	// Transform to [0,1] range 
	normalizedCoords = normalizedCoords * 0.5 + 0.5; 

	// Get closest depth value from light's perspective 
	float closestDepth = texture(shadowMap, normalizedCoords.xy).r;

	// Get depth of current fragment from light's perspective 
	float currentDepth = normalizedCoords.z;
	
    if (currentDepth > 1.0f) 
		return 0.0f; 

    float bias = max(0.05 * (1.0 - dot(fNormal, lightDir)), 0.005);  
	
    // Check whether current frag pos is in shadow 
	float shadow = currentDepth - bias > closestDepth  ? 1.0 : 0.0;

	return shadow;
}

vec3 computePointLight(vec3 lightDir, vec3 lightColor, vec4 fragPosLightSpace)
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

	float shadow = computeShadow(fragPosLightSpace, lightDir); 

	vec3 cameraPosEye = vec3(0.0f);//in eye coordinates, the viewer is situated at the origin
   
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    vec3 normalEye = normalize(normalMatrix * fNormal);

	//compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDirN = normalize(- fPosEye.xyz);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute distance to light 
	float dist = length(cameraPosEye - fPosEye.xyz); 

	//compute attenuation 
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//compute half vector 
	vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute ambient light
	ambient = att * ambientStrengthPointLight * lightColor;

	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//compute specular light 
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), 32); 
	specular = att * specularStrength * specCoeff * lightColor;

    //compute final vertex color 
    vec3 color = min((ambient + (1.0 - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0 - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    
	return color;
}

void main() 
{

	vec3 resultColor = computePointLight(leftPointLightDir, leftPointLightColor, fragPosLightSpaceLeft);

	resultColor += computePointLight(middlePointLightDir, middlePointLightColor, fragPosLightSpaceMiddle);

	resultColor += computePointLight(rightPointLightDir, rightPointLightColor, fragPosLightSpaceRight);

    fColor = vec4(resultColor, 1.0f);
}