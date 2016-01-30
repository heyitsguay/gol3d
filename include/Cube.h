//
// Created by mattguay on 1/23/16.
//

#ifndef GOL3D_CUBE_H
#define GOL3D_CUBE_H
#pragma once

#include <tuple>

#include <glm/glm.hpp>

#include "cubeTypes.h"

class Cube {
public:
    // Index of Cube
//    long int idx;

    // Cube center coordinates.
    int x, y, z;

    // Cube state: false = dead, true = alive.
    bool live;
    
    // Number of live neighbors.
    char live_neighbors;

    // Type (determines texture).
    int type;

    // Texture coordinate base.
    glm::ivec2 texBase;

    // Time-since-alive multiplier. 1 when state == alive.
    float mult;

    Cube();

    ~Cube();

    // Call to set up the Cube.
    void setup(int x_, int y_, int z_, int type_=T_BORDERED);
};

#endif //GOL3D_CUBE_H
