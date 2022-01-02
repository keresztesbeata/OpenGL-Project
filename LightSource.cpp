#include "LightSource.hpp"

LightSource::LightSource(glm::vec3 lightPosition, glm::vec3 lightTarget, glm::vec3 lightColor) {
    this->transformationMatrix = glm::mat4(1.0f);
    this->lightTarget = lightTarget;
    this->lightPosition = lightPosition;
    this->lightDir = lightTarget - lightPosition;
	this->lightColor = lightColor;
    this->cutOffAngle = 180;
    this->ambientStrength = 1.0;
    this->specularStrength = 0.0;
    this->shininess = 0.0;
}
void LightSource::setTransformationMatrix(glm::mat4 newTransformationMatrix) {
    this->transformationMatrix = newTransformationMatrix;
    this->lightDir = glm::mat3(transformationMatrix) * lightDir;
}
void LightSource::setLightPosition(glm::vec3 lightPosition) {
    this->lightPosition = lightPosition;
}

void LightSource::setLightTarget(glm::vec3 lightTarget) {
    this->lightTarget = lightTarget;
    this->lightDir = lightTarget - lightPosition;
}

void LightSource::setLightColor(glm::vec3 lightColor) {
	this->lightColor = lightColor;
}

void LightSource::setCutOffAngle(float cutOffAngle) {
    this->cutOffAngle = cutOffAngle;
}

glm::mat4 LightSource::getTransformationMatrix() {
    return this->transformationMatrix;
}
glm::vec3 LightSource::getLightDir() {
	return this->lightDir;
}

glm::vec3 LightSource::getLightPosition() {
    return this->lightPosition;
}
glm::vec3 LightSource::getLightColor() {
	return this->lightColor;
}

float LightSource::getCutOffAngle() {
    return this->cutOffAngle;
}

float LightSource::getAmbientStrength() {
    return this->ambientStrength;
}
float LightSource::getSpecularStrength() {
    return this->specularStrength;
}
float LightSource::getShininess() {
    return this->shininess;
}

glm::vec3 LightSource::getLightTarget() {
    return this->lightTarget;
}


glm::mat4 LightSource::computeLightSpaceTrMatrix() {
    // Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, 0.0f, 100.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
}

void LightSource::setLightAttributes(float ambientStrength, float specularStrength, float shininess) {
    this->ambientStrength = ambientStrength;
    this->specularStrength = specularStrength;
    this->shininess = shininess;
}

void LightSource::move(gps::MOVE_DIRECTION direction) {
    switch (direction) {
    case gps::MOVE_LEFT: {
        rotate(-1.0, glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }
    case gps::MOVE_RIGHT: {
        rotate(1.0, glm::vec3(0.0f, 0.0f, 1.0f));
        break;
    }
    case gps::ROTATE_CLOCKWISE: {
        rotate(-1.0, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    }
    case gps::ROTATE_COUNTER_CLOCKWISE: {
        rotate(1.0, glm::vec3(0.0f, 1.0f, 0.0f));
        break;
    }
    case gps::MOVE_UP: {
        rotate(-1.0, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    }
    case gps::MOVE_DOWN: {
        rotate(1.0, glm::vec3(1.0f, 0.0f, 0.0f));
        break;
    }
    case gps::MOVE_FORWARD: {
        if (ambientStrength < 1.0) {
            ambientStrength += 0.1;
        }
        break;
    }
    case gps::MOVE_BACKWARD: {
        if (ambientStrength > 0) {
            ambientStrength -= 0.1;
        }
        break;
    }
    default: break;
    }
}

void LightSource::rotate(float angle, glm::vec3 axis) {
    // vertical rotation (around y axis)
    setTransformationMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis));
}

