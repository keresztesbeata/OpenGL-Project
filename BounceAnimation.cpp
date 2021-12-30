#include "BounceAnimation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

/* helper functions */
float dampedOscillation(float amplitude, float dampingFactorFactor, float oscillationFrequency, float time);

BounceAnimation::BounceAnimation(Animation basicAnimation, float elasticity) {
	this->initialPosition = basicAnimation.getInitialPosition();
	this->currentPosition = basicAnimation.getCurrentPosition();
	this->transformationMatrix = basicAnimation.getTransformationMatrix();
	this->elasticity = elasticity;
	printVector(currentPosition);
	printMatrix(transformationMatrix);
}

void BounceAnimation::playAnimation() {
	bounce(elasticity, animationSpeed);
}

void BounceAnimation::bounce(float elasticity, float speed) {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingFactor = 1.0 - elasticity;
	float amplitude = BOUNCE_HEIGHT * (1.0 - dampingFactor);
	double oscillation = dampedOscillation(amplitude, dampingFactor, speed, elapsedTime);
	float prevY = currentPosition.y;
	float posY = UNIT_STEP * abs(oscillation);
	if (posY <= MIN_BOUNCE) {
		animationPlaying = false;
		posY = 0;
	}
	currentPosition.y = posY;
	transformationMatrix = glm::translate(transformationMatrix, glm::vec3(0.0, -prevY + posY, 0.0));
}

/* helper functions */
float dampedOscillation(float amplitude, float dampingFactorFactor, float oscillationFrequency, float time) {
	return amplitude * std::exp(-dampingFactorFactor * time) * cos(PI * oscillationFrequency * glm::radians(time));
}
