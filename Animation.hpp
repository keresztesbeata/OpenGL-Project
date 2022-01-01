#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

enum ANIMATION_TYPE {ROLL_ANIMATION, BOUNCE_ANIMATION, SPIN_ANIMATION, THROW_ANIMATION};
enum MOVE_DIRECTION { LEFT, RIGHT, UP, DOWN };

#pragma once
class Animation
{
public:
	// constructor
	Animation(float elasticity, glm::vec3 initialPosition);
	Animation(float elasticity, glm::vec3 initialPosition, float animationSpeed);
	// setters
	void setTransformationMatrix(glm::mat4 newTransformationMatrix);
	void setInitialPosition(glm::vec3 newPosition);
	void setAnimationSpeed(float speed);
	void setElasticity(float elasticity);
	// getters for querying the animation's state
	glm::mat4 getTransformationMatrix();
	glm::vec3 getCurrentPosition();
	glm::vec3 getInitialPosition();
	bool isAnimationPlaying();

	// control animation
	void startAnimation(ANIMATION_TYPE animationType);
	void stopAnimation();
	virtual void playAnimation();

private:
	glm::mat4 transformationMatrix;
	glm::vec3 initialPosition;
	glm::vec3 currentPosition;
	ANIMATION_TYPE currentAnimation;

	// general animation properties
	float animationSpeed = DEFAULT_ANIMATION_SPEED;
	bool animationPlaying = false;
	float animationStartTime;

	// object specific properties
	float elasticity;
	glm::vec3 spinAxis;
	MOVE_DIRECTION moveDirection;
	bool firstDrop;

	// constants
	const float UNIT_STEP = 0.01; 
	const float DEFAULT_ANIMATION_SPEED = 10.0;

	// bounce effect
	void bounce();
	void spin(glm::vec3 axis);

	void initAnimation(float elasticity, glm::vec3 initialPosition, float animationSpeed);
	void startBounceAnimation();

	// animation constants
	const float MIN_BOUNCE = 0.0001;
	const float BOUNCE_HEIGHT = 100.0;
	const float MAX_DAMPING = 0.001;
};

