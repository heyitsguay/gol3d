//
// Created by matt on 1/28/16.
//

#ifndef GOL3D_APPLICATION_H
#define GOL3D_APPLICATION_H
#pragma once

//#define _GLDEBUG

#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Skybox.h"
#include "User.h"

// Aliases for quality levels.
#define QUALITY_LOW 0 // Currently doesn't work.
#define QUALITY_MEDIUM 1 // Currently doesn't work.
#define QUALITY_HIGH 2
#define QUALITY_LAPTOP -1 // Just for my little 1366x768 laptop.

// The application.
class Application {
private:
    Application();
    Application(Application const&);
    void operator=(Camera const&);

    // When true, the Application prints performance info.
    bool printPerfInfo = false;

    void freeGL();

    void handleInput();

    void initGL(int monitorID, int quality, int aaSamples);

    void perfInfo();

public:
    static Application &getInstance() {
        static Application instance;
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

    void init(int monitorID=0, int quality=QUALITY_HIGH, int aaSamples=2);

    void terminate();

    void update();
};

#endif //GOL3D_APPLICATION_H
