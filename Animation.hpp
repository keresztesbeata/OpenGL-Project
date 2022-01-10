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
	Animation(glm::vec3 initialPosition);
	Animation(glm::vec3 initialPosition, float animationSpeed);
	// setters
	void setObjectProperties(float elasticity, float weight);
	void setCourtDimensions(glm::vec3 reference, float width, float length, float height);
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
	bool isOutsideBasketballCourt();
	bool isBallPickedUp();
	virtual void playAnimation();
	void hitAndBounce();

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
	float animationStartTime = 0.0;

	// object specific properties
	float elasticity = 0.0;
	float weight = 1.0;
	glm::vec3 spinAxis = glm::vec3(0,1,0); // y axis by default
	float pitch = 0.0, yaw = 0.0;
	float teta = 0.0;

	// constants
	const float UNIT_STEP = 0.01; 
	const float DEFAULT_ANIMATION_SPEED = 10.0;

	void dribble(float initialHeight);
	void bounce(float initialHeight);
	void spin(glm::vec3 axis);
	void throwBall(float pitch, float yaw);
	void hitWall();

	void startAnimation(ANIMATION_TYPE animationType);
	void initAnimation(glm::vec3 initialPosition, float animationSpeed);
	bool isFenceHit();

	// animation constants
	const float MIN_BOUNCE = 0.0001;
	const float BOUNCE_HEIGHT = 100.0;
	const float THROW_DISTANCE = 50.0;
	const float THROW_HEIGHT = 20.0;
	const float MAX_DAMPING = 0.001;

	// vertices for determining the shape of the bounding area for the object's movement
	glm::vec3 pO = glm::vec3(-1, 0, -1);
	glm::vec3 pZ = glm::vec3(-1, 0, 1);
	glm::vec3 pY = glm::vec3(-1, 1, -1);
	glm::vec3 pX = glm::vec3(1, 0, -1);

	// define the coordinate system for the 3d shape representing the court's boundaries
	glm::vec3 u;
	glm::vec3 v;
	glm::vec3 w;
};

