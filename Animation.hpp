#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

enum ANIMATION_EFFECT { BOUNCE, SPIN };

#pragma once
class Animation
{
public:
	// constructor
	Animation();
	// setters
	void setTransformationMatrix(glm::mat4 newTransformationMatrix);
	void setInitialPosition(glm::vec3 newPosition);
	void setAnimationSpeed(float speed);
	// getters for querying the animation's state
	glm::mat4 getTransformationMatrix();
	glm::vec3 getCurrentPosition();
	bool isAnimationPlaying();

	// control animation
	void startBounceAnimation(float elasticity);
	void startSpinAnimation(glm::vec3 axis, float dampingFactor);
	void stopAnimation();
	void playAnimation();

private:
	glm::mat4 transformationMatrix;
	glm::vec3 initialPosition;
	glm::vec3 currentPosition;
	ANIMATION_EFFECT animationEffect;

	// general animation properties
	float animationSpeed = DEFAULT_ANIMATION_SPEED;
	bool animationPlaying = false;
	float animationStartTime;

	// specific animation properties
	glm::vec3 axis = glm::vec3(0, 1, 0); // y axis
	float dampingFactor = 0.0;

	// bounce effect
	void bounce(float dampingFactor, float speed);
	// spin object around an axis
	void spin(glm::vec3 axis, float dampingFactor, float speed);
	// configure the basic animation params
	void configureAnimation(ANIMATION_EFFECT newAnimationEffect);

	// constants
	const float UNIT_STEP = 0.1; 
	
	const float MIN_BOUNCE = 0.0001;
	const float MAX_DAMPING = 0.001;

	const float BOUNCE_HEIGHT = 100.0;
	const float DEFAULT_ANIMATION_SPEED = 10.0;
};

