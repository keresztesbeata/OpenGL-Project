#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cmath>

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
	glm::vec3 getInitialPosition();
	bool isAnimationPlaying();

	// control animation
	void startAnimation();
	void stopAnimation();
	virtual void playAnimation();

protected:
	glm::mat4 transformationMatrix;
	glm::vec3 initialPosition;
	glm::vec3 currentPosition;

	// general animation properties
	float animationSpeed = DEFAULT_ANIMATION_SPEED;
	bool animationPlaying = false;
	float animationStartTime;

	// constants
	const float UNIT_STEP = 0.1; 
	const float DEFAULT_ANIMATION_SPEED = 10.0;

	/* helper functions */
	bool isBackToOriginalPosition(glm::vec3 newPosition, glm::vec3 originalPosition, float minDistance);

	/* helper functions for debugging */
	void printMatrix(glm::mat4 matrix);
	void printVector(glm::vec3 v);
};

