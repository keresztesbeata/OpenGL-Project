#define GLEW_STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp> //core glm functionality
#include <glm/gtc/matrix_transform.hpp> //glm extension for generating common transformation matrices
#include <glm/gtc/matrix_inverse.hpp> //glm extension for computing inverse matrices
#include <glm/gtc/type_ptr.hpp> //glm extension for accessing the internal data structure of glm types

#include "Window.h"
#include "Shader.hpp"
#include "Camera.hpp"
#include "Model3D.hpp"
#include "SkyBox.hpp"
#include "Animation.hpp"
#include "LightSource.hpp"

#include <iostream>

// window
gps::Window myWindow;
int myWindowWidth = 1024;
int myWindowHeight = 768;
int retina_width = myWindowWidth;
int retina_height = myWindowHeight;

// shadow
const unsigned int SHADOW_WIDTH = 2048;
const unsigned int SHADOW_HEIGHT = 2048;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;
glm::mat4 lightRotation;

// object's properties
const float BALL_ELASTICITY = 0.8;
const float BALL_WEIGHT = 5;
const float PLAYER_HEIGHT = 12.0;
const float DIST_FROM_CENTER_OF_FIELD = 50.0f;
const glm::vec3 DIST_FROM_BALL = glm::vec3(0, 3, 5);
const float MIN_DIST_FROM_BALL = 20;
const float MIN_DIST_FROM_GOAL = 2;
const float LIGHT_HEIGHT = 20.0f;
const float GLOBAL_LIGHT_HEIGHT = 20.0f;
const glm::vec3 GOAL1_POSITION = glm::vec3(0, 22, -86);
const glm::vec3 GOAL2_POSITION = glm::vec3(0, 22, 86);
const float BOARD_WIDTH = 4;
const float BOARD_LENGTH = 5;

// court dimensions
const float BASKETBALL_COURT_LENGTH = 176.0;
const float BASKETBALL_COURT_WIDTH = 50.0;
const float BASKETBALL_COURT_HEIGHT = 30.0;

// initial position of objects and camera
glm::vec3 ballInitialPosition = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraInitialPosition = glm::vec3(0.0, PLAYER_HEIGHT, DIST_FROM_CENTER_OF_FIELD);
glm::vec3 initialLightPosition = glm::vec3(0.0, GLOBAL_LIGHT_HEIGHT, 1.0);

// light sources
LightSource* directionalLight;
LightSource* spotLight;
LightSource* flashLight;
LightSource* pointLightMiddle;
LightSource* pointLightLeft;
LightSource* pointLightRight;

// initial light position
glm::vec3 initialPointLightMiddlePosition = glm::vec3(0.0, LIGHT_HEIGHT, 1.0);
glm::vec3 initialPointLightLeftPosition = glm::vec3(-10.0, LIGHT_HEIGHT, 1.0);
glm::vec3 initialPointLightRightPosition = glm::vec3(10.0, LIGHT_HEIGHT, 1.0);

// light properties
const glm::vec3 ORANGE_COLOUR = glm::vec3(1, 0.5, 0);
const glm::vec3 GREEN_COLOUR = glm::vec3(0.4, 1, 0.8);
const glm::vec3 BLUE_COLOUR = glm::vec3(0.5, 0.2, 1);
const glm::vec3 WHITE_COLOUR = glm::vec3(1, 1, 1);

const float MIN_CUT_OFF_ANGLE_FLASHLIGHTS = 1.0;
const float MIN_CUT_OFF_ANGLE_SPOTLIGHTS = 5.0;
float flashLightsCutOffAngle = 5.0; // defines the angle of contained area for flashLights
float spotLightsCutOffAngle = 10.0; // defines the angle of contained area for spotLights

// Animations
float animationSpeed = 5.0;
Animation ballAnimation(ballInitialPosition, animationSpeed);

// shaders
gps::Shader basicShader;
gps::Shader lightShader;
gps::Shader spotLightShader;
gps::Shader flashLightShader;
gps::Shader pointLightsShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

enum SHADER_TYPE { BASIC, FLASH_LIGHT, POINT_LIGHTS, SPOT_LIGHT };
SHADER_TYPE currentShader = BASIC;

