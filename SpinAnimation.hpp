#include "Animation.hpp"

#pragma once
class SpinAnimation :
    public Animation
{
public:
    SpinAnimation(Animation basicAnimation, glm::vec3 axis, float dampingFactor);
    void playAnimation() override;
private:
    // spin object around an axis
    void spin(glm::vec3 axis, float dampingFactor, float speed);
    // animation attributes
    glm::vec3 axis = glm::vec3(0, 1, 0); // y axis
    float dampingFactor = 0.0;
    const float MAX_DAMPING = 0.001;
};

