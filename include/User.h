//
// Created by matt on 1/28/16.
//

#ifndef GOL3D_USER_H
#define GOL3D_USER_H
#pragma once

#include <GL/glew.h>
#include <glm/glm.hpp>

#include "Camera.h"
#include "CellularAutomaton.h"
#include "World.h"

#include "global.h"

enum UserState {edit, move, selection};

class User {
private:
    // IO reference.
    IO &io;
    // Camera reference.
    Camera &cam;

    // World reference.
    World *world;

    // Reference to the World's active Object.
    Object **activeObj;

    // Max linear speed.
    const float max_speed = 80.f;

    // Rotational speed.
    const float rotation_speed = 0.03;

    // Number of frames to take when resetting the User's position.
    const int num_reset_frames = 60;

    // Number of frames left in the reset process.
    int reset_frames_left = 0;

    // Reset translation increment.
    glm::vec3 reset_dposition;

    // Reset horizontal angle increment.
    float reset_dhorizontal;

    // Reset vertical angle increment.
    float reset_dvertical;

    // Time at the last update.
    double tPrev;

    // Time between the current and previous updates.
    float dTime;

    // Reset position.
    glm::vec3 position0;

    // Reset heading angles.
    float horizontalAngle0;
    float verticalAngle0;

    // Variables to control Cube drawing.
    // Initializes drawing.
    bool drawStart;
    // If true, draw live Cubes at the cursor.
    bool drawLive;
    // If true, draw dead Cubes at the cursor.
    bool drawDead;
    // If true, draw dying Cubes at the cursor.
    bool drawDying;
    // Half width of drawn Cube cubes.
    int cubeHwidth;
    // Draw cursor offset from heading basepoint.
    glm::vec3 cursorOffset;
    // Determines how far along the heading vector the draw cursor basepoint is.
    const float baseDrawDist = 10.f;
    // Max distance the cursor can get from the User.
    const float cursorBound = 50.f;
    // Speed at which the cursor position changes.
    const float cursorSpeed = 0.2;

    // Used for computing the bounds of the currentRegion;
    int x0, x1, y0, y1, z0, z1;

    // OpenGL uniform for the cursor MVP matrix.
    GLuint uMVP;

    // OpenGL uniform for the cursor color state.
    GLuint uColorState;

    // Pointer to the cursor shader program.
    GLuint *programCursor;


    void computeRegionBounds();

public:
    // User state.
    UserState state;

    // Spatial position.
    glm::vec3 position;
    
    // Heading.
    glm::vec3 heading;
    
    // Heading horizontal angle.
    float horizontalAngle;
    
    // Heading vertical angle.
    float verticalAngle;
    
    // Keep track of an orthonormal coordinate system based on the User's heading, with additional basis vectors
    // pointing right and up.
    // Right-pointing basis vector.
    glm::vec3 right;
    // Up-pointing basis vector.
    glm::vec3 up;
    
    // Current speed.
    float speed;
    
    // Controls acceleration, but isn't an acceleration value in the physics sense.
    float accel;
    
    // Location of the draw cursor.
    glm::ivec3 drawCursor;

    // Contains the bottom-left front and top-right back coordinates
    // of the selection region. Only changes when the User manually
    // selects new points.
    glm::ivec3 selectedRegion[2];
    // A dynamic selection region that moves with the User.
    glm::ivec3 currentRegion[2];
    // Contains the top-right backcorner coordinate, relative to a bottom-left
    // front corner of (0,0,0).
    glm::ivec3 dRegion;
    // Center of the select region.
    glm::vec3 regionCenter;
    // Scaling values to transform a unit cube into the region.
    glm::vec3 regionScale;
    // Indicates how many of the 2 region selection indices have been set.
    // Can be 0, 1, or 2.
    int numSetSelections;
    // Clipboard for a region of Cubes to cut/copy/paste. Keys use
    // coordinates relative to a bottom-left front corner of (0,0,0).
    // A value of 'false' indicates a dying Cube, 'true' indicates a
    // live Cube.
    boolMap_t clipBoard;
    
    User();

    void addSelectionPoint(glm::ivec3 &point);

    void computeHeadingBasis();

    void copy();

    void cut();

    void deleteRegion();

    void draw();

    void handleInput();
    
    void init(World *world_,
              GLuint *programCursor_,
              glm::vec3 position_,
              float horizontalAngle_,
              float verticalAngle_);

    void makeCubes();

    void paste();

    void update(double t);

    void updateEdit();

    void updateSelect();
};

#endif //GOL3D_USER_H
