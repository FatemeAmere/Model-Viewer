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

std::vector<Model*> models;
bool showCubeMap = false;
const char* cubemapImageNames[6] = {
    "CubeMapTextures/right.jpg", "CubeMapTextures/left.jpg", 
    "CubeMapTextures/top.jpg", "CubeMapTextures/bottom.jpg",
    "CubeMapTextures/front.jpg", "CubeMapTextures/back.jpg"
};
#pragma endregion

int main() {
    //cubemap
    std::string fileName = "Models/cube";
    cyTriMesh cubeMap_ctm;
    if (!cubeMap_ctm.LoadFromFileObj((fileName + ".obj").c_str())) {
        return -1;
    }

   /* std::cout << "Enter the name of the obj model: ";
    std::getline(std::cin, fileName);*/
    cyTriMesh ctm;
    fileName = "teapot";
    if (!ctm.LoadFromFileObj(("Models/" + fileName + ".obj").c_str())) {
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
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
#pragma endregion

#pragma region TRANSFORMATION
    glm::vec3 cameraPos = glm::vec3(0, 0, 30.0f);
    glm::mat4 projection = glm::mat4(1.0f); 
    projection = glm::perspective(glm::radians(45.0f), (float)(SCR_WIDTH / SCR_HEIGHT), 0.1f, 500.0f);
    
    glm::vec4 lightPosition = glm::vec4(0, 20, 200, 1);
    glm::mat4 lightRotation = glm::mat4(1.0f);
#pragma endregion

#pragma region cubeMap
    Shader cubeMapShader("VertexShader.vs", "CubeMapFragmentShader.fs");
    models.push_back(new Model(cubeMap_ctm, cubeMapShader, cameraPos, glm::vec3(20.0f, 20.0f, 20.0f), cameraPos, projection, false, true, false));

    //textures
    stbi_set_flip_vertically_on_load(false);
    GLuint cubeMapTexID;
    glGenTextures(1, &cubeMapTexID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexID);
    int width, height, nrChannels;
    unsigned char* data;
    for (unsigned int i = 0; i < 6; i++)
    {
        data = stbi_load(cubemapImageNames[i], &width, &height, &nrChannels, 0);
        if (data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        }
        else
        {
            std::cout << "Failed to load texture " << cubemapImageNames[0] << std::endl;
        }
        stbi_image_free(data);
    }
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR); 
#pragma endregion

#pragma region planeUnderModel
    //plane under the object
    float vertices[] = {
        0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
        -0.5f, -0.5f, 0.0f, 0.0f, 0.0f, 1.0f,
         -0.5f,  0.5f, 0.0f, 0.0f, 0.0f, 1.0f
    };
    unsigned int indices[] = {
        0, 1, 3,
        1, 2, 3
    };
    GLuint VAO, VBO, EBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 6 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    Shader planeShader("VertexShader.vs", "PlaneFragmentShader.fs");
    planeShader.use();
    planeShader.setInt("cubemap", 5);
    planeShader.setBool("useCubeMap", false);
#pragma endregion  

    //model
    Shader otherShader("VertexShader.vs", "FragmentShader.fs");
    models.push_back(new Model(ctm, otherShader, glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), cameraPos, projection, true, true, true));
    //std::cout << ctm.M(0).name;
    //models.push_back(new Model(ctm, otherShader, glm::vec3(3.0f, 0.0f, 0.0f), glm::vec3(0.5f, 0.5f, 0.5f), cameraPos, projection, true, true, true));

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

        for(Model * m : models)
        {
            m->setLightPosition(lightRotation * lightPosition);
        }
        if (showCubeMap) {
            glActiveTexture(GL_TEXTURE5);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexID);
            models[1]->setCubeMapOrientation(models[0]->getModelViewMatrix());
            //models[2]->setCubeMapOrientation(models[0]->getModelViewMatrix());
        } 
        models[1]->ToggleCubeMapReflections(showCubeMap);
        models[1]->Draw();
        //models[2]->ToggleCubeMapReflections(showCubeMap);
        //models[2]->Draw();
        
        #pragma region planeUnderModel
        planeShader.use();
        glm::mat4 mod = glm::mat4(1.0f);
        mod = glm::translate(mod, glm::vec3(0, -2.0f, 0));
        mod = glm::rotate(mod, glm::radians(-90.0f), glm::vec3(1, 0, 0));
        mod = glm::scale(mod, glm::vec3(15, 15, 15));
        glm::mat4 view = models[1]->getViewMatrix();
        planeShader.setMat4("m", mod);
        planeShader.setMat4("mvp", projection * view * mod);
        planeShader.setMat4("mv", view * mod);
        planeShader.setMat4("mvN", glm::transpose(glm::inverse(view * mod)));   //mv for Normals
        planeShader.setMat4("cubeMapOrientation", models[0]->getModelViewMatrix());
        planeShader.setBool("useCubeMap", showCubeMap);
        planeShader.setVec3("lightPosition", lightPosition);
        planeShader.setVec3("ks", glm::vec3(1, 1, 1));
        planeShader.setVec3("ka", glm::vec3(0, 0, 0));
        planeShader.setVec3("kd", glm::vec3(0, 0, 0));
        planeShader.setFloat("specularExponent", 1000);
        
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        #pragma endregion  

        if (showCubeMap) {
            glDepthMask(GL_FALSE);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTexID);
            models[0]->Draw();

            glDepthMask(GL_TRUE);
        }


        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#pragma endregion

    for (Model* m : models)
    {
        delete m;
    }   
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

    if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS) {
        showCubeMap = true;
    }

    if (glfwGetKey(window, GLFW_KEY_V) == GLFW_PRESS) {
        showCubeMap = false;
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
        //cubeModel->rotate(xoffset, yoffset);
        for (Model* m : models)
        {
            m->rotate(xoffset, yoffset);
        }
    }

    if (ShouldRotateLight) {
        yaw_light += xoffset;
        pitch_light += yoffset;
    }

    if (ShouldMoveFromCamera) {
        cameraDistance += yoffset;
        //cubeModel->adjustDistanceFromCamera(yoffset);
        for (Model* m : models)
        {
            m->adjustDistanceFromCamera(yoffset);
        }
    }
}
