#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

enum ANIMATION_TYPE {BOUNCE_ANIMATION, SPIN_ANIMATION, THROW_ANIMATION, DRIBBLE_ANIMATION};

#pragma once
class Animation
{
public:
	// constructor
	Animation(float elasticity, float weight, glm::vec3 initialPosition);
	Animation(float elasticity, float weight, glm::vec3 initialPosition, float animationSpeed);
	// setters
	void setInitialPosition(glm::vec3 newPosition);
	void setTargetPosition(glm::vec3 newPosition);
	void setAnimationSpeed(float speed);
	// getters for querying the animation's state
	glm::mat4 getTransformationMatrix();
	glm::vec3 getCurrentPosition();

	void animateSpin();
	void animateThrow(float pitch, float yaw);
	void animateDribble();
	void animateBounce();

	void pickUpBall(glm::vec3 playerPosition);
	void dropBall();
	void moveBall(glm::vec3 playerPosition);

	// control animation
	void stopAnimation();
	bool isAnimationPlaying();
	bool isBallPickedUp();
	virtual void playAnimation();

private:
	glm::mat4 transformationMatrix;
	glm::vec3 initialPosition;
	glm::vec3 currentPosition;
	glm::vec3 targetPosition;
	ANIMATION_TYPE currentAnimation;

	bool ballPickedUp = false;
	// general animation properties
	float animationSpeed = DEFAULT_ANIMATION_SPEED;
	bool animationPlaying = false;
	float animationStartTime;

	// object specific properties
	float elasticity;
	float weight;
	glm::vec3 spinAxis;
	float pitch, yaw;
	float teta = 0;

	// constants
	const float UNIT_STEP = 0.01; 
	const float DEFAULT_ANIMATION_SPEED = 10.0;

	void dribble(float initialHeight);
	void bounce(float initialHeight);
	void spin(glm::vec3 axis);
	void throwBall(float pitch, float yaw);

	void startAnimation(ANIMATION_TYPE animationType);
	void initAnimation(float elasticity, float weight, glm::vec3 initialPosition, float animationSpeed);

	// animation constants
	const float MIN_BOUNCE = 0.0001;
	const float BOUNCE_HEIGHT = 100.0;
	const float THROW_DISTANCE = 50.0;
	const float THROW_HEIGHT = 20.0;
	const float MAX_DAMPING = 0.001;
};

