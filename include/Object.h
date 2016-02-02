//
// Created by matt on 1/30/16.
//

#ifndef GOL3D_OBJECT_H
#define GOL3D_OBJECT_H
#pragma once

#include <unordered_map>
#include <vector>

#include <glm/glm.hpp>

#include "Cube.h"
#include "IO.h"

#include "ivecHash.h"

// Possible Object update states.
enum State {stop, edit, run};

typedef std::unordered_map<glm::ivec3, Cube*, KeyFuncs, KeyFuncs> cubeMap_t;
typedef std::unordered_map<glm::ivec3, bool, KeyFuncs, KeyFuncs> boolMap_t;

class Object {
protected:
    // Tracks which part of the update cycle the Object is in.
    int cycleStage;

    // Reference to the IO handler.
    IO &io;

public:
    // Object state.
    State state;

    // If true, this Object is the one the User is currently manipulating. The Object only
    // responds to IO events when active.
    bool active;

    // Vector containing all of the uninitialized Cubes.
    std::vector<Cube*> limbo;

    // Hashmap containing the Cubes to be updates this frame.
    cubeMap_t activeCubes;

    // Hashmap containing the Cubes to be drawn this frame.
    cubeMap_t drawCubes;

    // Hashmap indicating which Cubes should be added to activeCubes next
    // cycle.
    boolMap_t addCubes;

    // Vector containing the indices of Cubes to remove from activeCubes.
    std::vector<glm::ivec3> removeCubes;

    // Initial number of Cubes created in limbo.
    int initNumCubes;

    // Spatial scale of the Cubes.
    float scale;

    // Double the spatial scale.
    float scale2;

    // Center of the Object, in spatial coordinates.
    glm::vec3 origin;

    Object();
    virtual ~Object();

    void add(int x, int y, int z);

    glm::ivec3 centerFromPoint(glm::vec3 &point);

    bool checkPoint(glm::vec3 &point);

    template<typename T>
    bool findIn(const std::unordered_map<glm::ivec3, T, KeyFuncs, KeyFuncs> &map, const glm::ivec3 &center);

    void freeMemory();

    virtual void handleInput() = 0;

    virtual void init(glm::vec3 origin_, float scale_, int initNumCubes_);

    virtual void remove(glm::ivec3 &center);

    void reset();

    virtual void update() = 0;
};

#endif //GOL3D_OBJECT_H
