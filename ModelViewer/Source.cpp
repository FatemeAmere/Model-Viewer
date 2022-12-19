#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "Shader.h"
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
float yaw = 0;
float pitch = 0;
float cameraDistance = 0;

bool ShouldRotateLight = false;
float yaw_light = 0;
float pitch_light = 0;
#pragma endregion

int main() {
    std::cout << "Enter the name of the obj model: ";
    std::string fileName;
    std::cin >> fileName;
    cyTriMesh ctm;
    if (!ctm.LoadFromFileObj((fileName+".obj").c_str())) {
        return -1;
    }

#pragma region OpenGL Initialization
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, false);

    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Review", NULL, NULL);
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

#pragma region VERTEX
    ctm.ComputeBoundingBox();
    cyVec3f boundMiddle = (ctm.GetBoundMax() + ctm.GetBoundMin()) / 2.0f;
    float ModelSize = (ctm.GetBoundMax() - ctm.GetBoundMin()).Length() / 20.0f;

    std::vector<float> vertices;
    for (size_t i = 0; i < ctm.NF(); i++)
    {
        for (size_t j = 0; j < 3; j++)
        {
            //vertices
            vertices.push_back((ctm.V(ctm.F(i).v[j]).x - boundMiddle.x) / ModelSize);
            vertices.push_back((ctm.V(ctm.F(i).v[j]).y - boundMiddle.y) / ModelSize);
            vertices.push_back((ctm.V(ctm.F(i).v[j]).z - boundMiddle.z) / ModelSize);
            //normals
            vertices.push_back(ctm.VN(ctm.FN(i).v[j]).x);
            vertices.push_back(ctm.VN(ctm.FN(i).v[j]).y);
            vertices.push_back(ctm.VN(ctm.FN(i).v[j]).z);
            //texture coordinates
            vertices.push_back(ctm.VT(ctm.FT(i).v[j]).x);
            vertices.push_back(ctm.VT(ctm.FT(i).v[j]).y);
        }
    }
    GLuint VBO, VAO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(float), vertices.data(), GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
#pragma endregion

#pragma region Materials and Textures
    stbi_set_flip_vertically_on_load(true);
    unsigned int materialCount = ctm.NM();  

    std::vector<GLuint*> textures;

    for (size_t i = 0; i < materialCount; i++)
    {
        GLuint* textureTmp = new GLuint[5];
        ManageTextures(textureTmp, ctm, i);
        textures.push_back(textureTmp);
    }

    Shader shader("VertexShader.vs", "FragmentShader.fs");
    shader.use();
    shader.setInt("ambientTexture", 0);
    shader.setInt("diffuseTexture", 1);
    shader.setInt("specularTexture", 2);
    shader.setInt("specularExponentTexture", 3);
    shader.setInt("alphaTexture", 4); 
#pragma endregion

#pragma region TRANSFORMATION
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::mat4(1.0f);
    view = glm::translate(view, glm::vec3(0.0f, 0.0f, -30.0f));
    glm::mat4 projection = glm::mat4(1.0f);
    projection = glm::perspective(glm::radians(45.0f), (float)(SCR_WIDTH / SCR_HEIGHT), 0.1f, 500.0f);

    glm::vec4 lightPosition = glm::vec4(0, 20, 200, 1);
    glm::mat4 lightRotation = glm::mat4(1.0f);
#pragma endregion

