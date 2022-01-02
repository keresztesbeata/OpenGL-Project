
#include <iostream>
#include "Camera.hpp"

#pragma once
class LightSource {
public:
    LightSource(glm::vec3 lightPosition, glm::vec3 lightTarget, glm::vec3 lightColor);
    void setTransformationMatrix(glm::mat4 newTransformationMatrix);
    void setLightPosition(glm::vec3 lightPosition);
    void setLightTarget(glm::vec3 lightTarget);
    void setLightColor(glm::vec3 lightColor);
    void setLightAttributes(float ambientStrength, float specularStrength);
    glm::mat4 getTransformationMatrix();
    glm::vec3 getLightDir();
    glm::vec3 getLightColor();
    glm::vec3 getLightTarget();
    glm::vec3 getLightPosition();
    float getAmbientStrength();
    float getSpecularStrength();
    void move(gps::MOVE_DIRECTION direction);
    glm::mat4 computeLightSpaceTrMatrix();

private:
    void rotate(float angle, glm::vec3 axis);

    // light parameters
    glm::mat4 transformationMatrix;
    glm::vec3 lightPosition;
    glm::vec3 lightTarget;
    glm::vec3 lightDir;
    glm::vec3 lightColor;
    float ambientStrength;
    float specularStrength;
};

