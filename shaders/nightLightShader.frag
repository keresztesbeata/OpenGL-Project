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

// positions
uniform vec3 nightLightPosition0;
uniform vec3 nightLightPosition1;
uniform vec3 nightLightPosition2;
uniform vec3 nightLightPosition3;
uniform vec3 nightLightPosition4;
uniform vec3 nightLightPosition5;

// targets
uniform vec3 nightLightTarget0;
uniform vec3 nightLightTarget1;
uniform vec3 nightLightTarget2;
uniform vec3 nightLightTarget3;
uniform vec3 nightLightTarget4;
uniform vec3 nightLightTarget5;

// reflector lights
// positions
uniform vec3 reflectorLightPosition0;
uniform vec3 reflectorLightPosition1;
uniform vec3 reflectorLightPosition2;
uniform vec3 reflectorLightPosition3;

// targets
uniform vec3 reflectorLightTarget0;
uniform vec3 reflectorLightTarget1;
uniform vec3 reflectorLightTarget2;
uniform vec3 reflectorLightTarget3;

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

	vec3 resultColor = computeSpotLight(nightLightPosition0, nightLightTarget0, lightColor);
	resultColor += computeSpotLight(nightLightPosition1, nightLightTarget1, lightColor);
	resultColor += computeSpotLight(nightLightPosition2, nightLightTarget2, lightColor);
	resultColor += computeSpotLight(nightLightPosition3, nightLightTarget3, lightColor);
	resultColor += computeSpotLight(nightLightPosition4, nightLightTarget4, lightColor);
	resultColor += computeSpotLight(nightLightPosition5, nightLightTarget5, lightColor);

	resultColor += computeSpotLight(reflectorLightPosition0, reflectorLightTarget0, lightColor);
	resultColor += computeSpotLight(reflectorLightPosition1, reflectorLightTarget1, lightColor);
	resultColor += computeSpotLight(reflectorLightPosition2, reflectorLightTarget2, lightColor);
	resultColor += computeSpotLight(reflectorLightPosition3, reflectorLightTarget3, lightColor);

	resultColor.x = min(resultColor.x, 1.0);
	resultColor.y= min(resultColor.y, 1.0);
	resultColor.z = min(resultColor.z, 1.0);

    fColor = vec4(resultColor, 1.0f);
}