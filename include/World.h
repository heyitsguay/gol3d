//
// Created by matt on 1/31/16.
//

#ifndef GOL3D_WORLD_H
#define GOL3D_WORLD_H
#pragma once

#include <vector>

#include <GL/glew.h>

#include "Camera.h"
#include "CellularAutomaton.h"
#include "GeneralizedCellularAutomaton.h"
#include "Object.h"

class World {
private:
    // Reference to the IO handler.
    IO &io;

    // Reference to the Camera.
    Camera &cam;

    // Pointer to the main Cube VAO.
    GLuint *cubeVAO;

    // Pointer to the World shader program.
    GLuint *program;

public:
    // Vector containing all the Objects in the World.
    std::vector<Object*> objects;

    // Pointer to the currently-active Object.
    Object *activeObject;

    // Arrays containing drawn Cube translation, scaling, and type info.
    std::vector<glm::vec3> translations;
    std::vector<float> scales;
    std::vector<glm::ivec2> types;

    // Base Cube hue.
    float baseCubeH = 0.7f;

    // Determines whether to use flat shading or not.
    float varyColor = 1.f;

    // OpenGL uniform variables.
    GLuint uMVP, uvaryColor, uhBase, ucameraPos, ut;

    // OpenGL VBOs for translation, scaling, and type info.
    GLuint translationVBO, scaleVBO, typeVBO;

    // Counts the actual number of Cubes drawn each frame.
    int drawCount = 0;

    // ID of the Cube texture atlas.
    GLuint atlasTex;

    // ID of the Cube texture sampler uniform
    GLuint sAtlas;

    World();
    ~World();

    void activate(Object *obj);

    void draw(float t);

    void handleInput();

    void init(GLuint *cubeVAO_, GLuint *program_);

    void initGL();

    void update();
};

#endif //GOL3D_WORLD_H
