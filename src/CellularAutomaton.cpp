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
void CellularAutomaton::cubeCube(
        int hwidth,
        float p,
        glm::ivec3 center) {
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
    // Increment state (mod numStates).
    c->state = (c->state + 1) % numStates;


    // Update the Cube's status in drawCubes.
    if(c->state == 0) {
        // Cube is dead, don't draw it.
        drawCubes.erase(c->center);

    } else if(c->state == 1) {
        // Cube is newly live, add it to drawCubes.
        drawCubes.insert({c->center, c});
    }

    glm::ivec3 newCenter;

    // Add neighbors to addCubes if they're not they're already.
    for (int dx = -1; dx <= 1; ++dx) {
        newCenter.x = c->x + dx;
        for (int dy = -1; dy <= 1; ++dy) {
            newCenter.y = c->y + dy;
            for (int dz = -1; dz <= 1; ++dz) {
                newCenter.z = c->z + dz;

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

        } else if(io.toggled(GLFW_KEY_3)) {
            state = run;

        } else if(io.toggled(GLFW_KEY_E)) {
            // Step forward one step, if stopped.
            if(state == stop) {
                state = step;
                // Record which stage of the update cycle the step
                // starts on.
                stepStart = cycleStage;
            }

        } else if(io.toggled(GLFW_KEY_R)) {
            // Reset.
            reset();
        }
    }
}

/**
 * CellularAutomaton.setCube()
 * Sets Cube *c's state to (state).
 * @param c: Pointer to the Cube whose state is being set.
 * @param state: State to set *c to.
 */
void CellularAutomaton::setCube(Cube *c, int state) {

    // Use this to track any changes in the Cube's state.
    int prevState = c->state;

    // Only update the Cube and its neighbors if the state changed.
    if(state != prevState) {
        // Set state.
        c->state = state;

        // Update the Cube's status in drawCubes.
        if (c->state == 0) {
            // Cube is dead, don't draw it.
            drawCubes.erase(c->center);

        } else if (prevState == 0) {
            // Cube is newly live or dying, add it to drawCubes.
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
}

/**
 * CellularAutomaton.setRule()
 * Sets the update rule for the Cube states.
 * @param stay_vals: A live Cube stays live if its live neighbor count is an element of this vector.
 * @param born_vals: A dead Cube becomes live if its live neighbor count is an element of this vector.
 * @param bbMode: If true, run the CellularAutomaton in Brian's brain mode (live Cubes have a 'dying'
 *                state for one update cycle before becoming dead).
 */
void CellularAutomaton::setRule(std::vector<int> born_vals, std::vector<int> stay_vals, bool bbMode) {
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
    if(bbMode) {
        numStates = 3;
    } else {
        numStates = 2;
    }
}

/**
 * CellularAutomaton.update()
 * Updates the Cubes in activeCubes.
 */
void CellularAutomaton::update() {
    // Handle user input.
    handleInput();

    // If this is the active Object, and it's in the 'run' state or
    // undergoing a single update step, update.
    if(active && (state != stop)) {
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

        if(state == step && initCycleStage != cycleStage && cycleStage == stepStart) {
            // We've made one loop through the update cycle since beginning a step.
            state = stop;
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

//    if(stepStart == cycleStage) {
//
//    }
    cycleStage++;
}

/**
 * CellularAutomaton.updateNeighborCount()
 * Counts the number of live Cubes neighboring each Cube in activeCubes.
 */
void CellularAutomaton::updateNeighborCount() {
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
                            // its liveNeighbors if it does.
                            auto key = glm::ivec3(X, Y, Z);
                            if(findIn(activeCubes, key)) {
                                activeCubes[key]->liveNeighbors++;
                            }
                        }

                    }
                }
            }
        }
    }
    cycleStage++;
}

/**
 * CellularAutomaton.updateResetCount()
 * Resets the liveNeighbors property to 0 for all Cubes in activeCubes.
 */
void CellularAutomaton::updateResetCount() {
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        c->liveNeighbors = 0;
    }

    cycleStage++;
}

/**
 * CellularAutomaton.updateState()
 * Updates the state of each Cube in activeCubes.
 */
void CellularAutomaton::updateState() {
    for(auto & activeCube : activeCubes) {
        Cube *c = activeCube.second;

        if(c->state == 0) {
            // Check if a dead Cube should become live.
            if(born[c->liveNeighbors]) {
                flip(c);

            } else {
                // Dead Cube stayed dead. Remove if from activeCubes.
                removeCubes.push_back(c->center);
            }
        }

        else if(c->state == 1) {
            // Check if a live Cube should stay.

            // Toggle state if liveNeighbors isn't a valid stay[] value.
            if(!stay[c->liveNeighbors]) {
                flip(c);

            }

        } else if(c->state == 2) {
            // A dying Cube should become dead (only occurs in bbMode).
            flip(c);
        }

    }

    cycleStage++;
}