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

#include <iostream>

// window
gps::Window myWindow;
int myWindowWidth = 1024;
int myWindowHeight = 768;
int retina_width, retina_height;

// matrices
glm::mat4 model;
glm::mat4 view;
glm::mat4 projection;
glm::mat3 normalMatrix;

// Animations
Animation objectAnimation;

// initial position of objects and camera
glm::vec3 objectInitialPosition = glm::vec3(0.0f, 0.0f, -10.0f);
glm::vec3 cameraInitialPosition = glm::vec3(0.0f, 0.0f, 3.0f);

// light parameters
glm::vec3 lightDir;
glm::vec3 lightColor;

GLfloat lightAngle;

// shader uniform locations
GLint modelLoc;
GLint viewLoc;
GLint projectionLoc;
GLint normalMatrixLoc;
GLint lightDirLoc;
GLint lightColorLoc;

// camera
gps::Camera myCamera(
    cameraInitialPosition,
    objectInitialPosition,
    glm::vec3(0.0f, 1.0f, 0.0f));

/* uniform camera movement taking into consideration the frequency of the rendered frames */
GLfloat cameraSpeed = 0.1f;
float deltaTime = 0.0f;	// Time between current frame and last frame
float lastFrame = 0.0f; // Time of last frame

GLboolean pressedKeys[1024];
GLfloat angle = 0.0f;

// models
gps::Model3D teapot;

// shaders
gps::Shader basicShader;

// check errors
GLenum glCheckError_(const char* file, int line);
#define glCheckError() glCheckError_(__FILE__, __LINE__)

/* attributes for updating the mouse coordinates within the screen frame */
float lastX = myWindowWidth/2, lastY = myWindowWidth/2;
float pitch = 0.0f, yaw = -90.0f, rollAngle = angle;
float angleX = 0.0f, angleY = 0.0f;
float mouseSensitivity = 0.1f;
bool firstMouseMovement = true;
bool allowMouseMovements = false;

/* initialize window, matrices and shaders, and add callbacks */
void initOpenGLWindow();
void setWindowCallbacks();
void initOpenGLState();
void initModels();
void initShaders();
void initUniforms(gps::Shader shader);
void initAnimations();

/* functions for processing movement actions */
void processMovement();
void processObjectMovement();
void processCameraMovement();

/* process objectAnimation movement */
void processAnimations();
void processObjectAnimation();

/* functions for updating the transformation matrices after the camera or the object has moved*/
void updateUniforms();
void updateTransformationMatrices();
void updateModelAfterObjectRotation(float angle);

/* render scene of objects */
void renderScene();
void renderTeapot(gps::Shader shader);

/* callback functions for handling user interactions */
void windowResizeCallback(GLFWwindow* window, int width, int height);
void keyboardCallback(GLFWwindow* window, int key, int scancode, int action, int mode);
void mouseCallback(GLFWwindow* window, double xpos, double ypos);
void mousButtonCallback(GLFWwindow* window, int button, int action, int mods);

/* clean-up*/
void cleanup();

int main(int argc, const char * argv[]) {

    try {
        initOpenGLWindow();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    initOpenGLState();
	initModels();
	initShaders();
	initUniforms(basicShader);
    initAnimations();
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
    processAnimations();
}

void processCameraMovement() {
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] || pressedKeys[GLFW_KEY_RIGHT_SHIFT]) {
        // combination of Shift + key is for controlling the objectAnimations
        return;
    }
    cameraSpeed = 2.5f * deltaTime;
    if (pressedKeys[GLFW_KEY_W]) {
        myCamera.move(gps::MOVE_FORWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_S]) {
        myCamera.move(gps::MOVE_BACKWARD, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_A]) {
        myCamera.move(gps::MOVE_LEFT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_D]) {
        myCamera.move(gps::MOVE_RIGHT, cameraSpeed);
    }
    if (pressedKeys[GLFW_KEY_I]) {
        // todo: invisible
    }
}

void processObjectMovement() {
    if (pressedKeys[GLFW_KEY_Q]) {
        angle -= 1.0f;
        updateModelAfterObjectRotation(angle);
    }
    if (pressedKeys[GLFW_KEY_E]) {
        angle += 1.0f;
        updateModelAfterObjectRotation(angle);
    }
}

