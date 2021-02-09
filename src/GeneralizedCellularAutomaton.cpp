//
// Created by matt on 12/13/20.
//
#include <chrono>
#include <fstream>
#include <iostream>
#include <random>
#include <sstream>

#include "utils.h"
#include "GeneralizedCellularAutomaton.h"

GeneralizedCellularAutomaton::GeneralizedCellularAutomaton() : Object() {
    numStates = -1;
    stepStart = 0;
}

GeneralizedCellularAutomaton::~GeneralizedCellularAutomaton() {
    freeMemory();
}

/**
 * GeneralizedCellularAutomaton.cubeCube()
 * Within a 3D region with logical coordinates (center) + [-hwidth, hwidth]^3,
 * adds a live Cube to the CellularAutomaton at each (x, y, z) location with
 * probability p.
 * @param hwidth: Setup volume half-width.
 * @param ps: Live Cube state activation probabilities.
 * @param center: Center of the cube of Cubes.
 */
void GeneralizedCellularAutomaton::cubeCube(
        int hwidth,
        std::vector<float> ps,
        glm::ivec3 center) {
    // Obtain a seed from the system clock:
    auto seed = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());

    std::mt19937 gen(seed);
    std::uniform_real_distribution<float> u(0.f, 1.f);

    int cx = center.x;
    int cy = center.y;
    int cz = center.z;

    std::vector<float> ps_cdf;
    for (int i = 0; i < ps.size(); ++i) {
        ps_cdf.push_back(0.f);
        for (int j = 0; j <= i; ++j) {
            ps_cdf.at(i) += ps[j];
        }
    }

    for(int x = cx-hwidth; x <= cx+hwidth; ++x) {
        for(int y = cy-hwidth; y <= cy+hwidth; ++y) {
            for(int z = cz-hwidth; z <= cz+hwidth; ++z) {
                float v = u(gen);
                // Add an active Cube at (x,y,z)
                for (int i = 0; i < ps_cdf.size(); i++) {
                    if (v < ps_cdf.at(i)) {
                        add(x, y, z);
                        setCube(activeCubes[glm::ivec3(x, y, z)], i + 1);
                        break;
                    }
                }

            }
        }
    }
}

/**
 * GeneralizedCellularAutomaton.handleInput()
 * Handles events triggered by user input.
 */
