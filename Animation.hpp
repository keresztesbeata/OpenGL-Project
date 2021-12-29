#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

enum ANIMATION_EFFECT { BOUNCE, CIRCLE, LOOK_AROUND };

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
	void startCircleAnimation(glm::vec3 center, float radius);
	void startLookAroundAnimation(glm::vec3 center, float radius);
	void stopAnimation();
	void playAnimation();

private:
	glm::mat4 transformationMatrix;
	glm::vec3 initialPosition;
	glm::vec3 currentPosition;
	ANIMATION_EFFECT animationEffect;

	// general animation properties
	float animationSpeed = 1.0;
	bool animationPlaying = false;
	float animationStartTime;

	// specific animation properties
	// for bounce effect
	float elasticity = 1.0;
	glm::vec3 center = glm::vec3(0.0);
	// for radius
	float radius = 1.0;

	// bounce effect
	void bounce(float elasticity, float speed);
	// circle around an object
	void circle(glm::vec3 center, float radius, float speed);
	// look around
	void lookAround(glm::vec3 center, float radius, float speed);
	// configure the basic animation params
	void configureAnimation(ANIMATION_EFFECT newAnimationEffect);

	// constants
	const float UNIT_STEP = 0.1; 
	const float MIN_BOUNCE = 0.001;
	const float MIN_DISPLACEMENT = 0.1;
	const float BOUNCE_HEIGHT = 50.0;
	const float DEFAULT_ANIMATION_SPEED = 10.0;
};

