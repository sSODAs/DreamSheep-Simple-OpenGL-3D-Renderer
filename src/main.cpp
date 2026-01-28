#undef GLFW_DLL
#include <iostream>
#include <stdio.h>
#include <string>
#include <string.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include <vector>
#include <cmath>

#include "Libs/Shader.h"
#include "Libs/Window.h"
#include "Libs/Mesh.h"
#include "Libs/stb_image.h"
#include <glm/ext/matrix_float4x4.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <utils.h>

const GLint WIDTH = 800, HEIGHT = 600;

Window mainWindow;
glm::vec3 lightColour = glm::vec3(1.0f, 1.0f, 1.0f);

glm::vec3 cameraPos = glm::vec3(0.889346, 0.0855256, 2.65506);
glm::vec3 cameraDirection = glm::vec3(0.253454f, 0.854458f, -0.453499f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));
glm::vec3 lightPos = glm::vec3(5.0f, 5.0f, 0.0f);

static const char *vStarShader = "Shaders/star.vert";
static const char *fStarShader = "Shaders/star.frag";
static const char *vTestShader = "Shaders/test.vert";
static const char *fTestShader = "Shaders/test.frag";

float yaw = -90.0f;
float pitch = 0.0f;
float lastX = 400, lastY = 300;
bool mouse = true;
float sensitivity = 0.1f;

void mouse_callback(GLFWwindow *window, double xpos, double ypos)
{
    if (mouse)
    {
        lastX = xpos;
        lastY = ypos;
        mouse = false;
    }

    float xoffset = xpos - lastX;
    float yoffset = lastY - ypos;
    lastX = xpos;
    lastY = ypos;

    xoffset *= sensitivity;
    yoffset *= sensitivity;

    yaw += xoffset;
    pitch += yoffset;

    if (pitch > 89.0f)
        pitch = 89.0f;
    if (pitch < -89.0f)
        pitch = -89.0f;

    glm::vec3 front;
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    cameraDirection = glm::normalize(front);
}

void DrawCube(Object *obj, glm::mat4 view, glm::mat4 projection, glm::vec3 position)
{
    glUniform1i(obj->shader->GetUniformLocation("texture1"), 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, obj->texture);
    GLuint uniformModel = 0, uniformProjection = 0, uniformView = 0, uniformLightColor = 0, uniformLightPos = 0, uniformViewPos = 0;

    uniformModel = obj->shader->GetUniformLocation("model");
    uniformProjection = obj->shader->GetUniformLocation("projection");
    uniformView = obj->shader->GetUniformLocation("view");
    uniformLightColor = obj->shader->GetUniformLocation("lightColour");
    uniformLightPos = obj->shader->GetUniformLocation("lightPos");
    uniformViewPos = obj->shader->GetUniformLocation("viewPos");

    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(0.8f, 0.8f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));

    glUniform3fv(uniformLightColor, 1, glm::value_ptr(lightColour));
    glUniform3fv(uniformLightPos, 1, glm::value_ptr(lightPos));
    glUniform3fv(uniformViewPos, 1, glm::value_ptr(cameraPos));
    obj->mesh->RenderMesh();
}

