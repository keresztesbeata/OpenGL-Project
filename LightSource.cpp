#include "LightSource.hpp"

/* for debugging */
void printMatrix(glm::mat4 matrix);
void printVector(glm::vec3 v);

LightSource::LightSource(glm::vec3 lightDir, glm::vec3 lightColor) {
    this->transformationMatrix = glm::mat4(1.0f);
	this->lightDir = lightDir;
	this->lightColor = lightColor;
}
void LightSource::setTransformationMatrix(glm::mat4 newTransformationMatrix) {
    this->transformationMatrix = newTransformationMatrix;
    this->lightDir = glm::mat3(transformationMatrix) * lightDir;
}
void LightSource::setLightDir(glm::vec3 lightDir) {
	this->lightDir = lightDir;
}
void LightSource::setLightColor(glm::vec3 lightColor) {
	this->lightColor = lightColor;
}

glm::mat4 LightSource::getTransformationMatrix() {
    return this->transformationMatrix;
}
glm::vec3 LightSource::getLightDir() {
	return this->lightDir;
}
glm::vec3 LightSource::getLightColor() {
	return this->lightColor;
}

glm::mat4 LightSource::computeLightSpaceTrMatrix() {
    // Return the light-space transformation matrix
    glm::mat4 lightView = glm::lookAt(lightDir, glm::vec3(0), glm::vec3(0.0f, 1.0f, 0.0f));
    const GLfloat near_plane = 0.1f, far_plane = 100.0f;
    glm::mat4 lightProjection = glm::ortho(-100.0f, 100.0f, 0.0f, 100.0f, near_plane, far_plane);
    glm::mat4 lightSpaceTrMatrix = lightProjection * lightView;
    return lightSpaceTrMatrix;
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
        // todo: increase light intensity
        break;
    }
    case gps::MOVE_BACKWARD: {
        // todo: decrease light intensity
        break;
    }
    default: break;
    }
}

void LightSource::rotate(float angle, glm::vec3 axis) {
    // vertical rotation (around y axis)
    setTransformationMatrix(glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis));
}


void printMatrix(glm::mat4 matrix) {
    std::cout << "matrix = " << std::endl;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4; j++) {
            std::cout << matrix[i][j] << " ";
        }
        std::cout << std::endl;
    }
}

void printVector(glm::vec3 v) {
    std::cout << "v = (" << v.x << ", " << v.y << ", " << v.z << ");" << std::endl;
}
