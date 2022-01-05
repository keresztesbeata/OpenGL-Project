#include "Animation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

// helper functions
float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time);
bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance);

// for debugging
void printVector(glm::vec3 v);
void printMatrix(glm::mat4 matrix);

Animation::Animation(float elasticity, float weight, glm::vec3 initialPosition, float animationSpeed) {
	initAnimation(elasticity, weight, initialPosition, animationSpeed);
}

Animation::Animation(float elasticity, float weight, glm::vec3 initialPosition) {
	initAnimation(elasticity, weight, initialPosition, 1.0);
}

void Animation::initAnimation(float elasticity, float weight, glm::vec3 initialPosition, float animationSpeed) {
	this->animationPlaying = false;
	this->animationStartTime = 0.0;
	this->spinAxis = glm::vec3(0, 1, 0); // y axis
	setAnimationSpeed(animationSpeed);
	this->initialPosition = glm::vec3(0.0, 0.0, 0.0);
	this->currentPosition = this->initialPosition;
	this->targetPosition = this->currentPosition;
	this->elasticity = elasticity;
	this->pitch = 0.0;
	this->yaw = 0.0;
	this->weight = weight;
	this->transformationMatrix = glm::mat4(1.0);
	this->isBallPickedUp = false;
}

void Animation::setInitialPosition(glm::vec3 newPosition) {
	this->initialPosition = newPosition;
	this->currentPosition = newPosition;
}

void Animation::setTargetPosition(glm::vec3 newPosition) {
	this->targetPosition = newPosition;
}

void Animation::setAnimationSpeed(float speed) {
	this->animationSpeed = speed * DEFAULT_ANIMATION_SPEED;
}

glm::mat4 Animation::getTransformationMatrix() {
	glm::mat4 moveBackToInitialPosition = glm::translate(glm::mat4(1.0), initialPosition);
	return moveBackToInitialPosition * this->transformationMatrix;
}

glm::vec3 Animation::getCurrentPosition() {
	return this->currentPosition;
}

bool Animation::isAnimationPlaying() {
	return animationPlaying;
}

void Animation::startAnimation(ANIMATION_TYPE animationType) {
	if (animationPlaying) {
		// a previous animation is already running
		return;
	}
	this->animationPlaying = true;
	this->currentAnimation = animationType;
	this->transformationMatrix = glm::mat4(1.0);
	this->animationStartTime = glfwGetTime();
	this->teta = 0;
	playAnimation();
}

void Animation::stopAnimation() {
	animationPlaying = false;
	if (currentAnimation == BOUNCE_ANIMATION || currentAnimation == THROW_ANIMATION || currentAnimation == ROLL_ANIMATION) {
		isBallPickedUp = false;
	}
	std::cout << "animation stopped" << std::endl;
}

void Animation::playAnimation() {
	if (!animationPlaying) {
		return;
	}
	switch (currentAnimation) {
		case BOUNCE_ANIMATION: {
			bounce(this->initialPosition.y);
			break;
		}
		case SPIN_ANIMATION: {
			spin(this->spinAxis);
			break;
		}
		case THROW_ANIMATION: {
			throwBall(pitch, yaw);
			break;
		}
		case ROLL_ANIMATION: {
			roll(yaw);
			break;
		}
		default: break;
	}
}

void Animation::pickUpBall(glm::vec3 playerPosition) {
	this->isBallPickedUp = true;
	setInitialPosition(playerPosition);
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::dropBall() {
	this->isBallPickedUp = false;
	startAnimation(BOUNCE_ANIMATION);
}

void Animation::moveBall(glm::vec3 playerPosition) {
	// move ball with camera
	setInitialPosition(playerPosition);
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::animateBounce() {
	this->initialPosition = currentPosition;
	startAnimation(BOUNCE_ANIMATION);
}

void Animation::animateSpin() {
	startAnimation(SPIN_ANIMATION);
}

void Animation::animateThrow(float pitch, float yaw) {
	this->pitch = pitch;
	this->yaw = yaw;
	startAnimation(THROW_ANIMATION);
}

void Animation::animateRoll(float yaw) {
	this->yaw = yaw;
	startAnimation(ROLL_ANIMATION);
}

/* bounce animation */
void Animation::bounce(float initialHeight) {
	float dampingFactor = 1.0 - elasticity;
	float elapsedTime = glfwGetTime() - animationStartTime;
	float amplitude = initialHeight * BOUNCE_HEIGHT * elasticity;
	double oscillation = dampedOscillation(amplitude, dampingFactor, animationSpeed, elapsedTime);
	float prevY = currentPosition.y;
	float posY = UNIT_STEP * abs(oscillation);
	if (posY <= MIN_BOUNCE) {
		stopAnimation();
		return;
	}
	currentPosition.y = posY;
	transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0,-initialHeight + posY,0));
}

