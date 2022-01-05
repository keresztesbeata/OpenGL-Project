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

	//todo:delete
	std::cout << "started animation: " << animationType << std::endl;
	teta = 0;
	std::cout << "initial=";
	printVector(initialPosition);
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
			roll(targetPosition);
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

void Animation::animateRoll(glm::vec3 rollDirection) {
	this->targetPosition = rollDirection;
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

void Animation::roll(glm::vec3 direction) {
	// Rotation = (Time * velocity) / radius

	glm::mat4 trMatrix = glm::mat4(1.0);
	trMatrix = glm::translate(trMatrix, -currentPosition);
	trMatrix = glm::translate(trMatrix, glm::vec3(0,-1.0,0));
	glm::mat4 rotationMatrix = glm::rotate(glm::mat4(1.0), 0.01f, glm::vec3(0.5, 0.5, 0));
	glm::mat4 invTrMatrix = glm::mat4(1.0);
	invTrMatrix = glm::translate(invTrMatrix, glm::vec3(0, 1.0, 0));
	invTrMatrix = glm::translate(invTrMatrix, currentPosition);
	//rotationMatrix = glm::translate(rotationMatrix, 0.01f * glm::normalize(direction));
	this->transformationMatrix = invTrMatrix * rotationMatrix * trMatrix;

	this->currentPosition = (glm::vec3(this->transformationMatrix * glm::vec4(this->currentPosition, 1.0)));
	printVector(this->currentPosition);
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

/*
void Animation::throwBall(float pitch, float yaw) {
	float unit_step = 0.1;

	float xOffset = (yaw < 0) ? -1.0 : 1.0;
	float zOffset = (targetPosition.z > 0) ? 1.0 : -1.0;

	std::cout << "curr pos = ";
	printVector(currentPosition);
	
	
	if (currentPosition.y <= 0) {
		if (pitch < 0) {
			// throw the ball down: hit ground and bounce back
			setInitialPosition(currentPosition);
			startAnimation(BOUNCE_ANIMATION);
			return;
		}
		else {
			// ball has fallen down to the ground
			// todo: check if it goes beyond the field's boundaries
			stopAnimation();
			return;
		}
	}
	
	float halfThrowDist =  10;//abs(THROW_HEIGHT / tan(pitch)) *10;
	std::cout << "half = " << halfThrowDist << std::endl;
	bool passedMiddle = length(currentPosition - initialPosition) > abs(halfThrowDist);
	float yOffset = (passedMiddle) ? 1.0 : -1.0;
	std::cout << "passed middle = " << passedMiddle << std::endl;

	glm::vec3 prev = currentPosition;

	currentPosition.x += xOffset * unit_step;
	currentPosition.z += zOffset * unit_step;

	float radius = halfThrowDist;
	glm::vec3 center = glm::vec3(initialPosition.x + radius * xOffset, initialPosition.y + radius, initialPosition.z + zOffset * radius);

	std::cout << "center=";
	printVector(center);
	float yPos = sphereEquation(radius, center, currentPosition.x, currentPosition.z);

	currentPosition.y = yPos * yOffset + center.y;

	std::cout << "ypos = " << yPos << std::endl;


	if (currentPosition.y <= initialPosition.y - THROW_OFFSET) {
		setInitialPosition(currentPosition);
		startAnimation(BOUNCE_ANIMATION);
		return;
	}

	this->transformationMatrix = glm::translate(this->transformationMatrix, -prev + currentPosition);
}
*/

void Animation::throwBall(float pitch, float yaw) {
	//trajectory: semi-circle

	float radius = 10; //todo: from pitch
	// the position of the ball on the curvilinear trajectory are expressed in polar coordinates
	float z = cos(glm::radians(teta)) * radius;
	float y = sin(glm::radians(teta)) * radius;
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
	currentPosition.y = initialPosition.y + y;
	currentPosition.z = initialPosition.z + z - radius;

	this->transformationMatrix = glm::translate(glm::mat4(1.0), glm::vec3(0, y, z - radius));
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
