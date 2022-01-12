#version 410 core

in vec3 textureCoordinates;

out vec4 color;

uniform samplerCube skybox;

uniform float ambientStrength;

void main()
{
     color = ambientStrength * texture(skybox, textureCoordinates);
}
