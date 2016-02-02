//
// Created by mattguay on 1/24/16.
//

#include <vector>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

Camera::Camera() : io(IO::getInstance()) {}

/**
 * Camera.handleInput()
 * Handles user input.
 */
void Camera::handleInput() {
    // Nothing, for the moment.
}

/**
 * Camera.init()
 * Initializes the Camera object.
 * @param window_: Pointer to the GLFW window.
 * @param world_: Pointer to the GOL TempName.
 * @param ratio: Window aspect ratio.
 */
void Camera::init() {
    fov = 45.f;
    aspectRatio = 1.77777778f;
    drawDistance = 2000.f;

    // Create the Projection matrix.
    Projection = glm::perspective(fov, aspectRatio, 0.1f, drawDistance);
}

/**
 * Camera.update()
 * Updates the Camera object.
 */
void Camera::update() {

    handleInput();

//    // Update the View matrix.
    View = glm::lookAt(position, position + heading, up);

    // Update the VP matrix.
    VP = Projection * View;
}
// End Camera.cpp