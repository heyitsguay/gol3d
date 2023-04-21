//
// Created by matt on 1/26/16.
//

#ifndef GOL3D_SKYBOX_H
#define GOL3D_SKYBOX_H
#pragma once

#include <vector>

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "load_shader.h"

// Aliases for drawState values.
#define DRAW_BOX 0
#define DRAW_BLACK 1
#define DRAW_WHITE 2

class Skybox {
private:
    // Identity model matrix.
    const glm::mat4 M = glm::mat4(1.f);

    // Pointer to the Camera.
    Camera *cam;

    // The IO object.
    IO &io;

    void handleInput();

    GLuint loadCubemap(std::vector<const GLchar*> faces);

public:
    // ID to the Skybox's cubemap texture.
    GLuint tex;

    // ID to the Skybox's shader program.
    GLuint *program;

    // ID to the Skybox's MVP uniform.
    GLuint uMVP;

    // Skybox scale parameter.
    float scale;

    // Skybox scaling vector.
    glm::vec3 scaleVec;

//    // Only draw the Skybox if this is true.
//    bool drawThis;

    // Indicates the draw state of the Skybox - Skybox, all black, or all white.
    char drawState;

    Skybox();

    ~Skybox();

    void init(GLuint *program_,
              Camera *cam_,
              float scale_=10000.f,
              bool useHD = true);

    void draw(int activeCubes);

};

#endif //GOL3D_SKYBOX_H
