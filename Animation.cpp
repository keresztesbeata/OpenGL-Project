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
	this->ballPickedUp = false;
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

bool Animation::isBallPickedUp() {
	return ballPickedUp;
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
	this->initialPosition = this->currentPosition;
	this->teta = 0;
	playAnimation();
}

void Animation::stopAnimation() {
	animationPlaying = false;
}

void Animation::playAnimation() {
	switch (currentAnimation) {
		case BOUNCE_ANIMATION: {
			bounce(this->initialPosition.y);
			break;
		}
		case DRIBBLE_ANIMATION: {
			dribble(this->initialPosition.y);
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
		default: break;
	}
}

void Animation::pickUpBall(glm::vec3 playerPosition) {
	this->ballPickedUp = true;
	setInitialPosition(playerPosition);
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::dropBall() {
	this->currentPosition.y = 0.0;
	this->ballPickedUp = false;
	startAnimation(BOUNCE_ANIMATION);
}

void Animation::moveBall(glm::vec3 playerPosition) {
	// move ball with camera
	setInitialPosition(playerPosition);
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::animateBounce() {
	ballPickedUp = false;
	startAnimation(BOUNCE_ANIMATION);
}

void Animation::animateDribble() {
	startAnimation(DRIBBLE_ANIMATION);
}

void Animation::animateSpin() {
	startAnimation(SPIN_ANIMATION);
}

void Animation::animateThrow(float pitch, float yaw) {
	ballPickedUp = false;
	this->pitch = pitch;
	this->yaw = yaw;
	startAnimation(THROW_ANIMATION);
}

void Animation::dribble(float initialHeight) {
	float elapsedTime = glfwGetTime() - animationStartTime;
	float amplitude = (initialHeight + 1) * BOUNCE_HEIGHT;
	double oscillation = dampedOscillation(amplitude, 0, animationSpeed, elapsedTime);
	float posY = UNIT_STEP * abs(oscillation);
	currentPosition.y = posY;
	transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, -initialHeight + posY, 0));
}

void Animation::bounce(float initialHeight) {
	float dampingFactor = 1.0 - elasticity;
	float elapsedTime = glfwGetTime() - animationStartTime;
	float amplitude = (initialHeight + 1) * BOUNCE_HEIGHT * elasticity;
	double oscillation = dampedOscillation(amplitude, dampingFactor, animationSpeed, elapsedTime);
	float posY = UNIT_STEP * abs(oscillation);
	if (posY <= MIN_BOUNCE) {
		stopAnimation();
		this->currentPosition.y = 0;
		this->ballPickedUp = false;
		return;
	}
	currentPosition.y = posY;
	transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, -initialHeight + posY,0));
}

void Animation::spin(glm::vec3 axis) {
	float dampingFactor = 1.0 - this->weight;
	float elapsedTime = glfwGetTime() - animationStartTime;
	float dampingCoefficient = std::exp(-elasticity * elapsedTime);
	float angularVelocity = this->animationSpeed * dampingCoefficient;
	GLfloat rotationAngle = elapsedTime * angularVelocity;
	if (dampingCoefficient <= MAX_DAMPING) {
		stopAnimation();
		return;
	}
	this->transformationMatrix = glm::rotate(this->transformationMatrix, glm::radians(rotationAngle), axis);
	glm::mat4 spinTransformation = glm::translate(this->transformationMatrix, initialPosition);
}

void Animation::throwBall(float pitch, float yaw) {
	// trajectory of the flying ball is a parabola
	float velocity = 25;
	float g = 9.8;
	float time = glfwGetTime() - animationStartTime;
	float z = - velocity * cos(glm::radians(pitch)) * time;
	float y = velocity * sin(glm::radians(pitch)) * time - g * time * time / 2;
	
	if (currentPosition.y < 0) {
		stopAnimation();
		currentPosition.y = initialPosition.y/2;
		animateBounce();
		return;
	}
	this->transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, y, z));
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
