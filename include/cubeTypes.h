//
// Created by matt on 1/29/16.
//

// Defines aliases for the Cube types, and coordinate offsets in the texture atlas for each type.

#ifndef GOL3D_CUBETYPES_H
#define GOL3D_CUBETYPES_H
#pragma once

#include <glm/glm.hpp>

#define T_GRASS 0
#define T_CONCRETE 1
#define T_DIRT 2
#define T_WOOD 3
#define T_BASRELIEF 4
#define T_STONECUBE 5
#define T_BRICK 6
#define T_COBBLESTONE 7
#define T_ROCK 8
#define T_SAND 9
#define T_GRAVEL 10
#define T_TRUNK 11
#define T_TRUNKTOP 12
#define T_SILVER 13
#define T_GOLD 14
#define T_GOLDORE 15
#define T_IRONORE 16
#define T_COALORE 17
#define T_OBSIDIAN 18
#define T_DIAMOND 19
#define T_REDSTONE 20
#define T_STONEBRICK 21
#define T_COTTON 22
#define T_SWAMP 23
#define T_SNOW 24
#define T_BORDERED 25
#define T_PUMPKIN 26
#define T_CAKE 27
#define T_BLANK 28

// Define type base coordinates.
const static glm::ivec2 typeBase[29] = {
        glm::ivec2(0, 0),
        glm::ivec2(1, 0),
        glm::ivec2(2, 0),
        glm::ivec2(4, 0),
        glm::ivec2(5, 0),
        glm::ivec2(6, 0),
        glm::ivec2(7, 0),
        glm::ivec2(0, 1),
        glm::ivec2(1, 1),
        glm::ivec2(2, 1),
        glm::ivec2(3, 1),
        glm::ivec2(4, 1),
        glm::ivec2(5, 1),
        glm::ivec2(6, 1),
        glm::ivec2(7, 1),
        glm::ivec2(0, 2),
        glm::ivec2(1, 2),
        glm::ivec2(2, 2),
        glm::ivec2(5, 2),
        glm::ivec2(2, 3),
        glm::ivec2(3, 3),
        glm::ivec2(6, 3),
        glm::ivec2(0, 4),
        glm::ivec2(1, 4),
        glm::ivec2(2, 4),
        glm::ivec2(3, 4),
        glm::ivec2(7, 7),
        glm::ivec2(9, 7),
        glm::ivec2(0, 8)
};


#endif //GOL3D_CUBETYPES_H
