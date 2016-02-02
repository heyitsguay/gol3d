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

    // Cube center coordinates.
    int x, y, z;

    // Cube center, vector form.
    glm::ivec3 center;

    // Cube state: 0 = dead, 1 = live, 2 = dying.
    char state;
    
    // Number of live neighbors.
    char live_neighbors;

    // Type (determines texture).
    int type;

    // Texture coordinate base.
    glm::ivec2 texBase;

    Cube();

    ~Cube();

    // Call to set up the Cube.
    void setup(int x_, int y_, int z_, int type_=T_BORDERED);
};

#endif //GOL3D_CUBE_H