LightSource* selectedLight = directionalLight;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint lightPositionLoc;
GLint cutOffAngleLoc;
GLint shininessLoc;
GLint cameraPosLoc;
GLint spotLightTargetLoc;

// camera
gps::Camera myCamera(
    cameraInitialPosition,
    ballInitialPosition,
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraAngle = -90.0f;
float rollAngle = 0.0;
float fov = 45.0f;
float pitch = 0.0f, yaw = -90.0f;

const float THROW_PITCH_OFFSET = 45;
const float THROW_YAW_OFFSET = 90;

// uniform camera movement taking into consideration the frequency of the rendered frames
GLfloat cameraSpeed = 1.0f;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

GLboolean pressedKeys[1024];

// models
gps::Model3D basketBall;
gps::Model3D basketBallCourt;
gps::Model3D lightCube;
gps::Model3D middleLight;
gps::Model3D leftLight;
gps::Model3D rightLight;

// skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

// depth map
GLuint shadowMapFBO;
GLuint depthMapTexture;

// check errors
GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// attributes for updating the mouse coordinates within the screen frame
float lastX = myWindowWidth / 2, lastY = myWindowWidth / 2;
float mouseSensitivity = 0.1f;
bool firstMouseMovement = true;
bool allowMouseMovements = false;

// initialize window, matrices and shaders, and add callbacks 
void initOpenGLWindow();
void setWindowCallbacks();
void initOpenGLState();
void initModels();
void initAnimations();
void initShaders();
void initUniforms();
void initUniformsForShader(gps::Shader shader);
void initLightSources();
void initFBO();
void initSkyBox();

// functions for processing movement actions
void processMovement();
void processLightMovement();
void processObjectMovement();
void processCameraMovement();

// select a shader
void selectShader();
// select a light source to animate
void selectLightSource();

// functions for updating the transformation matrices after the camera or the object has moved
void updateUniforms(gps::Shader shader, glm::mat4 model, bool depthPass);
void updateCommonUniformsForShader(gps::Shader shader, glm::mat4 model);
glm::mat4 getSceneTransformation();
glm::mat4 getModelForDrawingLightCube(LightSource* lightSource);

// render scene of objects
void renderScene();
void drawObjects(gps::Shader shader, bool depthPass);

// callback functions for handling user interactions
void windowResizeCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mousButtonCallback(GLFWwindow* window, int button, int action, int mods);
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset);

// clean-up
void cleanup();

int main(int argc, const char* argv[]) {

    try {
        initOpenGLWindow();
    }
    catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
    initModels();
    initAnimations();
    initSkyBox();
    initShaders();
    initLightSources();
    initUniforms();
    initFBO();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        selectShader();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
}

void selectShader() {
    if (pressedKeys[GLFW_KEY_1]) {
        currentShader = BASIC;
        selectedLight = directionalLight;
        initUniformsForShader(basicShader);
    }
    if (pressedKeys[GLFW_KEY_2]) {
        currentShader = FLASH_LIGHT;
        selectedLight = flashLight;
        initUniformsForShader(flashLightShader);
    }
    if (pressedKeys[GLFW_KEY_3]) {
        currentShader = SPOT_LIGHT;
        selectedLight = spotLight;
        initUniformsForShader(spotLightShader);
    }
    if (pressedKeys[GLFW_KEY_4]) {
        currentShader = POINT_LIGHTS;
        initUniformsForShader(pointLightsShader);
    }
}

void processMovement() {
    processCameraMovement();
    processObjectMovement();
    processLightMovement();
}

void processCameraMovement() {
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] || pressedKeys[GLFW_KEY_RIGHT_SHIFT]) {
        // combination of Shift + key is for controlling the objectAnimations
        return;
    }
    cameraSpeed = 20.0f * deltaTime;
    myCamera.setCameraSpeed(cameraSpeed);

    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT);
    }
    if (pressedKeys[GLFW_KEY_F]) {
        myCamera.move(gps::MOVE_UP);
    }
    if (pressedKeys[GLFW_KEY_C]) {
        myCamera.move(gps::MOVE_DOWN);
    }
    if (pressedKeys[GLFW_KEY_R]) {
        rollAngle += 1.0;
        myCamera.roll(rollAngle);
    }
    if (pressedKeys[GLFW_KEY_Q]) {
        cameraAngle -= 1.0f;
    }
    if (pressedKeys[GLFW_KEY_E]) {
        cameraAngle += 1.0f;
    }

    if (ballAnimation.isBallPickedUp()) {
        ballAnimation.setInitialPosition(myCamera.getCameraPosition() - DIST_FROM_BALL);
    }
}

