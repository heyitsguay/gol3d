//
// Created by mattguay on 1/23/16.
//

#ifndef PLAYGROUND_WORLD_H
#define PLAYGROUND_WORLD_H
#pragma once

#include <queue>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <vector>

#include <glm/glm.hpp>

#include "Cube.h"
#include "IO.h"

// Possible World update states.
enum State {stop, edit, run};

//typedef std::unordered_map<coord_t, Cube*> hashmap3d_t;

class World {
private:
    // Tracks the current frame relative to the the frames_per_draw-length
    // update cycle.
    int fcount;

    // Reference to the IO handler.
    IO &io;

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
    std::unordered_map<long int, Cube*> activeCubes;

    // Hashmap containing the Cubes to be drawn this frame.
    std::unordered_map<long int, Cube*> drawCubes;

    // Unordered set containing (x, y, z) centers of Cubes to add to
    // activeCubes.
    std::unordered_map<long int, std::tuple<int, int, int>> addCubes;

    // Vector containing the indices of Cubes to remove from activeCubes.
    std::vector<long int> removeCubes;

    // Maximum number of Cubes.
    int maxCubes;

    // Number of active Cubes.
    int nCubes;

    // Spatial scale of the Cubes.
    float scale;

    World();
    ~World();

    void add(int x, int y, int z);

    void cubeCube(int hwidth = 10, float p = 0.1, glm::ivec3 center = glm::ivec3(0,0,0));

    bool findActiveCubes(long int idx);

    bool findAddCubes(long int idx);

    void flip(Cube *c);

    void freeMemory();

    void handleInput();

    void init(
            std::vector<int> stay_vals,
            std::vector<int> born_vals,
            float scale_=1.f,
            int frames_per_draw_=4,
            int maxCubes_=4000000,
            int bound_=10000
    );

    long int key(int x, int y, int z);

    void remove(long int idx);

    void reset();

    void update();
};


#endif //PLAYGROUND_WORLD_H
