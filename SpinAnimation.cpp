#include "SpinAnimation.hpp"

SpinAnimation::SpinAnimation(Animation basicAnimation, glm::vec3 axis, float dampingFactor) {
	this->initialPosition = basicAnimation.getInitialPosition();
	this->currentPosition = basicAnimation.getCurrentPosition();
	this->transformationMatrix = basicAnimation.getTransformationMatrix();
	this->axis = axis;
	this->dampingFactor = dampingFactor;
}

void SpinAnimation::playAnimation() {
	spin(axis, dampingFactor, animationSpeed);
}

void SpinAnimation::spin(glm::vec3 axis, float dampingFactor, float speed) {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingCoefficient = std::exp(-dampingFactor * elapsedTime);
	GLfloat rotationAngle = elapsedTime * speed * dampingCoefficient;
	if (dampingCoefficient <= MAX_DAMPING) {
		animationPlaying = false;
	}
	transformationMatrix = glm::rotate(transformationMatrix, glm::radians(rotationAngle), axis);
}