#pragma region render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);

        glClearColor(0, 0, 0, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(0, 0, -cameraDistance));
        model = glm::rotate(model, glm::radians(pitch), glm::vec3(1, 0, 0));
        model = glm::rotate(model, glm::radians(yaw), glm::vec3(0, 1, 0));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1, 0, 0));

        lightRotation = glm::mat4(1.0f);
        lightRotation = glm::translate(lightRotation, glm::vec3(0, 0, -cameraDistance));
        lightRotation = glm::rotate(lightRotation, glm::radians(yaw_light), glm::vec3(0, 1, 0));
        lightRotation = glm::rotate(lightRotation, glm::radians(pitch_light), glm::vec3(1, 0, 0));

        shader.use();
        shader.setMat4("mvp", projection * view * model);
        shader.setMat4("mv", view * model);
        shader.setMat4("mvN", glm::transpose(glm::inverse(view * model)));   //mv for Normals
        shader.setVec3("lightPosition", view * lightRotation * lightPosition);

        for (size_t i = 0; i < materialCount; i++)
        {
            GLuint* t = textures[i];
            shader.setBool("usetexa", true);
            shader.setBool("usetexd", true);
            shader.setBool("usetexs", true);
            shader.setBool("usetexsc", true);

            if (!ctm.M(i).map_Ka) {
               shader.setVec3("ka", glm::vec3(ctm.M(i).Ka[0], ctm.M(i).Ka[1], ctm.M(i).Ka[2]));
               shader.setBool("usetexa", false);
            }
            if (!ctm.M(i).map_Kd) {
                shader.setVec3("kd", glm::vec3(ctm.M(i).Kd[0], ctm.M(i).Kd[1], ctm.M(i).Kd[2]));
                shader.setBool("usetexd", false);           
            }
            if (!ctm.M(i).map_Ks) {
                shader.setVec3("ks", glm::vec3(ctm.M(i).Ks[0], ctm.M(i).Ks[1], ctm.M(i).Ks[2]));
                shader.setBool("usetexs", false);
            } 
            if (!ctm.M(i).map_Ns) {
                shader.setFloat("specularExponent", ctm.M(i).Ns);
                shader.setBool("usetexsc", false);
            }

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, t[0]);
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_2D, t[1]);
            glActiveTexture(GL_TEXTURE2);
            glBindTexture(GL_TEXTURE_2D, t[2]);
            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_2D, t[3]);
            glActiveTexture(GL_TEXTURE4);
            glBindTexture(GL_TEXTURE_2D, t[4]);

            glBindVertexArray(VAO);
            glDrawArrays(GL_TRIANGLES, ctm.GetMaterialFirstFace(i) * 3, ctm.GetMaterialFaceCount(i) * 3);
        }

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
#pragma endregion

    for (size_t i = 0; i < materialCount; i++)
    {
        delete textures[i];
    }
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);

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
        yaw += xoffset;
        pitch += yoffset;
    }

    if (ShouldRotateLight) {
        yaw_light += xoffset;
        pitch_light += yoffset;
    }

    if (ShouldMoveFromCamera) {
        cameraDistance += yoffset;
    }
}

void ManageTextures(GLuint* textures, cyTriMesh ctm, size_t materialNum) {
    if (ctm.M(materialNum).map_Ka) {
        glGenTextures(1, &textures[0]);
        glBindTexture(GL_TEXTURE_2D, textures[0]);
        SetTextureData(ctm.M(materialNum).map_Ka);
    }
    if (ctm.M(materialNum).map_Kd) {
        glGenTextures(1, &textures[1]);
        glBindTexture(GL_TEXTURE_2D, textures[1]);
        SetTextureData(ctm.M(materialNum).map_Kd);
    }
    if (ctm.M(materialNum).map_Ks) {
        glGenTextures(1, &textures[2]);
        glBindTexture(GL_TEXTURE_2D, textures[2]);
        SetTextureData(ctm.M(materialNum).map_Ks);
    }
    if (ctm.M(materialNum).map_Ns) {
        glGenTextures(1, &textures[3]);
        glBindTexture(GL_TEXTURE_2D, textures[3]);
        SetTextureData(ctm.M(materialNum).map_Ns);
    }
    if (ctm.M(materialNum).map_d) {
        glGenTextures(1, &textures[4]);
        glBindTexture(GL_TEXTURE_2D, textures[4]);
        SetTextureData(ctm.M(materialNum).map_d);
    }
}

void SetTextureData(const char* name) {
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    int width, height, nrChannels;
    unsigned char* data = stbi_load(name, &width, &height, &nrChannels, 0);
    if (data)
    {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else
    {
        std::cout << "Failed to load texture " << name << std::endl;
    }
    stbi_image_free(data);
}