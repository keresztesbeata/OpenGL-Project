#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat3 normalMatrix;
uniform vec3 cameraPos;

uniform vec3 lightColor;
uniform float cutOffAngle;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap; 
// skybox
uniform samplerCube skybox;

// position
uniform vec3 lightPosition;
// target
uniform vec3 spotLightTarget;

//components
float ambientStrength = 0.85f;
float specularStrength = 0.25f;
float ambientStrengthPointLight = 1.0f;
float shininess = 52.0f;

//attenuation of light
float constant = 0.2f; 
float linear = 0.00045f; 
float quadratic = 0.00045f;
// spotlight's outercone's angle given in cos
float outerCone = 0.45;

vec3 computePointLight(vec3 lightPosition, vec3 lightColor)
{
	vec3 ambient;
	vec3 diffuse;
	vec3 specular;

    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec4 lightPosEye = view * model * vec4(lightPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

	//compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDirN = normalize(- fPosEye.xyz);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightPosEye.xyz - fPosEye.xyz, 0.0f)));

    //compute distance to light 
	float dist = length(lightPosEye.xyz - fPosEye.xyz); 

	//compute attenuation 
	float att = 1.0f / (constant + linear * dist + quadratic * (dist * dist));

	//compute half vector 
	vec3 halfVector = normalize(lightDirN + viewDirN);

	//compute ambient light
	ambient = att * ambientStrengthPointLight * lightColor;

	//compute diffuse light
	diffuse = att * max(dot(normalEye, lightDirN), 0.0f) * lightColor;

	//compute specular light 
	float specCoeff = pow(max(dot(normalEye, halfVector), 0.0f), shininess); 
	specular = att * specularStrength * specCoeff * lightColor;

    //compute final vertex color 
    vec3 color = min((ambient + diffuse) * texture(diffuseTexture, fTexCoords).rgb + specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    
	return color;
}


vec3 computeSpotLight(vec3 lightPosition, vec3 spotLightTarget, vec3 lightColor) {
	//compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
	vec4 lightPositionEye = view * model * vec4(lightPosition, 1.0f);
	vec4 spotLightTargetEye = view * model * vec4(spotLightTarget, 1.0f);

	 //normalize light direction
    vec4 lightDirN = normalize(lightPositionEye - fPosEye);

    float theta = dot(lightDirN, normalize(lightPositionEye - spotLightTargetEye));

	vec3 color = computePointLight(lightPosition, lightColor);

	 if(theta > cutOffAngle) {
        return color;
    }
	else{
		float epsilon = cutOffAngle - outerCone;
		float intensity = (theta - outerCone)/epsilon;
		return intensity * color;
	}
}

void main() 
{

	vec3 color = computeSpotLight(lightPosition, spotLightTarget, lightColor);
	
    fColor = vec4(color, 1.0f);
}