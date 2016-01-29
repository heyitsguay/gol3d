//
// Created by matt on 1/28/16.
//

#ifndef GOL3D_FRAMEWORK_H
#define GOL3D_FRAMEWORK_H
#pragma once

//#define _GLDEBUG

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Skybox.h"
#include "User.h"

// Framework for the GOL3D application.
class Framework {
private:
    Framework();
    Framework(Framework const&);
    void operator=(Camera const&);

    // When true, the Framework prints performance info.
    bool printPerfInfo = false;
public:
    static Framework &getInstance() {
        static Framework instance;
        return instance;
    }

    // The IO handler.
    IO &io;

    // The Camera.
    Camera &cam;

    // The Skybox.
    Skybox skybox;

    // The World.
    World world;

    // The User.
    User user;

    // Time since initialization.
    double t;

    // The main Cube vertices.
    std::vector<glm::vec3> cubeVertices;

    // The main Cube normals.
    std::vector<glm::vec3> cubeNormals;

    // The display window.
    GLFWwindow *window;

    // Cube Vertex Array Object (VAO).
    GLuint cubeVAO;

    // Cube vertex Vertex Buffer Object (VBO).
    GLuint vertexVBO;

    // Cube normal VBO.
    GLuint normalVBO;

    // Shader programs (SP).
    GLuint worldSP, cursorSP, skyboxSP;

    void draw();

    void freeGL();

    void handleInput();

    void init();

    void initGL();

    void perfInfo();

    void update();
};

#endif //GOL3D_FRAMEWORK_H
