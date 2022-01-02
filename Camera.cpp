#include "Camera.hpp"

namespace gps {

    //Camera constructor
    Camera::Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp) {
        this->cameraPosition = cameraPosition;
        this->cameraTarget = cameraTarget;
        this->cameraUpDirection = cameraUp;

        this->cameraFrontDirection = glm::normalize(cameraPosition - cameraTarget);
        this->cameraRightDirection = glm::normalize(glm::cross(this->cameraUpDirection, this->cameraFrontDirection));
    }

    //return the view matrix, using the glm::lookAt() function
    glm::mat4 Camera::getViewMatrix() {
        return glm::lookAt(cameraPosition, cameraPosition + cameraFrontDirection, cameraUpDirection);
    }
    // get camera position
    glm::vec3 Camera::getCameraPosition() {
        return this->cameraPosition;
    }
    // get the camera target
    glm::vec3 Camera::getCameraTarget() {
        return this->cameraTarget;
    }
    // set the camera position
    void Camera::setCameraPosition(glm::vec3 newCameraPosition) {
        this->cameraPosition = newCameraPosition;
    }
    // set the camera target
    void Camera::setCameraTarget(glm::vec3 newCameraTarget) {
        this->cameraTarget = newCameraTarget;
    }
    // set the camera movement's speed
    void Camera::setCameraSpeed(float speed) {
        this->cameraSpeed = speed;
    }

    //update the camera internal parameters following a camera move event
    void Camera::move(MOVE_DIRECTION direction) {
        switch (direction) {
        case MOVE_LEFT: {
            this->cameraPosition -= this->cameraRightDirection * cameraSpeed;
            break;
        }
        case MOVE_RIGHT: {
            this->cameraPosition += this->cameraRightDirection * cameraSpeed;
            break;
        }
        case MOVE_FORWARD: {
            this->cameraPosition += this->cameraFrontDirection * cameraSpeed;
            break;
        }
        case MOVE_BACKWARD: {
            this->cameraPosition -= this->cameraFrontDirection * cameraSpeed;
            break;
        }
        case MOVE_UP: {
            this->cameraPosition += this->cameraUpDirection * cameraSpeed;
            break;
        }
        case MOVE_DOWN: {
            this->cameraPosition -= this->cameraUpDirection * cameraSpeed;
            break;
        }
        default: break;
        }
    }

    //update the camera internal parameters following a camera rotate event
    //yaw - camera rotation around the y axis
    //pitch - camera rotation around the x axis
    void Camera::rotate(float pitch, float yaw) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
        direction.y = sin(glm::radians(pitch));
        direction.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
        this->cameraFrontDirection = glm::normalize(direction);
    }

    // roll camera with a given angle
    void Camera::roll(float rollAngle) {
        glm::vec3 direction;
        direction.x = cos(glm::radians(rollAngle));
        direction.y = sin(glm::radians(rollAngle));
        direction.z = cameraRightDirection.z;
        this->cameraRightDirection = direction;
        this->cameraUpDirection = glm::normalize(glm::cross(cameraFrontDirection, this->cameraRightDirection));
    }
}