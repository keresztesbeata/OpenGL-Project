#version 410 core

in vec3 fPosition;
in vec3 fNormal;
in vec2 fTexCoords;
in vec4 fragPosLightSpace;

out vec4 fColor;

//matrices
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat3 normalMatrix;

//lighting
uniform vec3 lightDir;
uniform vec3 lightColor;

// textures
uniform sampler2D diffuseTexture;
uniform sampler2D specularTexture;
uniform sampler2D shadowMap; 

// skybox
uniform samplerCube skybox;

//components
vec3 ambient;
vec3 diffuse;
vec3 specular;

void computeDirLight()
{
    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);
    vec3 normalEye = normalize(normalMatrix * fNormal);

    //compute view direction (in eye coordinates, the viewer is situated at the origin
    vec3 viewDir = normalize(- fPosEye.xyz);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute ambient light
    ambient = ambientStrength * lightColor;

    //compute diffuse light
    diffuse = max(dot(normalEye, lightDirN), 0.0f) * lightColor;

    //compute specular light
    vec3 reflectDir = reflect(-lightDirN, normalEye);
    float specCoeff = pow(max(dot(viewDir, reflectDir), 0.0f), shininess);
    specular = specularStrength * specCoeff * lightColor;
}


float computeShadow() {
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

void main() 
{

    vec3 color;

    // do lighting calculations
    computeDirLight();

    float shadow = computeShadow();

    //compute eye space coordinates
    vec4 fPosEye = view * model * vec4(fPosition, 1.0f);

    //normalize light direction
    vec3 lightDirN = vec3(normalize(view * vec4(lightDir, 0.0f)));

    //compute view direction (in eye coordinates, the viewer is situated at the origin)
    vec3 viewDir = normalize(- fPosEye.xyz);

    float theta = dot(lightDirN, normalize(lightTarget.xyz-viewDir));

    if(theta < cutOffAngle) 
    { 
        ambient *= 0.1;
        diffuse *= 0.1;
        specular *= 0.1;
    }
   
    color  = min((ambient + (1.0 - shadow) * diffuse) * texture(diffuseTexture, fTexCoords).rgb + (1.0 - shadow) * specular * texture(specularTexture, fTexCoords).rgb, 1.0f);
    fColor = vec4(color, 1.0f);
}