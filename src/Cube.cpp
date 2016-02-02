//
// Created by mattguay on 1/23/16.
//

#include "Cube.h"

Cube::Cube() {};

Cube::~Cube() {};

void Cube::setup(int x_, int y_, int z_, int type_) {
// Sets up a Cube at a specific location, with a given state (default false).
    x = x_;
    y = y_;
    z = z_;
    center = glm::ivec3(x, y, z);
    state = 0;
    live_neighbors = 0;
    type = type_;
    texBase = typeBase[type];

}