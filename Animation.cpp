#include "Animation.hpp"

// precompute PI and cache it
const double PI = std::atan(1.0) * 4;

// helper functions
float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time);

// for debugging
void printVector(glm::vec3 v);
void printMatrix(glm::mat4 matrix);

Animation::Animation(glm::vec3 initialPosition, float animationSpeed) {
	initAnimation(initialPosition, animationSpeed);
}

Animation::Animation(glm::vec3 initialPosition) {
	initAnimation(initialPosition, 1.0);
}

void Animation::initAnimation(glm::vec3 initialPosition, float animationSpeed) {
	setAnimationSpeed(animationSpeed);
	this->initialPosition = initialPosition;
	this->currentPosition = this->initialPosition;
	this->targetPosition = this->currentPosition;
	this->transformationMatrix = glm::mat4(1.0);
}

void Animation::setObjectProperties(float elasticity, float weight) {
	this->elasticity = elasticity;
	this->weight = weight;
}

void Animation::setCourtDimensions(glm::vec3 reference, float width, float length, float height) {
	this->courtWidth = width;
	this->courtLength = length;
	this->fenceHeight = height;

	glm::mat4 transformCourtCorners = glm::mat4(1.0);
	transformCourtCorners = glm::scale(transformCourtCorners, glm::vec3(length/2, height, width/2));

	pO = glm::vec3(transformCourtCorners * glm::vec4(pO, 1.0));
	pX = glm::vec3(transformCourtCorners * glm::vec4(pX, 1.0));
	pY = glm::vec3(transformCourtCorners * glm::vec4(pY, 1.0));
	pZ = glm::vec3(transformCourtCorners * glm::vec4(pZ, 1.0));

	this->u = pX - pO;
	this->v = pY - pO;
	this->w = pZ - pO;
}

void Animation::setGoalProperties(glm::vec3 goalPosition, float width, float height, float maxError) {
	this->goalPosition = goalPosition;
	this->maxError = maxError;
	this->goalWidth = width;
	this->goalLength = height;
	this->maxError = maxError;
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
}

void Animation::stopAnimation() {
	animationPlaying = false;
	if (isOutsideBasketballCourt()) {
		// immediately return the ball to the player if it goes beyond the fence, after the ball has fallen down on the other side of the fence (in case the ball was thrown)
		pickUpBall(this->initialPosition);
	}
}

/*
* Checks if the point representing the ball's current position lies inside the basketball court (rectangular 3D shape of the predefined coordinates) using the dot product.
*/
bool Animation::isOutsideBasketballCourt() {
	glm::vec3 point = currentPosition;
	if (!animationPlaying) {
		// should only be checked during an animation (until the ball didn't stop moving)
		return false;
	}

	// check the x and z direction vectors, the upper part is not limited, the ball can fly as high up as the sky
	if(glm::dot(u, point) < glm::dot(u, pO) || glm::dot(u, point) > glm::dot(u, pX)) {
		return true;
	}
	if (glm::dot(w, point) < glm::dot(w, pO) || glm::dot(w, point) > glm::dot(w, pZ)) {
		return true;
	}

	return false;
}

bool Animation::isFenceHit() {
	return isOutsideBasketballCourt() && currentPosition.y <= fenceHeight;
}

bool Animation::isGoalHit() {
	return abs(currentPosition.y - goalPosition.y) < goalLength &&
		abs(currentPosition.x) - abs(goalPosition.x) < goalWidth &&
		abs(currentPosition.z) - abs(goalPosition.z) <= 0;
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
	
	if (isOutsideBasketballCourt()) {
		if (isFenceHit()) {
			hitAndBounce();
		}
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

void Animation::hitAndBounce() {
	stopAnimation();
	animateThrow(180, -180);
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
	float angularVelocity = 2*this->animationSpeed * dampingCoefficient;
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
	float x = - velocity * cos(glm::radians(yaw)) * sin(glm::radians(pitch)) * time;
	float z = - velocity * cos(glm::radians(pitch)) * time;
	float y = velocity * sin(glm::radians(pitch)) * sin(glm::radians(yaw)) * time - g * time * time / 2;
	
	this->transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, y, z));
	this->currentPosition = glm::vec3(this->transformationMatrix * glm::vec4(this->initialPosition, 1.0));

	if (currentPosition.y < 0) {
		stopAnimation();
		currentPosition.y = initialPosition.y / 2;
		animateBounce();
	}
}

/* helper functions */

float dampedOscillation(float amplitude, float dampingFactor, float oscillationFrequency, float time) {
	return amplitude * std::exp(-dampingFactor * time) * cos(PI * oscillationFrequency * glm::radians(time));
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