void selectLightSource() {
    if (pressedKeys[GLFW_KEY_5]) {
        selectedLight = pointLightLeft;
    }
    if (pressedKeys[GLFW_KEY_6]) {
        selectedLight = pointLightMiddle;
    }
    if (pressedKeys[GLFW_KEY_7]) {
        selectedLight = pointLightRight;
    }
}

void processLightMovement() {
    
    // select a light source

    selectLightSource();
    if (pressedKeys[GLFW_KEY_J]) {
        selectedLight->move(gps::MOVE_LEFT);
    }
    if (pressedKeys[GLFW_KEY_L]) {
        selectedLight->move(gps::MOVE_RIGHT);
    }
    if (pressedKeys[GLFW_KEY_I]) {
        selectedLight->move(gps::MOVE_DOWN);
        if (selectedLight == spotLight) {
           // spotLightsCutOffAngle -= 0.1;
        }
    }
    if (pressedKeys[GLFW_KEY_K]) {
        selectedLight->move(gps::MOVE_UP);
        if (selectedLight == spotLight) {
          //  spotLightsCutOffAngle += 0.1;
        }
    }
    if (pressedKeys[GLFW_KEY_U]) {
        selectedLight->move(gps::ROTATE_CLOCKWISE);
    }
    if (pressedKeys[GLFW_KEY_O]) {
        selectedLight->move(gps::ROTATE_COUNTER_CLOCKWISE);
    }
    if (pressedKeys[GLFW_KEY_H]) {
        selectedLight->move(gps::MOVE_FORWARD);
    }
    if (pressedKeys[GLFW_KEY_N]) {
        selectedLight->move(gps::MOVE_BACKWARD);
    }
}

bool isGoalHit() {
   glm::vec3 currentPosition = ballAnimation.getCurrentPosition();
   return abs(currentPosition.y - GOAL1_POSITION.y) < BOARD_WIDTH &&
       abs(currentPosition.x - GOAL1_POSITION.x) < BOARD_LENGTH &&
       abs(currentPosition.z - GOAL1_POSITION.z) < MIN_DIST_FROM_GOAL;
}

bool isCloseToBall() {
    // height doesn't count: player can pick up the ball when he gets close enough to it
    return (abs(myCamera.getCameraPosition().x - ballAnimation.getCurrentPosition().x) < MIN_DIST_FROM_BALL) &&
        (abs(myCamera.getCameraPosition().z - ballAnimation.getCurrentPosition().z) < MIN_DIST_FROM_BALL);
}

void processObjectMovement() {
    
    // control keys for attaching/detaching object to/from the camera: combination of left CTRL + key

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && pressedKeys[GLFW_KEY_W]) {
        // show wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    }

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && pressedKeys[GLFW_KEY_F]) {
        // disable wireframe
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (isGoalHit()) {
        std::cout << "goal!" << std::endl;
        // todo display success!!
        ballAnimation.animateBounce();
        goto ANIMATE;
    }

   // if (isCloseToBall()) {
        // allow the player to pick up the ball when he/she is in close to it
       // std::cout << "You can pick up the ball" << std::endl;
        if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && pressedKeys[GLFW_KEY_P]) {
            // pick up the ball from the ground
            ballAnimation.pickUpBall(myCamera.getCameraPosition() - DIST_FROM_BALL);
            myCamera.setCameraTarget(ballAnimation.getCurrentPosition());
        }
    //}

    if (pressedKeys[GLFW_KEY_LEFT_CONTROL] && pressedKeys[GLFW_KEY_D]) {
        // drop the ball 
        ballAnimation.dropBall();
        goto ANIMATE;
    }

    // control keys for animation: combination of left SHIFT + key

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_T]) {
        // throw the ball if it was previously picked up
        ballAnimation.setTargetPosition(GOAL1_POSITION);
        ballAnimation.animateThrow(pitch + THROW_PITCH_OFFSET, cameraAngle);
        goto ANIMATE;
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_D]) {
        // dribble
        ballAnimation.animateDribble();
        goto ANIMATE;
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_B]) {
        // bounce
        ballAnimation.animateBounce();
        goto ANIMATE;
    }

    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_S]) {
        // spin
        ballAnimation.animateSpin();
        goto ANIMATE;
    }
    
