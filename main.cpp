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
#include "BounceAnimation.hpp"
#include "SpinAnimation.hpp"
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
float ballElasticity = 0.5;

// initial position of objects and camera
glm::vec3 ballPosition = glm::vec3(0.0f, 5.0f, -10.0f);
glm::vec3 cameraInitialPosition = glm::vec3(0.0f, 7.0f, 15.0f);

// Animations
float animationSpeed = 5.0;
Animation ballAnimation(ballElasticity, ballPosition, animationSpeed);

// light source
LightSource * lightSource;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;
GLint cameraPosLoc;

// camera
gps::Camera myCamera(
    cameraInitialPosition,
    ballPosition,
    glm::vec3(0.0f, 1.0f, 0.0f));

GLfloat cameraAngle = 90.0f;
float rollAngle = 0.0;
// uniform camera movement taking into consideration the frequency of the rendered frames
GLfloat cameraSpeed = 0.1f;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

GLboolean pressedKeys[1024];

// models
gps::Model3D basketBall;
gps::Model3D lightCube;
gps::Model3D basketBallCourt;

// skybox
std::vector<const GLchar*> faces;
gps::SkyBox mySkyBox;

// shaders
gps::Shader basicShader;
gps::Shader lightShader;
gps::Shader depthMapShader;
gps::Shader skyboxShader;

GLuint shadowMapFBO;
GLuint depthMapTexture;

// check errors
GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

// attributes for updating the mouse coordinates within the screen frame
float lastX = myWindowWidth / 2, lastY = myWindowWidth / 2;
float pitch = 0.0f, yaw = -90.0f;
float angleX = 0.0f, angleY = 0.0f;
float mouseSensitivity = 0.1f;
bool firstMouseMovement = true;
bool allowMouseMovements = false;

// initialize window, matrices and shaders, and add callbacks 
void initOpenGLWindow();
void setWindowCallbacks();
void initOpenGLState();
void initModels();
void initShaders();
void initUniforms(gps::Shader shader);
void initLightSources();
void initFBO();
void initSkyBox();

// functions for processing movement actions
void processMovement();
void processLightMovement();
void processObjectMovement();
void processCameraMovement();

// functions for updating the transformation matrices after the camera or the object has moved
void updateUniforms(gps::Shader shader, glm::mat4 model, bool depthPass);
void updateUniformsForGivenShader(gps::Shader shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection);
glm::mat4 getBallTransformation();
glm::mat4 getModelForDrawingLightCube();

// render scene of objects
void renderScene();
void drawObjects(gps::Shader shader, bool depthPass);

// callback functions for handling user interactions
void windowResizeCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mousButtonCallback(GLFWwindow* window, int button, int action, int mods);

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
    initSkyBox();
    initShaders();
    initLightSources();
    initUniforms(basicShader);
    initFBO();
    setWindowCallbacks();

    glCheckError();
    // application loop
    while (!glfwWindowShouldClose(myWindow.getWindow())) {
        processMovement();
        renderScene();

        glfwPollEvents();
        glfwSwapBuffers(myWindow.getWindow());

        glCheckError();
    }

    cleanup();

    return EXIT_SUCCESS;
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
    cameraSpeed = 2.5f * deltaTime;
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
        model = glm::rotate(glm::mat4(1.0), glm::radians(cameraAngle), glm::vec3(0, 1, 0));
    }
    if (pressedKeys[GLFW_KEY_E]) {
        cameraAngle += 1.0f;
        model = glm::rotate(glm::mat4(1.0), glm::radians(cameraAngle), glm::vec3(0, 1, 0));
    }
}

void processLightMovement() {
    if (pressedKeys[GLFW_KEY_J]) {
        lightSource->move(gps::MOVE_LEFT);
    }
    if (pressedKeys[GLFW_KEY_L]) {
        lightSource->move(gps::MOVE_RIGHT);
    }
    if (pressedKeys[GLFW_KEY_I]) {
        lightSource->move(gps::MOVE_UP);
    }
    if (pressedKeys[GLFW_KEY_K]) {
        lightSource->move(gps::MOVE_DOWN);
    }
    if (pressedKeys[GLFW_KEY_U]) {
        lightSource->move(gps::ROTATE_CLOCKWISE);
    }
    if (pressedKeys[GLFW_KEY_O]) {
        lightSource->move(gps::ROTATE_COUNTER_CLOCKWISE);
    }
    if (pressedKeys[GLFW_KEY_H]) {
        lightSource->move(gps::MOVE_FORWARD);
    }
    if (pressedKeys[GLFW_KEY_N]) {
        lightSource->move(gps::MOVE_BACKWARD);
    }
}

void processObjectMovement() {
    // use left shift for object animation
    
    //start a new animation
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_B]) {
        // bounce
        ballAnimation.setInitialPosition(ballPosition);
        ballAnimation.startAnimation(BOUNCE_ANIMATION);
        return;
    }
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_S]) {
        // spin
        ballAnimation.setInitialPosition(ballPosition);
        ballAnimation.startAnimation(SPIN_ANIMATION);
        return;
    }
    if (ballAnimation.isAnimationPlaying()) {
        if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_Z]) {
            // stop object animation
            ballAnimation.stopAnimation();
            return;
        }
        // play the current animation if any
        ballAnimation.playAnimation();
        ballPosition = ballAnimation.getCurrentPosition();
    }
}

