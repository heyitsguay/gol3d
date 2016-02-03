//
// Created by matt on 1/31/16.
//

#include "Object.h"

#ifndef GOL3D_CELLULARAUTOMATON_H
#define GOL3D_CELLULARAUTOMATON_H

class CellularAutomaton : public Object {
private:
    // Indicates whether the CellularAutomaton is currently 'stepping' - updating one time,
    // while not in the run state.
    bool stepping = false;

    // Indicates which update cycle position to update to, while stepping.
    int stepStart;

    // First part of the update Cycle.
    void updateActiveCubes();

    // Second part of the update cycle.
    void updateNeighborCount();

    // Third part of the update cycle.
    void updateState();

    // Fourth part of the update cycle.
    void updateResetCount();

public:
    // CellularAutomaton rules, stored as a pair of arrays. Using Game of Life-style
    // update rules, specifying which values of live neighbor counts will cause a dead
    // cell to be born or a live cell to stay.
    bool stay[27];
    bool born[27];

    // Number of Cube states - indicates whether we're in regular Game of Life mode
    // (2 states) or Brian's brain mode (3 states).
    int numStates;

    CellularAutomaton();
    ~CellularAutomaton();

    void cubeCube(int hwidth=10, float p=0.1, glm::ivec3 center=glm::ivec3(0,0,0));

    void flip(Cube *c);

    virtual void handleInput();

    void setCube(Cube *c, int state);

    void setRule(std::vector<int> born_vals, std::vector<int> stay_vals, bool bbMode);

    virtual void update();
};

#endif //GOL3D_CELLULARAUTOMATON_H
