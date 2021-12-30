
#include <iostream>
#include "Camera.hpp"

#pragma once
class LightSource {
public:
    LightSource(glm::vec3 lightDir, glm::vec3 lightColor);
    void setTransformationMatrix(glm::mat4 newTransformationMatrix);
    void setLightDir(glm::vec3 lightDir);
    void setLightColor(glm::vec3 lightColor);
    glm::mat4 getTransformationMatrix();
    glm::vec3 getLightDir();
    glm::vec3 getLightColor();
    void move(gps::MOVE_DIRECTION direction);

private:
    void rotate(float angle, glm::vec3 axis);

    // light parameters
    glm::mat4 transformationMatrix;
    glm::vec3 lightDir;
    glm::vec3 lightColor;
    // todo: light intensity / shininess
};

