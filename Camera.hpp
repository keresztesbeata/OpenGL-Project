#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtx/transform.hpp>
#include <GLFW/glfw3.h>

#include <string>

namespace gps {
    
    enum MOVE_DIRECTION {MOVE_FORWARD, MOVE_BACKWARD, MOVE_RIGHT, MOVE_LEFT, MOVE_UP, MOVE_DOWN, ROTATE_CLOCKWISE, ROTATE_COUNTER_CLOCKWISE};
    
    class Camera
    {
    public:
        //Camera constructor
        Camera(glm::vec3 cameraPosition, glm::vec3 cameraTarget, glm::vec3 cameraUp);
        //return the view matrix, using the glm::lookAt() function
        glm::mat4 getViewMatrix();
        glm::vec3 getCameraPosition();
        glm::vec3 getCameraTarget();
        void setCameraPosition(glm::vec3 newCameraPosition);
        void setCameraTarget(glm::vec3 newCameraTarget);
        void setCameraSpeed(float speed);

        //update the camera internal parameters following a camera move event
        void move(MOVE_DIRECTION direction);
        //update the camera internal parameters following a camera rotate event
        //yaw - camera rotation around the y axis
        //pitch - camera rotation around the x axis
        void rotate(float pitch, float yaw);
        // roll camera with a given angle
        void roll(float rollAngle);
        
    private:
        glm::vec3 cameraPosition;
        glm::vec3 cameraTarget;
        glm::vec3 cameraFrontDirection;
        glm::vec3 cameraRightDirection;
        glm::vec3 cameraUpDirection;
        float cameraSpeed = 1.0;
    };
    
}

#endif /* Camera_hpp */