void GeneralizedCellularAutomaton::handleInput() {
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
 * GeneralizedCellularAutomaton.parseRuleRow()
 * Convert a rule row from its human-friendly string-based "external"
 * representation into a computation friendly int-based "internal"
 * representation.
 * @param rowExt
 * @return row_int
 */
std::vector<int> GeneralizedCellularAutomaton::parseRuleRow(
        const std::vector<std::string> &rowExt) {
    // If a complement token is found for transition state j, all neighbor
    // count values not assigned another transition state are assigned j after
    // the rest of the assignments are complete.
    int complement_state = -1;

    // The rule row in its internal representation is a vector of 27 ints. The
    // value -1 represents a position in the row that has not been assigned
    // a transition state. This function should probably validate the rule row
    // after assignment is finished to verify that none of the values in the
    // returned `row_int` are -1, but it doesn't.
    std::vector<int> row_int;
    row_int.assign(27, -1);

    // Iterate through the elements of the external representation of the rule
    // row. Each element is a string containing either "A", "C", "-", or a
    // comma-separated list of integers in [0, 26].
    for (int i = 0; i < rowExt.size(); ++i) {
        // The int `i` indicates the transition state associated with `rowExt`
        // element `el`.
        const std::string &el = rowExt.at(i);

        // If `el` is a single character, it's either one of the special
        // characters or a single-digit number. Handle this case separately.
        if (el.length() == 1) {
            // Get the single character `c`.
            const char &c = el.at(0);
            if (c == 'A') {
                // `c` is 'A', indicating that `row_int` should transition to
                // state `i` for every neighbor count.
                for (int &r : row_int) {
                    r = i;
                }
            } else if (c == 'C') {
                // `c` is 'C', indicating that the complement of the other
                // assigned neighbor counts should transition to state `i`. This
                // is handled after the explicit transition state assignment, so
                // for now just indicate that `i` is the complement state to
                // assign at the end.
                complement_state = i;
            }
            else if (c != '-') {
                // If `c` is '-', then no neighbor count transitions to `i`, so
                // any other single character must be a single-digit number
                // (or a mistake, which isn't checked for).
                // Little trick to convert a char representing a digit into the
                // digit itself.
                int j = c - '0';
                // A live neighbor count of `j` should transition to state `i`.
                row_int.at(j) = i;
            }
        } else {
            // String is more than one character. Assume it's a comma-separated
            // string of ints - split along the ",".
            std::vector<std::string> nums = split(el, ",");
            for (const std::string &num : nums) {
                // Convert each int string to an int, and assign that position
                // in `row_int` to a transition to `i`.
                int j = std::stoi(num);
                row_int.at(j) = i;
            }
        }
    }

    if (complement_state > -1) {
        // If `complement_state` > -1, then a complement state has been
        // assigned. Find any unassigned elements in `row_int` (i.e. elements
        // with value -1) and assign them the state `complement_state`.
        for (int &r : row_int) {
            if (r == -1) {
                r = complement_state;
            }
        }
    }
    return row_int;
}


/**
 * GeneralizedCellularAutomaton.setCube()
 * Sets Cube *c's state to (state).
 * @param c: Pointer to the Cube whose state is being set.
 * @param state: State to set *c to.
 */
void GeneralizedCellularAutomaton::setCube(Cube *c, int state) {

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
 * GeneralizedCellularAutomaton.setRule()
 * Sets the update rule for the Cube states. Given k states {0,...,k-1}, the
 * rule matrix M is a kxk array such that M_ij specifies a subset of
 * A:={0,...,26}, indicating counts of a voxel's 26 neighbors which are in the
 * `liveStates` list, for which state i transitions to state j. Each row of M
 * must partition A.
 *
 * Each entry is specified as either a string of comma-separated integers in A
 * or a single special character: "A" for all of A, "-" for the empty set, or
 * "C" for the complement (in A) of the rest of the row. So for instance, the
 * classic Conway's Game of Life rule B3/S23 would be expressed as
 * [["C", "2"], ["C", "2,3"]], and the Brian's brain variant is
 * [["C", "2", "-"], ["-", "2,3", "C"], ["A", "-", "-"]].
 * @param _ruleMatrixExt
 * @param _liveStates
 */
void GeneralizedCellularAutomaton::setRule(
        const std::vector<std::vector<std::string>> &_ruleMatrixExt,
        const std::set<int> &_liveStates) {
    // Internally the rule set is represented as a kx27 array B of ints, such
    // that B_ij is the state that a voxel in state i with j live neighbors will
    // transition to
    ruleMatrixExt = _ruleMatrixExt;
    liveStates = _liveStates;
    for (auto &row : ruleMatrixExt) {
        ruleMatrixInt.push_back(parseRuleRow(row));
    }
    numStates = ruleMatrixExt.size();

    std::stringstream ruleStringStream;
    ruleStringStream << "{";
    for (int i = 0; i < ruleMatrixExt.size(); ++i) {
        ruleStringStream << "{";
        const std::vector<std::string> &ruleRow = ruleMatrixExt.at(i);
        for (int j = 0; j < ruleRow.size(); ++j) {
            const std::string &s = ruleRow.at(j);
            ruleStringStream << s;
            if (j < ruleRow.size() - 1) {
                ruleStringStream << "/";
            } else {
                ruleStringStream << "}";
            }
        }
        if (i < ruleMatrixExt.size() - 1) {
            ruleStringStream << ", ";
        } else {
            ruleStringStream << "}";
        }
    }
    ruleString = ruleStringStream.str();


    std::ofstream debugFile;
    debugFile.open("stateDebug.txt");
    for (int i = 0; i < 27; i++) {
        debugFile << i << " ";
        if (i < 10) debugFile << " ";
    }
    debugFile << "\n\n";
    for (auto & row : ruleMatrixInt) {
        for (auto & col : row) {
            debugFile << col << "  ";
        }
        debugFile << "\n";
    }
    debugFile.close();
}


/**
 * GeneralizedCellularAutomaton.update()
 * Updates the Cubes in activeCubes.
 */
void GeneralizedCellularAutomaton::update() {
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
 * GeneralizedCellularAutomaton.updateActiveCubes()
 * Processes removeCubes and addCubes to update activeCubes.
 */
void GeneralizedCellularAutomaton::updateActiveCubes() {
    // First remove inactive Cubes from activeCubes.
    for(auto &center : removeCubes) {
        remove(center);
    }

    // Second, add newly-active Cubes to activeCubes.
    for(auto & addCube : addCubes) {
        glm::ivec3 center = addCube.first;
        add(center.x, center.y, center.z);
    }

    // Clear addCubes and removeCubes.
    addCubes.clear();
    removeCubes.clear();

    cycleStage++;
}

/**
 * GeneralizedCellularAutomaton.updateNeighborCount()
 * Counts the number of live Cubes neighboring each Cube in activeCubes.
 */
void GeneralizedCellularAutomaton::updateNeighborCount() {
    for(auto & activeCube : activeCubes) {
        Cube *c = activeCube.second;

        // Only update if c is live.
        if(in(liveStates, c->state)) {
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
 * GeneralizedCellularAutomaton.updateResetCount()
 * Resets the liveNeighbors property to 0 for all Cubes in activeCubes.
 */
void GeneralizedCellularAutomaton::updateResetCount() {
    for(auto & activeCube : activeCubes) {
        Cube *c = activeCube.second;

        c->liveNeighbors = 0;
    }

    cycleStage++;
}


/**
 * GeneralizedCellularAutomaton.updateState()
 * Updates the state of each Cube in activeCubes.
 */
void GeneralizedCellularAutomaton::updateState() {
    for (auto &activeCube : activeCubes) {
        Cube *c = activeCube.second;
        int oldState = c->state;
        int newState = ruleMatrixInt.at(oldState).at(c->liveNeighbors);
        if (newState != oldState) {
            setCube(c, newState);
        } else if (oldState == 0) {
            removeCubes.push_back(c->center);
        }
    }
    cycleStage++;
}
