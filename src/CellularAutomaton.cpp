//
// Created by matt on 1/31/16.
//
#include "CellularAutomaton.h"

#include <chrono>
#include <random>

CellularAutomaton::CellularAutomaton() : Object() {}

CellularAutomaton::~CellularAutomaton() {
    freeMemory();
}

/**
 * CellularAutomaton.cubeCube()
 * Within a 3D region with logical coordinates (center) + [-hwidth, hwidth]^3,
 * adds a live Cube to the CellularAutomaton at each (x, y, z) location with
 * probability p.
 * @param hwidth: Setup volume half-width.
 * @param p: Cube activation probability.
 * @param center: Center of the cube of Cubes.
 */
void CellularAutomaton::cubeCube(int hwidth, float p, glm::ivec3 center) {
    // Obtain a seed from the system clock:
    auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> u(0.f, 1.f);

    int cx = center.x;
    int cy = center.y;
    int cz = center.z;

    for(int x = cx-hwidth; x <= cx+hwidth; ++x) {
        for(int y = cy-hwidth; y <= cy+hwidth; ++y) {
            for(int z = cz-hwidth; z <= cz+hwidth; ++z) {
                float v = u(gen);
                // Add an active Cube at (x,y,z) with probability p.
                if(v < p) {
                    add(x, y, z);
                    flip(activeCubes[glm::ivec3(x, y, z)]);
                }
            }
        }
    }
}

/**
 * CellularAutomaton.flip()
 * Toggles the state of Cube *c, then adds its neighbors to activeCubes.
 * @param c: Pointer to the Cube whose state is being toggled.
 */
void CellularAutomaton::flip(Cube *c) {
    // If in Brian's brain mode, increment the state by 1 (mod 3). Otherwise, toggle between
    // 0 and 2.
    if(bbMode) {
        c->state = char((c->state + 1) % 3);

    } else {
        c->state = (char)1 - c->state;
    }

    // Update the Cube's status in drawCubes.
    if(c->state == 0) {
        // Cube is dead, don't draw it.
        drawCubes.erase(c->center);

    } else if(c->state == 1) {
        // Cube is newly live, add it to drawCubes.
        drawCubes.insert({c->center, c});
    }

    // Add neighbors to addCubes if they're not they're already.
    for (int dx = -1; dx <= 1; ++dx) {
        int X = c->x + dx;
        for (int dy = -1; dy <= 1; ++dy) {
            int Y = c->y + dy;
            for (int dz = -1; dz <= 1; ++dz) {
                int Z = c->z + dz;

                auto newCenter = glm::ivec3(X, Y, Z);
                if (!findIn(addCubes, newCenter)) {
                    addCubes.insert({newCenter, true});
                }
            }
        }
    }
}

/**
 * CellularAutomaton.handleInput()
 * Handles events triggered by user input.
 */
void CellularAutomaton::handleInput() {
    // Only handle IO if this Object is active.
    if(active) {
        // CellularAutomaton state changes use keys 1, 2, 3.
        if(io.toggled(GLFW_KEY_1)) {
            state = stop;

        } else if(io.toggled(GLFW_KEY_2)) {
            state = edit;

            // Push to the end of the update cycle to make sure newly-inserted Cubes
            // are updated properly.
            cycleStage = 4;

        } else if(io.toggled(GLFW_KEY_3)) {
            state = run;

        } else if(io.toggled(GLFW_KEY_E)) {
            // Step forward one step.
            if(!stepping && state != run) {
                stepping = true;
                stepStart = cycleStage;
            }

        } else if(io.toggled(GLFW_KEY_R)) {
            // Reset.
            reset();
        }
    }
}

/**
 * CellularAutomaton.setRule()
 * Sets the update rule for the Cube states.
 * @param stay_vals: A live Cube stays live if its live neighbor count is an element of this vector.
 * @param born_vals: A dead Cube becomes live if its live neighbor count is an element of this vector.
 * @param bbMode: If true, run the CellularAutomaton in Brian's brain mode (live Cubes have a 'dying'
 *                state for one update cycle before becoming dead).
 */
void CellularAutomaton::setRule(std::vector<int> born_vals, std::vector<int> stay_vals, bool bbMode_) {
    // Reset the rule arrays.
    for(int i = 0; i < 27; ++i) {
        born[i] = false;
        stay[i] = false;
    }

    // Rule setup.
    for(auto& b : born_vals) {
        if(b >= 0 && b < 27) {
            born[b] = true;
        }
    }
    for(auto& s : stay_vals) {
        if(s >= 0 && s < 27) {
            stay[s] = true;
        }
    }

    // Specify whether or not to use Brian's brain mode.
    bbMode = bbMode_;
}

/**
 * CellularAutomaton.update()
 * Updates the Cubes in activeCubes.
 */