ANIMATE:
    if (ballAnimation.isAnimationPlaying()) {
        // only 1 animation can be played at a time -> animations are independent of the player (object is moving while the player may not)
        if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_Z]) {
            // stop object animation
            ballAnimation.stopAnimation();
            return;
        }
        // play the current animation if any
        ballAnimation.playAnimation();
    }
    
}

void updateUniforms(gps::Shader shader, glm::mat4 model, bool depthPass) {
    //send model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    // do not send the other matrices to the depth map shader
    if (depthPass) {
        glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(directionalLight->computeLightSpaceTrMatrixDirectionalLight()));
        return;
    }
    //update view matrix
    view = myCamera.getViewMatrix();
    // send view matrix to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));  
    //send normal matrix data to shader
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(normalMatrix));
    // send projection matrix to shader
    projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
    // send the camera's position to shader
    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "cameraPos"), 1, GL_FALSE, glm::value_ptr(myCamera.getCameraPosition()));

    // update the light sources

    // flashlight is attached to the front of the camera
    flashLight->setLightPosition(myCamera.getCameraPosition());
    // the flashlight is oriented towards the camera's viewing (front) direction
    flashLight->setLightTarget(myCamera.getCameraFrontDirection());

    // both the spotlight and the directional light are oriented towards the ball's position
    spotLight->setLightTarget(ballAnimation.getCurrentPosition());
    directionalLight->setLightTarget(ballAnimation.getCurrentPosition());

    // send light dir to shader
    switch (currentShader) {
        case BASIC: {
            glUniform3fv(lightDirLoc, 1, glm::value_ptr(directionalLight->getLightPosition()));
            glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(directionalLight->computeLightSpaceTrMatrixDirectionalLight()));
            break;
        }
        case SPOT_LIGHT: {
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPosition"), 1, glm::value_ptr(spotLight->getLightPosition()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "spotLightTarget"), 1, glm::value_ptr(spotLight->getLightTarget()));
            glUniform1f(glGetUniformLocation(shader.shaderProgram, "cutOffAngle"), cos(glm::radians(spotLightsCutOffAngle)));
            break;
        }
        case FLASH_LIGHT: {
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "lightPosition"), 1, glm::value_ptr(flashLight->getLightPosition()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "spotLightTarget"), 1, glm::value_ptr(flashLight->getLightTarget()));
            glUniform1f(glGetUniformLocation(shader.shaderProgram, "cutOffAngle"), cos(glm::radians(flashLightsCutOffAngle)));
            break;
        }
        case POINT_LIGHTS: {
            
            // send point light dir and color to shader
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "leftPointLightColor"), 1, glm::value_ptr(pointLightLeft->getLightColor()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "rightPointLightColor"), 1, glm::value_ptr(pointLightRight->getLightColor()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "middlePointLightColor"), 1, glm::value_ptr(pointLightMiddle->getLightColor()));

            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "leftPointLightPosition"), 1, glm::value_ptr(pointLightLeft->getLightPosition()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "rightPointLightPosition"), 1, glm::value_ptr(pointLightRight->getLightPosition()));
            glUniform3fv(glGetUniformLocation(shader.shaderProgram, "middlePointLightPosition"), 1, glm::value_ptr(pointLightMiddle->getLightPosition()));
            break;
        }
    }
    
}

