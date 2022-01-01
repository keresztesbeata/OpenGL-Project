#include "Animation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

// helper functions
float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time);
bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance);
// for debugging
void printVector(glm::vec3 v);
void printMatrix(glm::mat4 matrix);

Animation::Animation(float elasticity, glm::vec3 initialPosition, float animationSpeed) {
	initAnimation(elasticity, initialPosition, animationSpeed);
}

Animation::Animation(float elasticity, glm::vec3 initialPosition) {
	initAnimation(elasticity, initialPosition, 1.0);
}

void Animation::initAnimation(float elasticity, glm::vec3 initialPosition, float animationSpeed) {
	this->animationPlaying = false;
	this->animationStartTime = 0.0;
	this->spinAxis = glm::vec3(0, 1, 0); // y axis
	setAnimationSpeed(animationSpeed);
	this->initialPosition = glm::vec3(0.0, 0.0, 0.0);
	this->currentPosition = this->initialPosition;
	this->elasticity = elasticity;
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::setTransformationMatrix(glm::mat4 newTransformationMatrix) {
	this->transformationMatrix = newTransformationMatrix;
	this->initialPosition = glm::mat3(transformationMatrix) * this->initialPosition;
	this->currentPosition = glm::mat3(transformationMatrix) * this->currentPosition;
}

void Animation::setInitialPosition(glm::vec3 newPosition) {
	this->initialPosition = newPosition;
	this->currentPosition = this->initialPosition;
}

void Animation::setAnimationSpeed(float speed) {
	this->animationSpeed = speed * DEFAULT_ANIMATION_SPEED;
}

void Animation::setElasticity(float elasticity) {
	this->elasticity = elasticity;
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

void Animation::startAnimation(ANIMATION_TYPE animationType) {
	this->animationPlaying = true;
	this->currentAnimation = animationType;
	this->transformationMatrix = glm::mat4(1.0);
	this->animationStartTime = glfwGetTime();

	if (animationType == BOUNCE_ANIMATION) {
		startBounceAnimation();
	}
	playAnimation();
}

void Animation::startBounceAnimation() {
	this->firstDrop = true;
}

void Animation::stopAnimation() {
	animationPlaying = false;
}

void Animation::playAnimation() {
	if (!animationPlaying) {
		return;
	}
	switch (currentAnimation) {
		case BOUNCE_ANIMATION: {
			bounce();
			break;
		}
		case SPIN_ANIMATION: {
			spin(this->spinAxis);
			break;
		}
		case THROW_ANIMATION: {
			//TODO
			break;
		}
		case ROLL_ANIMATION: {
			//TODO
			break;
		}
		default: break;
	}
	//std::cout << "current animation = " << currentAnimation << "curr pos = ";
	//printVector(currentPosition);
	//std::cout << "current matrix = ";
	//printMatrix(transformationMatrix);
}


/* bounce animation */
void Animation::bounce() {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingFactor = 1.0 - elasticity;
	float amplitude = BOUNCE_HEIGHT * (1.0 - dampingFactor);
	double oscillation = dampedOscillation(amplitude, dampingFactor, animationSpeed, elapsedTime);
	float prevY = currentPosition.y;
	float posY = UNIT_STEP * abs(oscillation);
	if (posY <= MIN_BOUNCE) {
		animationPlaying = false;
		posY = 0;
	}
	currentPosition.y = posY;
	transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0.0, posY, 0.0));
}

void Animation::spin(glm::vec3 axis) {
	float dampingFactor = 1.0 - this->elasticity;
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingCoefficient = std::exp(-dampingFactor * elapsedTime);
	GLfloat rotationAngle = elapsedTime * this->animationSpeed * dampingCoefficient;
	if (dampingCoefficient <= MAX_DAMPING) {
		animationPlaying = false;
	}
	this->transformationMatrix = glm::rotate(this->transformationMatrix, glm::radians(rotationAngle), axis);
}

/* helper functions */

float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time) {
	return amplitude * std::exp(-dampingFactor * time) * cos(PI * oscillationFrequency * glm::radians(time));
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
