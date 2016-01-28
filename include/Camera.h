//
// Created by mattguay on 1/24/16.
//

#ifndef PLAYGROUND_CAMERA_H
#define PLAYGROUND_CAMERA_H
#pragma once

#include <vector>

#include <GLFW/glfw3.h>

#include <glm/glm.hpp>

#include "IO.h"
#include "World.h"

const double PI = 3.1415926535897;

//#define pressd(k) glfwGetKey(window, k) == GLFW_PRESS
//#define released(k) glfwGetKey(window, k) == GLFW_RELEASE
#define mod(m, n) fmod(fmod(m, n) + n, n)
#define clamp(n, lower, upper) if(n < lower) n = lower; else if(n > upper) n = upper

class Camera {
private:
    // Max camera speed.
    const float max_speed = 80.f;

    // Camera rotation speed
    const float rotation_speed = 0.03;

    // Number of frames to take when resetting the camera.
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
    float lastTime;

    // Time at the current update.
    float currentTime;

    // Time between this update and the previous update.
    float deltaTime;

    // Points to the right of the current Camera heading. Used for strafing.
    glm::vec3 right;

    // Points upward relative to the current Camera heading.
    glm::vec3 up;

    // Initial position.
    glm::vec3 position0;

    // Cube draw offset vector.
    glm::vec3 drawOffset;

    // Toggles the initialization of drawing.
    bool drawToggle;

    // Initializes drawing.
    bool drawStart;

    // When true, make the Cube under the cursor alive.
    bool drawLive;

    // When true, make the Cube under the cursor dead.
    bool drawDead;

    // Initial heading angles.
    float horizontalAngle0;
    float verticalAngle0;

    // The window.
    GLFWwindow *window;

    // The World.
    World *world;

    // The IO handler.
    IO &io;

    // Used for drawing Cube cubes.
    int cubeHwidth;

public:
    // Camera View matrix.
    glm::mat4 View;

    // Camera Projection matrix.
    glm::mat4 Projection;

    // Camera position.
    glm::vec3 position;

    // Camera heading.
    glm::vec3 heading;

    // Camera heading - horizontal angle.
    float horizontalAngle;

    // Camera heading - vertical angle.
    float verticalAngle;

    // Field of view.
    float fov;

    // Camera speed.
    float speed;

    // Location of the Cube draw cursor.
    glm::ivec3 drawCursor;

    // Draw distance.
    float drawDistance;

    Camera();
    ~Camera();

    // Initialize the Camera.
    void init(GLFWwindow *window_, World *world_, float ratio=1.777777778f);

    // Handle key input
    void handleInput();

    // Make a Cube cube.
    void makeCubes();

    // Update the Camera.
    void update();

    // Update the Camera's draw feature.
    void updateDraw();
};

#endif //PLAYGROUND_CAMERA_H