void DrawBackground(Object *obj, glm::mat4 view, glm::mat4 projection, glm::vec3 position)
{
    GLuint uniformModel = 0, uniformProjection = 0, uniformView = 0, uniformLightColor = 0, uniformLightPos = 0, uniformViewPos = 0, uniformBgColor = 0, uniformTime = 0;

    uniformModel = obj->shader->GetUniformLocation("model");
    uniformProjection = obj->shader->GetUniformLocation("projection");
    uniformView = obj->shader->GetUniformLocation("view");
    uniformLightColor = obj->shader->GetUniformLocation("lightColour");
    uniformLightPos = obj->shader->GetUniformLocation("lightPos");
    uniformViewPos = obj->shader->GetUniformLocation("viewPos");
    uniformBgColor = obj->shader->GetUniformLocation("bg_color");
    uniformTime = obj->shader->GetUniformLocation("time");

    glm::vec4 bgColor(0.094f, 0.173f, 0.235f, 1.0f);

    glUniformMatrix4fv(uniformView, 1, GL_FALSE, glm::value_ptr(view));
    glm::mat4 model(1.0f);
    model = glm::translate(model, position);
    model = glm::scale(model, glm::vec3(30.0f, 30.0f, 1.0f));
    glUniformMatrix4fv(uniformModel, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(uniformProjection, 1, GL_FALSE, glm::value_ptr(projection));
    glUniform4fv(uniformBgColor, 1, glm::value_ptr(bgColor));
    glUniform1f(uniformTime, glfwGetTime());
    glUniform3fv(uniformLightColor, 1, glm::value_ptr(lightColour));
    glUniform3fv(uniformLightPos, 1, glm::value_ptr(lightPos));
    glUniform3fv(uniformViewPos, 1, glm::value_ptr(cameraPos));
    obj->mesh->RenderMesh();
}

int main()
{
    mainWindow = Window(WIDTH, HEIGHT, 3, 3);
    mainWindow.initialise();
    float deltaTime = 0.0f;
    float lastTime = 0.0f;

    Shader *testShader = PrepareShader(vTestShader, fTestShader);
    std::cout << "Preparing scene..." << std::endl;

    Shader *starShader = PrepareShader(vStarShader, fStarShader);
    unsigned int star = PrepareObjectFromMesh(PreparePlane(), "");

    unsigned int scene = PrepareObject("Models/Final.obj", "Textures/Final.png");
    objects[scene].shader = testShader;
    objects[scene].drawPrt = DrawCube;

    unsigned int star_background = PrepareObjectFromMesh(PreparePlane(), "");
    objects[star_background].shader = starShader;
    objects[star_background].drawPrt = DrawBackground;

    glm::mat4 projection = glm::perspective(45.0f, (GLfloat)mainWindow.getBufferWidth() / (GLfloat)mainWindow.getBufferHeight(), 0.1f, 100.0f);
    // glm::mat4 projection = glm::ortho(-4.0f, 4.0f, -3.0f, 3.0f, 0.1f, 100.0f);
    // Loop until window closed
    std::cout << "Starting main loop..." << std::endl;
    while (!mainWindow.getShouldClose())
    {

        float currentTime = static_cast<float>(glfwGetTime());
        deltaTime = currentTime - lastTime;
        lastTime = currentTime;
        // Get + Handle user input events
        glfwPollEvents();
        glfwSetCursorPosCallback(mainWindow.getWindow(), mouse_callback);

        // Clear window
        glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_W) == GLFW_PRESS)
        {
            cameraPos += cameraDirection * 5.0f * deltaTime;
        }

        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_S) == GLFW_PRESS)
        {
            cameraPos -= cameraDirection * 5.0f * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_A) == GLFW_PRESS)
        {
            cameraPos -= cameraRight * 5.0f * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_D) == GLFW_PRESS)
        {
            cameraPos += cameraRight * 5.0f * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
        {
            cameraPos.y -= 5.0f * deltaTime;
        }
        if (glfwGetKey(mainWindow.getWindow(), GLFW_KEY_SPACE) == GLFW_PRESS)
        {
            cameraPos.y += 5.0f * deltaTime;
        }

        cameraRight = glm::normalize(glm::cross(cameraDirection, cameraUp));

        glm::mat4 view(1.0f);
        view = glm::lookAt(cameraPos, cameraPos + cameraDirection, cameraUp);
        // display camera pos, camera direction, camera up
        // std::cout << "Camera Pos: (" << cameraPos.x << ", " << cameraPos.y << ", " << cameraPos.z << ") " << std::endl;

        // draw here
        objects[scene].UseShader();
        objects[scene].Draw(view, projection, glm::vec3(0.0f, -2.0f, -7.5f));

        objects[star_background].UseShader();
        objects[star_background].Draw(view, projection, glm::vec3(0.0f, 0.0f, -12.0f));

        glUseProgram(0);
        // end draw

        mainWindow.swapBuffers();
    }

    return 0;
}
