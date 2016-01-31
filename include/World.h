//
// Created by mattguay on 1/23/16.
//

#ifndef GOL3D_WORLD_H
#define GOL3D_WORLD_H
#pragma once

#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include "Camera.h"
#include "Cube.h"
#include "IO.h"
#include "ivecHash.h"

// Possible World update states.
enum State {stop, edit, run};

typedef std::unordered_map<glm::ivec3, Cube*, KeyFuncs, KeyFuncs> cubeMap_t;
typedef std::unordered_map<glm::ivec3, bool, KeyFuncs, KeyFuncs> boolMap_t;

class World {
private:
    // Tracks the current frame relative to the the frames_per_draw-length
    // update cycle.
    int fcount;

    // Reference to the IO handler.
    IO &io;

    // Reference to the Camera
    Camera &cam;

    // First part of the update cycle.
    void updateActiveCubes();

    // Second part of the update cycle.
    void updateNeighborCount();

    // Third part of the update cycle.
    void updateState();

    // Fourth part of the update cycle.
    void updateResetCount();

    // Tracks the number of updates needed to advance the World one step.
    int stepCounter = 0;

    // Pointer to the main Cube VAO.
    GLuint *cubeVAO;

    // Pointer to the World shader program.
    GLuint *program;

public:
    // World state.
    State state;

    // How many frames between successive World draw calls.
    int frames_per_draw;

    // World rules, stored as a pair of length-27 bool arrays.
    // If stay[i], then a live cell with i live neighbors will stay.
    bool stay[27] = {false};
    // If born[j], then a dead cell with j live neighbors will become live.
    bool born[27] = {false};

    // Queue containing all of the uninitialized Cubes in the World.
    std::queue<Cube*> limbo;

    // Cube centers must stay in the box [-bound, bound]^3.
    int bound;

    // Hashmap containing the Cubes to be updated this frame.
    cubeMap_t activeCubes;

    // Hashmap containing the Cubes to be drawn this frame.
    cubeMap_t drawCubes;

    // Unordered set containing (x, y, z) centers of Cubes to add to
    // activeCubes.
    boolMap_t addCubes;

    // Vector containing the indices of Cubes to remove from activeCubes.
    std::vector<glm::ivec3> removeCubes;

    // Maximum number of Cubes.
    int maxCubes;

    // Number of active Cubes.
    int nCubes;

    // Spatial scale of the Cubes.
    float scale;

    // 2 * the spatial scale of the Cubes.
    float scale2;

    // Arrays containing drawn Cube translation, scaling, and type info.
    std::vector<glm::vec3> translations;
    std::vector<float> scales;
    std::vector<glm::ivec2> types;

    // OpenGL uniform variables.
    GLuint uMVP, ucamera_pos, ut;

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

    void add(int x, int y, int z);

    void cubeCube(int hwidth = 10, float p = 0.1, glm::ivec3 center=glm::ivec3(0,0,0));

    void draw(float t);

    template<typename T>
    bool findIn(std::unordered_map<glm::ivec3, T, KeyFuncs, KeyFuncs> &map, glm::ivec3 key);

    void flip(Cube *c);

    void freeMemory();

    void handleInput();

    void init(
            GLuint *cubeVAO_,
            GLuint *program_,
            float scale_=1.f,
            int frames_per_draw_=10,
            int maxCubes_=4000000,
            int bound_=10000
    );

    void initGL();

    void remove(glm::ivec3 key);

    void reset();

    void setRule(std::vector<int> stay_vals, std::vector<int> born_vals);

    void update();
};


#endif //GOL3D_WORLD_H
