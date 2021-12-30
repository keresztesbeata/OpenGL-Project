#include "Animation.hpp"


Animation::Animation() {
	this->animationPlaying = false;
	this->animationStartTime = 0.0;
	this->animationSpeed = DEFAULT_ANIMATION_SPEED;
	this->initialPosition = glm::vec3(0.0, 0.0, 0.0);
	this->currentPosition = this->initialPosition;
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::setTransformationMatrix(glm::mat4 newTransformationMatrix) {
	this->transformationMatrix = newTransformationMatrix;
}

void Animation::setInitialPosition(glm::vec3 newPosition) {
	this->initialPosition = newPosition;
	this->currentPosition = this->initialPosition;
}

void Animation::setAnimationSpeed(float speed) {
	this->animationSpeed = speed * DEFAULT_ANIMATION_SPEED;
}

glm::mat4 Animation::getTransformationMatrix() {
	return this->transformationMatrix;
}

glm::vec3 Animation::getCurrentPosition() {
	return this->currentPosition;
}

glm::vec3 Animation::getInitialPosition() {
	return this->initialPosition;
}

bool Animation::isAnimationPlaying() {
	return animationPlaying;
}

void Animation::startAnimation() {
	animationPlaying = true;
	animationStartTime = glfwGetTime();
	playAnimation();
}

void Animation::stopAnimation() {
	animationPlaying = false;
}

void Animation::playAnimation() {
}



/* helper functions */

bool Animation::isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance) {
	return abs(newPosition.x - originalPosition.x) < minDistance &&
		abs(newPosition.z - originalPosition.z) < minDistance &&
		abs(newPosition.y - originalPosition.y) < minDistance;
}

/* for debugging */
void Animation::printMatrix(glm::mat4 matrix) {
	std::cout << "matrix = "<<std::endl;
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			std::cout << matrix[i][j] << " ";
		}
		std::cout << std::endl;
	}
}

void Animation::printVector(glm::vec3 v) {
	std::cout << "v = (" << v.x << ", " << v.y << ", " << v.z << ");" << std::endl;
}