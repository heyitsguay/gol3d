//
// Created by mattguay on 1/24/16.
//

#ifndef GOL3D_CAMERA_H
#define GOL3D_CAMERA_H
#pragma once

#include <glm/glm.hpp>

#include "IO.h"

class Camera {
private:
    Camera();
    Camera(Camera const&);
    void operator=(Camera const&);

    // The IO handler.
    IO &io;

public:
    static Camera &getInstance() {
        static Camera instance;
        return instance;
    }
    // Camera View matrix.
    glm::mat4 View;

    // Camera Projection matrix.
    glm::mat4 Projection;

    // View-Projection matrix.
    glm::mat4 VP;

    // Camera position.
    glm::vec3 position;

    // Camera heading.
    glm::vec3 heading;

    // Upward-facing vector orthogonal to heading.
    glm::vec3 up;

    // Field of view.
    float fov;

    // Aspect ratio.
    float aspectRatio;

    // Draw distance.
    float drawDistance;

    // Handle key input
    void handleInput();

    // Initialize the Camera.
    void init();

    // Update the Camera.
    void update();
};

#endif //GOL3D_CAMERA_H
