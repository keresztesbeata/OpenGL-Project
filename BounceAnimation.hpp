
#include "Animation.hpp"

#pragma once
class BounceAnimation :
    public Animation
{
public:
    BounceAnimation(Animation basicAnimation, float elasticity);
    void playAnimation() override;

private:
    // bounce effect
    void bounce(float elasticity, float speed);
    // animation attributes
    float elasticity = 1.0;
    const float MIN_BOUNCE = 0.0001;
    const float BOUNCE_HEIGHT = 100.0;
};