void CellularAutomaton::update() {
    // Handle user input.
    handleInput();

    if(active && (state == run || stepping)) {
        // Track the cycleStage at the beginning of each frame's update, to see when it changes.
        int initCycleStage = cycleStage;

        if(cycleStage == 0) {
            updateActiveCubes();

        } else if(cycleStage == 1) {
            updateNeighborCount();

        } else if(cycleStage == 2) {
            updateState();

        } else if(cycleStage == 3) {
            updateResetCount();

        } else if(cycleStage == 4) {
            // End of the update cycle.
            cycleStage = 0;

        } else {
            printf("Something has gone wrong with a CellularAutomaton update.\n");
            abort();
        }

        if(initCycleStage != cycleStage && cycleStage == stepStart) {
            // We've made one loop through the update cycle since beginning a step.
            stepping = false;
        }
    }
}

/**
 * CellularAutomaton.updateActiveCubes()
 * Processes removeCubes and addCubes to update activeCubes.
 */
void CellularAutomaton::updateActiveCubes() {
    // First remove inactive Cubes from activeCubes.
    for(auto &center : removeCubes) {
        remove(center);
    }

    // Second, add newly-active Cubes to activeCubes.
    for(auto it = addCubes.begin(); it != addCubes.end(); ++it) {
        glm::ivec3 center = it->first;
        add(center.x, center.y, center.z);
    }

    // Clear addCubes and removeCubes.
    addCubes.clear();
    removeCubes.clear();

    if(stepStart == cycleStage) {

    }
    cycleStage++;
}

/**
 * CellularAutomaton.updateNeighborCount()
 * Counts the number of live Cubes neighboring each Cube in activeCubes.
 */
void CellularAutomaton::updateNeighborCount() {
    // Iterate through at most this number of live Cubes in activeCubes per frame.
//    static int updateCount = 500000;
    // When true, reset iter at the start of the update.
//    static bool resetIter = true;
    // Iterator through activeCubes.
//    static auto iter = activeCubes.begin();

//    if(resetIter) {
//        iter = activeCubes.begin();
//    }

//    while(updateCount > 0 && iter != activeCubes.end()) {
    for(auto iter = activeCubes.begin(); iter != activeCubes.end(); ++iter) {
        Cube *c = iter->second;

        // Only update if c is live.
        if(c->state == 1) {
            // Increment its neighbors in activeCubes.
            for (int dx = -1; dx <= 1; ++dx) {
                int X = c->x + dx;
                for (int dy = -1; dy <= 1; ++dy) {
                    int Y = c->y + dy;
                    for (int dz = -1; dz <= 1; ++dz) {
                        // Don't update yourself.
                        if (!(dx == 0 && dy == 0 && dz == 0)) {
                            int Z = c->z + dz;

                            // Check if the neighbor exists in activeCubes. Increment
                            // its live_neighbors if it does.
                            auto key = glm::ivec3(X, Y, Z);
                            if(findIn(activeCubes, key)) {
                                activeCubes[key]->live_neighbors++;
                            }
                        }

                    }
                }
            }

//            updateCount--;
        }
//        ++iter;
    }

    // Escaped the while loop. Either we ran out of update iterations for this frame, or we
    // reached the end of activeCubes.
//    if(iter == activeCubes.end()) {
        // Reached the end of activeCubes. Reset static variables, advance to the
        // next stage of the update cycle.
//        resetIter = true;
//        cycleStage++;
//    }
    cycleStage++;

    // Reset updateCount for the next round of neighbor updates.
//    updateCount = 500000;
}

/**
 * CellularAutomaton.updateResetCount()
 * Resets the live_neighbors property to 0 for all Cubes in activeCubes.
 */
void CellularAutomaton::updateResetCount() {
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        c->live_neighbors = 0;
    }

    cycleStage++;
}

/**
 * CellularAutomaton.updateState()
 * Updates the state of each Cube in activeCubes.
 */
void CellularAutomaton::updateState() {
    // Iterate through at most this number of live Cubes in activeCubes per frame.
//    static int updateCount = 500000;
    // When true, reset iter to activeCubes.begin();
//    static bool resetIter = true;
    // Iterator through activeCubes.
//    static auto iter = activeCubes.begin();

//    if(resetIter) {
//        iter = activeCubes.begin();
//    }

//    while(updateCount > 0 && iter != activeCubes.end()) {
    for(auto iter = activeCubes.begin(); iter != activeCubes.end(); ++iter) {
        Cube *c = iter->second;

        if(c->state == 0) {
            // Check if a dead Cube should become live.
            if(born[c->live_neighbors]) {
                flip(c);

            } else {
                // Dead Cube stayed dead. Remove if from activeCubes.
                removeCubes.push_back(c->center);
            }
        }

        else if(c->state == 1) {
            // Check if a live Cube should stay.

            // Toggle state if live_neighbors isn't a valid stay[] value.
            if(!stay[c->live_neighbors]) {
                flip(c);

            }

        } else if(c->state == 2) {
            // A dying Cube should become dead (only occurs in bbMode).
            flip(c);
        }

//        updateCount--;
//        resetIter = true;
    }

    // Escaped the while loop. Either we ran out of update iterations for this frame, or we
    // reached the end of activeCubes.
//    if(iter == activeCubes.end()) {
        // Reached the end of activeCubes. Reset static variables, advance to the
        // next stage of the update cycle.
//        iter = activeCubes.begin();
//        cycleStage++;
//    }

    cycleStage++;
    // Reset updateCount for the next round of neighbor updates.
//    updateCount = 500000;
}