void updateModelAfterObjectRotation(float angle) {
    model = glm::rotate(glm::mat4(1.0), glm::radians(angle), glm::vec3(0, 1, 0));
}

void processAnimations() {
    processObjectAnimation();
}

void processObjectAnimation() {
    // use left shift for object
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_B]) {
        float elasticity = 0.5;
        float speed = 5.0;
        objectAnimation.setAnimationSpeed(speed);
        objectAnimation.setTransformationMatrix(model);
        objectAnimation.startBounceAnimation(elasticity);
    }
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_S]) {
        float speed = 5.0;
        glm::vec3 axis = glm::vec3(0,1,0); // y axis
        float dampingFactor = 0.5;
        objectAnimation.setAnimationSpeed(speed);
        objectAnimation.setTransformationMatrix(model);
        objectAnimation.startSpinAnimation(axis, dampingFactor);
    }
    if (pressedKeys[GLFW_KEY_LEFT_SHIFT] && pressedKeys[GLFW_KEY_Z]) {
        // stop object animation
        objectAnimation.stopAnimation();
    }
    objectAnimation.playAnimation();
}

void updateTransformationMatrices() {
    // update model matrix
    if (objectAnimation.isAnimationPlaying()) {
        model = objectAnimation.getTransformationMatrix();
    }
    //update view matrix
    view = myCamera.getViewMatrix();
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void updateUniforms() {
    //send model matrix data to shader
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    //send normal matrix data to shader
    glUniformMatrix3fv(normalMatrixLoc, 1, GL_FALSE, glm::value_ptr(normalMatrix));
    // send projection matrix data to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
    // send view matrix to shader
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}


void renderTeapot(gps::Shader shader) {
    // select active shader program
    shader.useShaderProgram();
    updateTransformationMatrices();
    updateUniforms();
    // draw teapot
    teapot.Draw(shader);
}

void renderScene() {
    /* update time variables after rendering the each frame, to create a uniform camera movement */
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    //render the scene
    renderTeapot(basicShader);
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

void initModels() {
    teapot.LoadModel("models/teapot/teapot20segUT.obj");
}

void initShaders() {
    basicShader.loadShader(
        "shaders/basic.vert",
        "shaders/basic.frag");
}

void initUniforms(gps::Shader shader) {
    shader.useShaderProgram();

    // create model matrix for teapot
    model = glm::rotate(glm::mat4(1.0f), glm::radians(angle), glm::vec3(0.0f, 1.0f, 0.0f));
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
        0.1f, 20.0f);
    projectionLoc = glGetUniformLocation(shader.shaderProgram, "projection");
    // send projection matrix to shader
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

    //set the light direction (direction towards the light)
    lightDir = glm::vec3(0.0f, 1.0f, 1.0f);
    //lightRotation = glm::rotate(glm::mat4(1.0f), glm::radians(lightAngle), glm::vec3(0.0f, 1.0f, 0.0f));
    lightDirLoc = glGetUniformLocation(shader.shaderProgram, "lightDir");
    // send light dir to shader
    glUniform3fv(lightDirLoc, 1, glm::value_ptr(lightDir));

    //set light color
    lightColor = glm::vec3(1.0f, 1.0f, 1.0f); //white light
    lightColorLoc = glGetUniformLocation(shader.shaderProgram, "lightColor");
    // send light color to shader
    glUniform3fv(lightColorLoc, 1, glm::value_ptr(lightColor));
}

void initAnimations() {
    objectAnimation.setInitialPosition(objectInitialPosition);
    objectAnimation.setTransformationMatrix(model);
}

/* callback functions */

void windowResizeCallback(GLFWwindow* window, int width, int height) {
    fprintf(stdout, "Window resized! New width: %d , and height: %d\n", width, height);
    glfwGetFramebufferSize(window, &retina_width, &retina_height);
    basicShader.useShaderProgram();
    projection = glm::perspective(glm::radians(45.0f), (float)retina_width / (float)retina_height, 0.1f, 1000.0f);
    projectionLoc = glGetUniformLocation(basicShader.shaderProgram, "projection");
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
    view = myCamera.getViewMatrix();
    // compute normal matrix for teapot
    normalMatrix = glm::mat3(glm::inverseTranspose(view * model));
}

void cleanup() {
    myWindow.Delete();
    //cleanup code for your own data
}