void updateCommonUniformsForShader(gps::Shader shader, glm::mat4 model) {
    view = myCamera.getViewMatrix();
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    projection = glm::perspective(glm::radians(fov), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

glm::mat4 getModelForDrawingLightCube(LightSource* lightSource) {
    glm::mat4 lightCubeModel = lightSource->getTransformationMatrix();
    lightCubeModel = glm::translate(lightCubeModel, 1.0f * lightSource->getLightPosition());
    lightCubeModel = glm::scale(lightCubeModel, glm::vec3(0.5f, 0.5f, 0.5f));
    return getSceneTransformation() * lightCubeModel;
}

glm::mat4 getSceneTransformation() {
    glm::mat4 sceneTransformation = glm::mat4(1.0);
    sceneTransformation = glm::rotate(sceneTransformation, glm::radians(cameraAngle), glm::vec3(0, 1, 0));
    return sceneTransformation;
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
  
    model = getSceneTransformation();
    // draw ball
    glm::mat4 ballTransformation = ballAnimation.getTransformationMatrix();
    if (!ballAnimation.isBallPickedUp() || ballAnimation.isAnimationPlaying()) {
        ballTransformation = ballTransformation * model;
    }
    updateUniforms(shader, ballTransformation, depthPass);
    basketBall.Draw(shader);
    updateUniforms(shader, model, depthPass);
    // draw the basketball court
    basketBallCourt.Draw(shader);
}

void drawLightSources(gps::Shader shader) {
    shader.useShaderProgram();
    switch (currentShader) {
        case BASIC: {
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(directionalLight));
            lightCube.Draw(shader);
            break;
        }
        case FLASH_LIGHT: {
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(flashLight));
            lightCube.Draw(shader);
            break;
        }
        case SPOT_LIGHT: {
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(spotLight));
            lightCube.Draw(shader);
            break;
        }
        case POINT_LIGHTS: {
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(pointLightMiddle));
            middleLight.Draw(shader);
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(pointLightLeft));
            leftLight.Draw(shader);
            updateCommonUniformsForShader(shader, getModelForDrawingLightCube(pointLightRight));
            rightLight.Draw(shader);
            break;
        }
    }
}

void renderScene() {
    // update time variables after rendering the each frame, to create a uniform camera movement 
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    //render the scene to the depth buffer 
    glViewport(0, 0, SHADOW_WIDTH, SHADOW_HEIGHT);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawObjects(depthMapShader, true);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    // final scene rendering pass (with shadows)
    glViewport(0, 0, retina_width, retina_height);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    gps::Shader selectedShader = basicShader;
    switch (currentShader) {
        case BASIC: {
            selectedShader = basicShader;
            break;
        }
        case FLASH_LIGHT: {
            selectedShader = flashLightShader;
            break;
        }
        case SPOT_LIGHT: {
            selectedShader = spotLightShader;
            break;
        }
        case POINT_LIGHTS: {
            selectedShader = pointLightsShader;
            break;
        }
    }
    
    //bind the shadow map
    selectedShader.useShaderProgram();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(selectedShader.shaderProgram, "shadowMap"), 3);

    // draw the objects with the currently seleted shader
    drawObjects(selectedShader, false);

    //draw a white cube around each light
    drawLightSources(lightShader);

    // draw the skybox last
    mySkyBox.Draw(skyboxShader, view, projection);
}

void initModels() {
    basketBall.LoadModel("models/basketball/basketball.obj", "models/basketball/");
    basketBallCourt.LoadModel("models/basketball_court_glossy/basketball_court_glossy.obj", "models/basketball_court_glossy/");
    lightCube.LoadModel("models/cube/cube.obj");
    leftLight.LoadModel("models/cube/cube.obj");
    rightLight.LoadModel("models/cube/cube.obj");
    middleLight.LoadModel("models/cube/cube.obj");
}

void initAnimations() {
    ballAnimation.setCourtDimensions(glm::vec3(0,0,0), BASKETBALL_COURT_WIDTH, BASKETBALL_COURT_LENGTH, BASKETBALL_COURT_HEIGHT);
    ballAnimation.setObjectProperties(BALL_ELASTICITY, BALL_WEIGHT);
}

void initShaders() {
    basicShader.loadShader(
        "shaders/shadowShader.vert",
        "shaders/shadowShader.frag");
    lightShader.loadShader(
        "shaders/lightShader.vert",
        "shaders/lightShader.frag");
    depthMapShader.loadShader(
        "shaders/depthMapShader.vert",
        "shaders/depthMapShader.frag");
    skyboxShader.loadShader(
        "shaders/skyboxShader.vert",
        "shaders/skyboxShader.frag");
    flashLightShader.loadShader(
        "shaders/flashLightShader.vert",
        "shaders/flashLightShader.frag");
    spotLightShader.loadShader(
        "shaders/spotLightShader.vert",
        "shaders/spotLightShader.frag");
    pointLightsShader.loadShader(
        "shaders/pointLightsShader.vert",
        "shaders/pointLightsShader.frag");
}

