#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>
#include <memory>

#include "Shader.h"
#include "Model.h"
#include "cyTriMesh.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);
void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
void ManageTextures(GLuint* textures, cyTriMesh ctm, size_t materialNum);
void SetTextureData(const char* name);

#pragma region parameters
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 800;

double xPos = SCR_WIDTH / 2.0f, yPos = SCR_HEIGHT / 2.0f; //mouse movement
bool ShouldRotateModel = false;
bool ShouldMoveFromCamera = false;
const float mouseSensitivity = 0.5f;
float cameraDistance = 0;

bool ShouldRotateLight = false;
float yaw_light = 0;
float pitch_light = 0;

Model* model;
#pragma endregion

int main() {
    std::cout << "Enter the name of the obj model: ";
    std::string fileName;
    std::getline(std::cin, fileName);

    cyTriMesh ctm;
    if (!ctm.LoadFromFileObj((fileName + ".obj").c_str())) {
        return -1;
    }
    
#pragma region OpenGL Initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Model Viewer", NULL, NULL);
    if (window == NULL)
    {
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, mouseMovementCallback);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    glEnable(GL_DEPTH_TEST);
#pragma endregion

#pragma region TRANSFORMATION
    glm::mat4 projection = glm::mat4(1.0f); 
    glm::mat4 view = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)(SCR_WIDTH / SCR_HEIGHT), 0.1f, 500.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -30.0f));
    
    glm::vec4 lightPosition = glm::vec4(0, 20, 200, 1);
    glm::mat4 lightRotation = glm::mat4(1.0f);
#pragma endregion

    Shader shader("VertexShader.vs", "FragmentShader.fs");
    model = new Model(ctm, shader, view, projection);

#pragma region render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        lightRotation = glm::mat4(1.0f);
        lightRotation = glm::translate(lightRotation, glm::vec3(0, 0, -cameraDistance));
        lightRotation = glm::rotate(lightRotation, glm::radians(yaw_light), glm::vec3(0, 1, 0));
        lightRotation = glm::rotate(lightRotation, glm::radians(pitch_light), glm::vec3(1, 0, 0));
        model->setLightPosition(lightRotation * lightPosition);

        model->Draw();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#pragma endregion

    delete model;
    glfwTerminate();
    return 0;
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void processInput(GLFWwindow* window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        glfwSetWindowShouldClose(window, true);
    }
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
        //light rotation
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            ShouldRotateLight = true;
            glfwGetCursorPos(window, &xPos, &yPos);
        }
    }
    else {
        //model rotation
        if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS) {
            ShouldRotateModel = true;
            glfwGetCursorPos(window, &xPos, &yPos);
        }
    }
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE) {
        ShouldRotateLight = false;
        ShouldRotateModel = false;
    }

    //model distance to camera
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
        ShouldMoveFromCamera = true;
        glfwGetCursorPos(window, &xPos, &yPos);
    }
    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
        ShouldMoveFromCamera = false;
    }
}

void mouseMovementCallback(GLFWwindow* window, double xpos, double ypos) {
    float xoffset;
    float yoffset;
    xoffset = xpos - xPos;
    yoffset = ypos - yPos;
    xPos = xpos;
    yPos = ypos;
    xoffset *= mouseSensitivity;
    yoffset *= mouseSensitivity;
    if (ShouldRotateModel) {
        model->rotate(xoffset, yoffset);
    }

    if (ShouldRotateLight) {
        yaw_light += xoffset;
        pitch_light += yoffset;
    }

    if (ShouldMoveFromCamera) {
        cameraDistance += yoffset;
        model->adjustDistanceFromCamera(yoffset);
    }
}