glm::mat4 getModelForDrawingLightCube() {
    glm::mat4 lightCubeModel = glm::mat4(1.0);
    lightCubeModel = glm::translate(lightCubeModel, 1.0f * lightSource->getLightDir());
    lightCubeModel = glm::scale(lightCubeModel, glm::vec3(0.5f, 0.5f, 0.5f));
    return lightCubeModel;
}

void updateUniforms(gps::Shader shader, glm::mat4 model, bool depthPass) {
    //send model matrix data to shader
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "lightSpaceTrMatrix"), 1, GL_FALSE, glm::value_ptr(lightSource->computeLightSpaceTrMatrix()));
    // do not send the other matrices for the depth map shader
    if (depthPass) {
        return;
    }
    //update view matrix
    view = myCamera.getViewMatrix();
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // compute normal matrix
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightSource->getLightDir()));
    // send camera position to shader
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));
}

void updateUniformsForGivenShader(gps::Shader shader, glm::mat4 model, glm::mat4 view, glm::mat4 projection) {
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "view"), 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "model"), 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(glGetUniformLocation(shader.shaderProgram, "projection"), 1, GL_FALSE, glm::value_ptr(projection));
}

glm::mat4 getBallTransformation() {
    return glm::translate(glm::mat4(1.0), 1.0f * ballPosition);
}

void drawObjects(gps::Shader shader, bool depthPass) {
    shader.useShaderProgram();
    // draw teapot
    glm::mat4 ballTransformation = getBallTransformation();
    if (ballAnimation.isAnimationPlaying()) {
        ballTransformation = ballTransformation * ballAnimation.getTransformationMatrix();
    }
    updateUniforms(shader, ballTransformation * model, depthPass);
    basketBall.Draw(shader);
    // animate only the object, not the ground
    updateUniforms(shader, model, depthPass);
    basketBallCourt.Draw(shader);
}

void drawLightSource(gps::Shader shader) {
    shader.useShaderProgram();
    updateUniformsForGivenShader(shader, getModelForDrawingLightCube(), myCamera.getViewMatrix(), projection);
    lightCube.Draw(shader);
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
    
    //bind the shadow map
    basicShader.useShaderProgram();
    glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, depthMapTexture);
    glUniform1i(glGetUniformLocation(basicShader.shaderProgram, "shadowMap"), 3);
    drawObjects(basicShader, false);
    
    //draw a white cube around the light
    drawLightSource(lightShader);

    // draw the skybox last
    //mySkyBox.Draw(skyboxShader, view, projection);
}

void initModels() {
    basketBall.LoadModel("models/basketball/basketball.obj","models/basketball/");
    lightCube.LoadModel("models/bloom_light/bloom_light.obj","models/bloom_light");
    basketBallCourt.LoadModel("models/basketball_court/basketball_court.obj", "models/basketball_court/");
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
}

void initUniforms(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(cameraAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    modelLoc = glGetUniformLocation(shader.shaderProgram, "model");

    // get view matrix for current camera
    view = myCamera.getViewMatrix();
    viewLoc = glGetUniformLocation(shader.shaderProgram, "view");
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
    normalMatrixLoc = glGetUniformLocation(shader.shaderProgram, "normalMatrix");

    // create projection matrix
    projection = glm::perspective(glm::radians(45.0f),
        (float)myWindow.getWindowDimensions().width / (float)myWindow.getWindowDimensions().height,
        0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    // send light dir to shader
    lightDirLoc = glGetUniformLocation(shader.shaderProgram, "lightDir");
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightSource->getLightDir()));

    // send light color to shader
    lightColorLoc = glGetUniformLocation(shader.shaderProgram, "lightColor");
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightSource->getLightColor()));

    cameraPosLoc = glGetUniformLocation(shader.shaderProgram, "cameraPos");
    glUniform3fv(cameraPosLoc, 1, glm::value_ptr(myCamera.getCameraPosition()));

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
    //set the light direction (direction towards the light = position o flight source)
    glm::vec3 lightDir = glm::vec3(0.0f, 5.0f, 1.0f);
    //set light color => white light
    glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

    lightSource = new LightSource(lightDir, lightColor);
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

void mouseCallback(GLFWwindow* window, double xpos, double ypos) {
    if (!(firstMouseMovement || allowMouseMovements)) {
        return;
    }
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
    pitch += yoffset;

    // the viewer cannot turn his head on 360*, only up until 90* and down until -90*
    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

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
}

void initOpenGLState() {
    glClearColor(0.7f, 0.7f, 0.7f, 1.0f);
    glViewport(0, 0, myWindow.getWindowDimensions().width, myWindow.getWindowDimensions().height);
    glEnable(GL_FRAMEBUFFER_SRGB);
    glEnable(GL_DEPTH_TEST); // enable depth-testing
    glDepthFunc(GL_LESS); // depth-testing interprets a smaller value as "closer"
    glEnable(GL_CULL_FACE); // cull face
    glCullFace(GL_BACK); // cull back face
    glFrontFace(GL_CCW); // GL_CCW for counter clock-wise
}