void initUniformsForShader(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix 
    model = getSceneTransformation();;
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix 
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(shader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(fov),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glUniformMatrix3fv(glGetUniformLocation(shader.shaderProgram, "cameraPos"), 1, GL_FALSE, glm::value_ptr(myCamera.getCameraPosition()));

    if (currentShader == POINT_LIGHTS) {
        // send light dir to shader
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "leftPointLightColor"), 1, glm::value_ptr(pointLightLeft->getLightColor()));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "rightPointLightColor"), 1, glm::value_ptr(pointLightRight->getLightColor()));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "middlePointLightColor"), 1, glm::value_ptr(pointLightMiddle->getLightColor()));

        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "leftPointLightPosition"), 1, glm::value_ptr(pointLightLeft->getLightPosition()));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "rightPointLightPosition"), 1, glm::value_ptr(pointLightRight->getLightPosition()));
        glUniform3fv(glGetUniformLocation(shader.shaderProgram, "middlePointLightPosition"), 1, glm::value_ptr(pointLightMiddle->getLightPosition()));
        
        return;
    }

    if (currentShader == SPOT_LIGHT) {
        lightPositionLoc = glGetUniformLocation(shader.shaderProgram, "lightPosition");
        glUniform3fv(lightPositionLoc, 1, glm::value_ptr(spotLight->getLightPosition()));
    }
    else if (currentShader == FLASH_LIGHT) {
        lightPositionLoc = glGetUniformLocation(shader.shaderProgram, "lightPosition");
        glUniform3fv(lightPositionLoc, 1, glm::value_ptr(flashLight->getLightPosition()));
    }
    else {
        // send light dir to shader
        lightDirLoc = glGetUniformLocation(shader.shaderProgram, "lightDir");
        glUniform3fv(lightDirLoc, 1, glm::value_ptr(directionalLight->getLightDir()));
    }
    // send light color to shader
    lightColorLoc = glGetUniformLocation(shader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(directionalLight->getLightColor()));

    if (currentShader == FLASH_LIGHT) {
        spotLightTargetLoc = glGetUniformLocation(shader.shaderProgram, "spotLightTarget");
        glUniform3fv(spotLightTargetLoc, 1, glm::value_ptr(flashLight->getLightTarget()));

        cutOffAngleLoc = glGetUniformLocation(shader.shaderProgram, "cutOffAngle");
        glUniform1f(cutOffAngleLoc, cos(glm::radians(flashLightsCutOffAngle)));
    }

    if (currentShader == SPOT_LIGHT) {
        spotLightTargetLoc = glGetUniformLocation(shader.shaderProgram, "spotLightTarget");
        glUniform3fv(spotLightTargetLoc, 1, glm::value_ptr(spotLight->getLightTarget()));

        cutOffAngleLoc = glGetUniformLocation(shader.shaderProgram, "cutOffAngle");
        glUniform1f(cutOffAngleLoc, cos(glm::radians(spotLightsCutOffAngle)));
    }

}

