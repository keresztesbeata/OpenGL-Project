#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform vec3 cameraPos;

uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec3 spotLightTarget;
uniform float cutOffAngle;

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
// spotlight's outercone's angle given in cos
float outerCone = 0.99;

float computeShadow(vec4 fragPosLightSpace) {

    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);
	
	vec3 lightDir = normalize(spotLightTarget - lightPosition);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

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

vec3 computeDirLight(vec3 lightDir, vec3 lightColor, vec4 fragPosLightSpace)
{

	float shadow = computeShadow(fragPosLightSpace); 

	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    specular = specularStrength * specCoeff * lightColor;

	//compute final vertex color 
    vec3 color = min((ambient + (1.0 - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0 - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    
	return color;
}

vec3 computeSpotLight(vec3 lightPosition, vec3 lightColor, vec4 fragPosLightSpace) {
	//compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    //compute view direction (in eye coordinates, the viewer is situated at the origin)
    vec3 viewDirN = normalize(- fPosEye.xyz);

	vec3 lightDir = normalize(spotLightTarget - lightPosition);

	//normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

	// compute the teta angle's cosine (using the dot product between the direction from the fragment to the light source and the spotlight's orientation)
    float theta = dot(lightDirN, normalize(spotLightTarget - viewDirN));
	
	vec3 color = computeDirLight(lightDir, lightColor, fragPosLightSpace);
    
	 if(theta > cutOffAngle) {
        return color;
    }
	else{
		float epsilon = (cutOffAngle - outerCone);
		float intensity = (theta - outerCone)/epsilon;
		return intensity * color;
	}
  }

void main() 
{

	vec3 resultColor = computeSpotLight(lightPosition, lightColor, fragPosLightSpace);

    fColor = vec4(resultColor, 1.0f);
}