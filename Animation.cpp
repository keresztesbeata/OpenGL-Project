#include "Animation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

/* helper functions */
float dampedOscillation(float amplitude, float dampingFactorFactor, float oscillationFrequency, float time);
bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance);

/* helper functions for debugging */
void printMatrix(glm::mat4 matrix);
void printVector(glm::vec3 v);

Animation::Animation() {
	this->animationPlaying = false;
	this->animationStartTime = 0.0;
	this->animationSpeed = DEFAULT_ANIMATION_SPEED;
	this->initialPosition = glm::vec3(0.0, 0.0, 0.0);
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

bool Animation::isAnimationPlaying() {
	return animationPlaying;
}

void Animation::startBounceAnimation(float elasticity) {
	this->dampingFactor = 1.0 - elasticity;
	configureAnimation(BOUNCE);
	bounce(this->dampingFactor, animationSpeed);
}

void Animation::startSpinAnimation(glm::vec3 axis, float dampingFactor) {
	this->axis = axis;
	this->dampingFactor = dampingFactor;
	configureAnimation(SPIN);
	spin(this->axis, this->dampingFactor, animationSpeed);
}

void Animation::configureAnimation(ANIMATION_EFFECT newAnimationEffect) {
	animationPlaying = true;
	animationEffect = newAnimationEffect;
	animationStartTime = glfwGetTime();
}

void Animation::stopAnimation() {
	animationPlaying = false;
}

void Animation::playAnimation() {
	if (!animationPlaying) {
		return;
	}
	switch (animationEffect) {
		case BOUNCE: {
			bounce(this->dampingFactor, this->animationSpeed);
			break;
		}
		case SPIN: {
			spin(this->axis, this->dampingFactor, this->animationSpeed);
			break;
		}
		default: break;
	}
}

void Animation::bounce(float dampingFactor, float speed) {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float amplitude = BOUNCE_HEIGHT * (1.0 - dampingFactor);
	double oscillation = dampedOscillation(amplitude, dampingFactor, speed, elapsedTime);
	float prevY = currentPosition.y;
	float posY = UNIT_STEP * abs(oscillation);
	if (posY <= MIN_BOUNCE) {
		animationPlaying = false;
		posY = 0;
	}
	currentPosition.y = posY;
	transformationMatrix = glm::translate(transformationMatrix, glm::vec3(0.0, - prevY + posY, 0.0));
}

void Animation::spin(glm::vec3 axis, float dampingFactor, float speed) {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingCoefficient = std::exp(-dampingFactor * elapsedTime);
	GLfloat rotationAngle = elapsedTime * speed * dampingCoefficient;
	if (dampingCoefficient <= MAX_DAMPING) {
		animationPlaying = false;
	}
	transformationMatrix = glm::rotate(transformationMatrix, glm::radians(rotationAngle), axis);
}

/* helper functions */

float dampedOscillation(float amplitude, float dampingFactorFactor, float oscillationFrequency, float time) {
	return amplitude * std::exp(-dampingFactorFactor * time) * cos( PI * oscillationFrequency * glm::radians(time));
}

bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance) {
	return abs(newPosition.x - originalPosition.x) < minDistance &&
		abs(newPosition.z - originalPosition.z) < minDistance &&
		abs(newPosition.y - originalPosition.y) < minDistance;
}

/* for debugging */
void printMatrix(glm::mat4 matrix) {
	std::cout << "matrix = "<<std::endl;
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