void initUniforms() {
    // init the uniform matrices for the basic shader
    initUniformsForShader(basicShader);

    // send the projection matrix to the light shader
    lightShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(lightShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));

    // load faces for skybox    
    mySkyBox.Load(faces);
    skyboxShader.useShaderProgram();
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(skyboxShader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

// must be called before initUniforms()!!!
void initLightSources() {
    //set the light target (where the lights are pointing to) and position of light sources
    //set light color 

    directionalLight = new LightSource(initialLightPosition, ballInitialPosition, WHITE_COLOUR);
    flashLight = new LightSource(cameraInitialPosition, ballInitialPosition, WHITE_COLOUR);
    spotLight = new LightSource(initialLightPosition, ballInitialPosition, WHITE_COLOUR);
    pointLightMiddle = new LightSource(initialPointLightMiddlePosition, ballInitialPosition, WHITE_COLOUR);
    pointLightLeft = new LightSource(initialPointLightLeftPosition, ballInitialPosition, WHITE_COLOUR);
    pointLightRight = new LightSource(initialPointLightRightPosition, ballInitialPosition, WHITE_COLOUR);
}

void initFBO() {
    //Create the FBO, the depth texture and attach the depth texture to the FBO
    //generate FBO ID 
    glGenFramebuffers(1, &shadowMapFBO);
    //create depth texture for FBO 
    glGenTextures(1, &depthMapTexture);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
        SHADOW_WIDTH, SHADOW_HEIGHT, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    //attach texture to FBO 
    glBindFramebuffer(GL_FRAMEBUFFER, shadowMapFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void initSkyBox() {
    faces.push_back("textures/skybox/right.tga");
    faces.push_back("textures/skybox/left.tga");
    faces.push_back("textures/skybox/top.tga");
    faces.push_back("textures/skybox/bottom.tga");
    faces.push_back("textures/skybox/back.tga");
    faces.push_back("textures/skybox/front.tga");
}

// callback functions

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    if (width == 0 || height == 0) {
        return;
    }
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    basicShader.useShaderProgram();

    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(basicShader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    glViewport(0, 0, retina_width, retina_height);
}

void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode) {
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }
    if (key >= 0 && key < 1024) {
        if (action == GLFW_PRESS) {
            pressedKeys[key] = true;
        }
        else if (action == GLFW_RELEASE) {
            pressedKeys[key] = false;
        }
    }
}

void mousButtonCallback(GLFWwindow* window, int button, int action, int mods) {
    allowMouseMovements = button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS;
}

void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    // zoom the camera
    fov -= (float)yoffset;
    if (fov < 1.0f)
        fov = 1.0f;
    if (fov > 45.0f)
        fov = 45.0f;

    // change the radius of the flashlight
    float inc = (yoffset < 0) ? 0.1 : -0.1;
    flashLightsCutOffAngle += inc;
    if (flashLightsCutOffAngle < MIN_CUT_OFF_ANGLE_FLASHLIGHTS) {
        flashLightsCutOffAngle = MIN_CUT_OFF_ANGLE_FLASHLIGHTS;
    }
}

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (firstMouseMovement) {
        lastX = xpos;
        lastY = ypos;
        firstMouseMovement = false;
    }
    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos; // in open-gl, the direction of the y axis is reversed
    lastX = xpos;
    lastY = ypos;

    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    yaw += xoffset;
    pitch += yoffset * 2;

    if (pitch >= 89) {
        pitch = 89;
    }
    else if (pitch <= -89) {
        pitch = -89;
    }

    myCamera.rotate(pitch, yaw);
}

void cleanup() {
    glDeleteTextures(1, &depthMapTexture);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glDeleteFramebuffers(1, &shadowMapFBO);
    myWindow.Delete();
    //close GL context and any other GLFW resources
    glfwTerminate();
}

GLenum glCheckError_(const char* file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR) {
        std::string error;
        switch (errorCode) {
        case GL_INVALID_ENUM:
            error = "INVALID_ENUM";
            break;
        case GL_INVALID_VALUE:
            error = "INVALID_VALUE";
            break;
        case GL_INVALID_OPERATION:
            error = "INVALID_OPERATION";
            break;
        case GL_STACK_OVERFLOW:
            error = "STACK_OVERFLOW";
            break;
        case GL_STACK_UNDERFLOW:
            error = "STACK_UNDERFLOW";
            break;
        case GL_OUT_OF_MEMORY:
            error = "OUT_OF_MEMORY";
            break;
        case GL_INVALID_FRAMEBUFFER_OPERATION:
            error = "INVALID_FRAMEBUFFER_OPERATION";
            break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

void initOpenGLWindow() {
    myWindow.Create(myWindowWidth, myWindowHeight, "OpenGL Project");
}

void setWindowCallbacks() {
    glfwSetWindowSizeCallback(myWindow.getWindow(), windowResizeCallback);
    glfwSetKeyCallback(myWindow.getWindow(), keyboardCallback);
    glfwSetCursorPosCallback(myWindow.getWindow(), mouseCallback);
    glfwSetMouseButtonCallback(myWindow.getWindow(), mousButtonCallback);
    glfwSetScrollCallback(myWindow.getWindow(), scrollCallback);
}

void initOpenGLState() {
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}