#version 410 core

layout(location=0) in vec3 vPosition;
layout(location=1) in vec3 vNormal;
layout(location=2) in vec2 vTexCoords;

out vec3 fPosition;
out vec3 fNormal;
out vec2 fTexCoords;
out vec4 fragPosLightSpace;
out vec4 fragPosLightSpaceLeft;
out vec4 fragPosLightSpaceRight;
out vec4 fragPosLightSpaceMiddle;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceTrMatrix; 
uniform mat4 lightSpaceTrMatrixLeft; 
uniform mat4 lightSpaceTrMatrixRight; 
uniform mat4 lightSpaceTrMatrixMiddle;

void main() 
{
	gl_Position = projection * view * model * vec4(vPosition, 1.0f);
	fPosition = vPosition;
	fNormal = vNormal;
	fTexCoords = vTexCoords;
	fragPosLightSpace = lightSpaceTrMatrix * model * vec4(vPosition, 1.0f);
	fragPosLightSpaceLeft = lightSpaceTrMatrixLeft * model * vec4(vPosition, 1.0f);
	fragPosLightSpaceRight = lightSpaceTrMatrixRight * model * vec4(vPosition, 1.0f);
	fragPosLightSpaceMiddle = lightSpaceTrMatrixMiddle * model * vec4(vPosition, 1.0f);

}