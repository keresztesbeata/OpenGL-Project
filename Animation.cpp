#include "Animation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

/* helper functions */
float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time);
bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance);

/* helper functions for debugging */
void printMatrix(glm::mat4 matrix);
void printVector(glm::vec3 v);

Animation::Animation() {
	this->animationPlaying = false;
	this->animationStartTime = 0.0;
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
	this->animationSpeed = speed;
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
	this->elasticity = elasticity;
	configureAnimation(BOUNCE);
	bounce(this->elasticity, animationSpeed);
}

void Animation::startCircleAnimation(glm::vec3 center, float radius) {
	this->radius = radius;
	transformationMatrix = glm::translate(transformationMatrix, glm::vec3(radius, 0.0, 0.0));
	configureAnimation(CIRCLE);
	circle(this->center, this->radius, animationSpeed);
}

void Animation::startLookAroundAnimation(glm::vec3 center, float radius) {
	this->center = center;
	this->radius = radius;
	configureAnimation(CIRCLE);
	circle(this->center, this->radius, animationSpeed);
}

void Animation::configureAnimation(ANIMATION_EFFECT newAnimationEffect) {
	animationPlaying = true;
	animationEffect = newAnimationEffect;
	animationStartTime = glfwGetTime();
	//todo: delete!
	std::cout << "animation started : " << newAnimationEffect << std::endl;
}

void Animation::stopAnimation() {
	animationPlaying = false;
	std::cout << "initial = ";
	printVector(initialPosition);
}

void Animation::playAnimation() {
	if (!animationPlaying) {
		return;
	}
	switch (animationEffect) {
		case BOUNCE: {
			bounce(this->elasticity, this->animationSpeed);
			break;
		}
		case CIRCLE: {
			circle(this->center, this->radius, this->animationSpeed);
			break;
		}
		case LOOK_AROUND: {
			lookAround(this->center, this->radius, this->animationSpeed);
			break;
		}
		default: break;
	}
}

void Animation::bounce(float elasticity, float speed) {
	float amplitude = elasticity * BOUNCE_HEIGHT;
	float oscillationFreq = speed * DEFAULT_ANIMATION_SPEED;
	float elapsedTime = glfwGetTime() - animationStartTime;
	double oscillation = dampedOscillation(amplitude, elasticity, oscillationFreq, elapsedTime);
	float prevY = currentPosition.y;
	float posY = UNIT_STEP * abs(oscillation);
	if (posY < MIN_BOUNCE) {
		animationPlaying = false;
		posY = 0;
		std::cout << "animation stopped" << std::endl;
	}
	std::cout << "oscillation=" << oscillation << std::endl;
	currentPosition.y = posY;
	std::cout << "prev position=" << prevY << std::endl;
	std::cout << "curr position=" << posY << std::endl;
	transformationMatrix = glm::translate(transformationMatrix, glm::vec3(0.0, -prevY + posY, 0.0));
}

void Animation::circle(glm::vec3 center, float radius, float speed) {
	GLfloat timeAngle = glfwGetTime() * animationSpeed;
	//std::cout << abs(glm::radians(timeAngle)) << std::endl;
	/* todo : add stop condition after a full circle
	if () { 
		animationPlaying = false;
		std::cout << "animation stopped" << std::endl;
	}
	*/
	transformationMatrix = glm::rotate(glm::mat4(1.0f), glm::radians(timeAngle), glm::vec3(0, 1, 0));
}

void Animation::lookAround(glm::vec3 center, float radius, float speed) {
	float posX = sin(glfwGetTime()) * radius;
	float posZ = cos(glfwGetTime()) * radius;
	glm::vec3 newPosition = glm::vec3(posX, this->initialPosition.y, posZ);
	if (isBackToOriginalPosition(newPosition, initialPosition, MIN_DISPLACEMENT)) {
		animationPlaying = false;
		std::cout << "animation stopped" << std::endl;
	}
	std::cout << "new position =>";
	printVector(newPosition);
	transformationMatrix = glm::translate(transformationMatrix, center);
	currentPosition = glm::mat3(transformationMatrix) * newPosition;
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