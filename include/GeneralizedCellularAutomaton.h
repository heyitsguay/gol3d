//
// Created by matt on 12/13/20.
//
#include <set>
#include <string>

#include "Object.h"

#ifndef GOL3D_GENERALIZEDCELLULARAUTOMATON_H
#define GOL3D_GENERALIZEDCELLULARAUTOMATON_H
#pragma once

class GeneralizedCellularAutomaton : public Object {
/* Generalized cellular automaton (GCA) that is a generalization of the
 * cellular automata types implemented in CellularAutomaton.
 *
 *
 * The original CA implements two classes of rules:
 *
 *   (1) "Game of Life", which has two states {0,1}, defined on a grid of voxels
 *       each connected to their 26 neighbors in 3D. Transitions are defined
 *       voxelwise by a "birth" set B<={0,1,...,26}:=A containing the 1-neighbor
 *       counts which cause a 0->1 transition, and a "stay" set S<=N
 *       containing the 1-neighbor counts cause a 1->1 transition. 0->0 is B's
 *       complement, and 1->0 is S's complement. Transitions can be defined
 *       as as a transition "matrix" of sets:
 *            0  1
 *          0 B' B
 *          1 S' S
 *
 *   (2) "Brian's Brain", a GoL variant that adds a third state {0,1,2}, but
 *       is still defined by B and S subsets of A. Transitions are
 *       defined as
 *            0  1  2
 *          0 B' B  -
 *          1 -  S  S'
 *          2 A  -  -
 *
 *   This pattern can be generalized to k states {0,1,..,k-1} with transition
 *   matrices defined such that each row is a partition of A. That is what
 *   this GeneralizedCellularAutomaton (GCA) implements.
 */
private:
    // Indicates whether the CellularAutomaton is currently 'stepping' - updating one time,
    // while not in the run state.
    bool stepping = false;

    // Indicates which update cycle position to update to, while stepping.
    int stepStart;

    // Rule matrix in its internal representation.
    std::vector<std::vector<int>> ruleMatrixInt;

    static std::vector<int> parseRuleRow(
            const std::vector<std::string> &rowExt);

    // First part of the update Cycle.
    void updateActiveCubes();

    // Second part of the update cycle.
    void updateNeighborCount();

    // Third part of the update cycle.
    void updateState();

    // Fourth part of the update cycle.
    void updateResetCount();

public:
    // Rule matrix in its external representation.
    std::vector<std::vector<std::string>> ruleMatrixExt;
    // String representation of the rule matrix
    std::string ruleString;

    // List of states that count as "alive" when computing live neighbor counts.
    std::set<int> liveStates;

    // Number of Cube states
    int numStates;

    // Counts the number of active Cubes in each state. `stateCounts[i]` is the count
    // for Cube state `i`.
    std::vector<int> stateCounts;

    GeneralizedCellularAutomaton();
    ~GeneralizedCellularAutomaton() override;

    void cubeCube(int hwidth=10, std::vector<float> ps={0.1}, glm::ivec3 center=glm::ivec3(0,0,0));

    void handleInput() override;

    void setCube(Cube *c, int state);

    void recomputeStateCounts();

    void setRule(
            const std::vector<std::vector<std::string>> &_ruleMatrixExt,
            const std::set<int>& _liveStates);

    void update() override;
};

#endif //GOL3D_GENERALIZEDCELLULARAUTOMATON_H