void Animation::spin(glm::vec3 axis) {
	float dampingFactor = 1.0 - this->elasticity;
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingCoefficient = std::exp(-elasticity * elapsedTime);
	GLfloat rotationAngle = elapsedTime * this->animationSpeed * dampingCoefficient;
	if (dampingCoefficient <= MAX_DAMPING) {
		stopAnimation();
		return;
	}
	this->transformationMatrix = glm::rotate(this->transformationMatrix, glm::radians(rotationAngle), axis);
	glm::mat4 spinTransformation = glm::translate(this->transformationMatrix, initialPosition);
	this->currentPosition = (glm::vec3(spinTransformation * glm::vec4(this->currentPosition, 1.0)));
}

void Animation::roll(float angle) {
	// Rotation = (Time * velocity) / radius

	glm::vec3 direction = glm::vec3(sin(glm::radians(yaw)), 0, cos(glm::radians(yaw))); // in normalized coordinates
	glm::vec3 rollAxis = glm::cross(glm::vec3(0, 1, 0), direction);
	this->transformationMatrix = glm::rotate(this->transformationMatrix, teta, rollAxis);
	currentPosition = glm::vec3(this->transformationMatrix * glm::vec4(currentPosition, 1.0));
	currentPosition += UNIT_STEP * direction;
	initialPosition = glm::vec3(0, 0, 0);
	teta += 1.0;

	//todo: check boundaries

	/*
	float R = 100.0;
	float dr = glm::length(direction);
	float dx = direction.x;
	float dy = direction.y;
	float cosRotationAngle = cos(dr / R);
	float sinRotationAngle = sin(dr / R);
	glm::mat4 rotationMatrix = glm::mat4(1.0);
	rotationMatrix[0][0] = cosRotationAngle + (dy / dr) * (dy / dr) * (1 - cosRotationAngle);
	rotationMatrix[0][1] = - (dy / dr) * (dy / dr) * (1 - cosRotationAngle);
	rotationMatrix[0][2] = (dy / dr) * sinRotationAngle;
	rotationMatrix[1][0] = -(dx / dr) * (dy / dr) * (1 - cosRotationAngle);
	rotationMatrix[1][1] = cosRotationAngle + (dx / dr) * (dx / dr) * (1 - cosRotationAngle);
	rotationMatrix[1][2] = (dy / dr) * sinRotationAngle;
	rotationMatrix[2][0] = -(dx / dr) * sinRotationAngle;
	rotationMatrix[2][1] = -(dy / dr) * sinRotationAngle;
	rotationMatrix[2][2] = cosRotationAngle;
	rotationMatrix[3][0] = 0.1;
	rotationMatrix[3][1] = 0.0;
	*/

}

void Animation::throwBall(float pitch, float yaw) {
	//trajectory: ellipsoid

	float distance = abs(sin(glm::radians(pitch))) * THROW_DISTANCE;
	float height = abs(sin(glm::radians(pitch))) * THROW_HEIGHT;
	// the position of the ball on the curvilinear trajectory are expressed in polar coordinates
	float x = sin(glm::radians(yaw)) * distance;
	float z = cos(glm::radians(teta)) * distance;
	float y = sin(glm::radians(teta)) * height;
	// the angle enclosed between the yz axis, which is incremented at each step to reconstruct the path of the flying ball
	teta += 1.0;

	if (teta >= 180) {
		stopAnimation();
		animateBounce();
		return;
	}

	if (currentPosition.y <= 0) {
		stopAnimation();
		return;
	}

	this->transformationMatrix = glm::rotate(glm::mat4(1.0), glm::radians(yaw), glm::vec3(0, 1, 0));
	this->transformationMatrix = glm::translate(this->transformationMatrix, glm::vec3(x, y, z - distance));

	this->currentPosition = glm::vec3(this->transformationMatrix * glm::vec4(this->initialPosition, 1.0));
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
