//
// Created by mattguay on 1/24/16.
//

#include <cmath>
#include <vector>

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "Camera.h"

Camera::Camera() : io(IO::getInstance()){
    position0 = glm::vec3(0, 0, 80);
    horizontalAngle0 = float(PI);
    verticalAngle0 = 0.f;

    position = position0;
    horizontalAngle = horizontalAngle0;
    verticalAngle = verticalAngle0;

    fov = 55.f;
}
Camera::~Camera() {}

/**
 * Camera.init()
 * Initializes the Camera object.
 * @param window_: Pointer to the GLFW window.
 * @param world_: Pointer to the GOL World.
 * @param ratio: Window aspect ratio.
 */
void Camera::init(GLFWwindow *window_, World *world_, float ratio) {

    drawDistance = 2000.f;

    // Create the Projection matrix.
    Projection = glm::perspective(fov, ratio, 0.1f, drawDistance);

    lastTime = 0.f;
    currentTime = 0.f;
    window = window_;
    world = world_;
    speed = 0.f;

    up = glm::vec3(0, 0, 0);
    right = glm::vec3(0, 0, 0);

    drawOffset = glm::vec3(0, 0, 0);
    drawCursor = glm::ivec3(0, 0, 0);

    drawToggle = true;
    drawStart = false;
    drawLive = false;
    drawDead = false;

    cubeHwidth = 4;
}

/**
 * Camera.handleInput()
 * Handles user input (currently just key presses).
 */
void Camera::handleInput() {
    // Handle rotations.
    // Rotate up
    if(io.pressed(GLFW_KEY_W)) {
        verticalAngle += rotation_speed;
    }
    // Rotate down
    if(io.pressed(GLFW_KEY_S)) {
        verticalAngle -= rotation_speed;
    }
    // Rotate left
    if(io.pressed(GLFW_KEY_A)) {
        horizontalAngle += rotation_speed;
    }
    // Rotate right
    if(io.pressed(GLFW_KEY_D)) {
        horizontalAngle -= rotation_speed;
    }
    // Clamp the vertical angle.
    clamp(verticalAngle, -PI/2, PI/2);
    // Keep the horizontal angle in [0, 2*PI)
    horizontalAngle = float(mod(horizontalAngle, 2*PI));

    // Handle speed update.
    if(
            io.pressed(GLFW_KEY_UP) ||
            io.pressed(GLFW_KEY_DOWN) ||
            io.pressed(GLFW_KEY_LEFT) ||
            io.pressed(GLFW_KEY_RIGHT)
    ) {
        speed += 0.03 * (max_speed - speed);
    } else {
        speed *= 0.8;
    }

    // Handle translations.
    // Move forward (or upward if left Shift is pressd)
    if(io.pressed(GLFW_KEY_UP)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            position += up * deltaTime * speed;
        } else {
            position += heading * deltaTime * speed;
        }
    }
    // Move backward (or downward if left Shift is pressd)
    if(io.pressed(GLFW_KEY_DOWN)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            position -= up * deltaTime * speed;
        } else {
            position -= heading * deltaTime * speed;
        }
    }
    // Strafe right
    if(io.pressed(GLFW_KEY_RIGHT)) {
        position += right * deltaTime * speed;
    }
    // Strafe left
    if(io.pressed(GLFW_KEY_LEFT)) {
        position -= right * deltaTime * speed;
    }

    // Check for a reset.
    if(io.pressed(GLFW_KEY_T)) {
        reset_frames_left = num_reset_frames;
        speed = 0.f;
        reset_dposition = (position - position0) / float(num_reset_frames);
        reset_dhorizontal = (horizontalAngle - horizontalAngle0) / float(num_reset_frames);
        reset_dvertical = (verticalAngle - verticalAngle0) / float(num_reset_frames);
    }

    // Handle drawing updates.
    const float cursorBound = 50.f;
    const float cursorSpeed = 0.2;
    // Shift cursor left.
    if(io.pressed(GLFW_KEY_J)) {
        drawOffset.x -= cursorSpeed;
    }
    // Shift cursor right.
    if(io.pressed(GLFW_KEY_L)) {
        drawOffset.x += cursorSpeed;
    }
    // Shift cursor upward (or forward if left shift is held).
    if(io.pressed(GLFW_KEY_I)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            drawOffset.z += cursorSpeed;
        } else {
            drawOffset.y += cursorSpeed;
        }
    }
    // Shift cursor downward (or backward if left shift is held).
    if(io.pressed(GLFW_KEY_K)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            drawOffset.z -= cursorSpeed;
        } else {
            drawOffset.y -= cursorSpeed;
        }
    }
    // Reset the cursor offset.
    if(io.pressed(GLFW_KEY_P)) {
        drawOffset = glm::vec3(0, 0, 0);
    }
    // Clamp the offset.
    clamp(drawOffset.x, -cursorBound, cursorBound);
    clamp(drawOffset.y, -5.f, cursorBound);
    clamp(drawOffset.z, -cursorBound, cursorBound);
    // Start drawing.
    if(io.toggled(GLFW_KEY_SPACE)) {
        if(world->state == edit) {
            drawStart = true;
        }
    }
    // Stop drawing.
    if(io.released(GLFW_KEY_SPACE)) {
        drawLive = false;
        drawDead = false;
    }

    // Create a Cube cube.
    if(io.toggled(GLFW_KEY_Q)) {
        makeCubes();
    }
    // Increase Cube cube half-width.
    if(io.toggled(GLFW_KEY_RIGHT_BRACKET)) {
        cubeHwidth += 1;
    }
    if(io.toggled(GLFW_KEY_LEFT_BRACKET)) {
        cubeHwidth -= 1;
        if(cubeHwidth < 1) {
            cubeHwidth = 1;
        }
    }
}

/**
 * Camera.makeCubes()
 * Makes a Cube cube - randomly activated Cubes within a cubic spatial region centered at the
 * draw cursor.
 */
void Camera::makeCubes() {
    // Only do it in edit mode.
    if(world->state == edit) {
        world->cubeCube(cubeHwidth, 0.1, drawCursor);
    }
}

/**
 * Camera.update()
 * Updates the Camera object.
 */
void Camera::update() {
    // Update time variables.
    lastTime = currentTime;
    currentTime = (float)glfwGetTime();
    deltaTime = currentTime - lastTime;

    // If reset_frames_left > 0, keep performing the reset instead of handling
    // input.
    if(reset_frames_left > 0) {
        reset_frames_left--;
        position = position - reset_dposition;
        horizontalAngle -= reset_dhorizontal;
        verticalAngle -= reset_dvertical;
    } else {
        // Handle input.
        handleInput();
    }

    // Update heading information.
    // Heading vector.
    heading = glm::vec3(
            std::cos(verticalAngle) * std::sin(horizontalAngle),
            std::sin(verticalAngle),
            std::cos(verticalAngle) * std::cos(horizontalAngle)
    );
    // Right vector.
    right = glm::vec3(
            std::sin(horizontalAngle - PI / 2),
            0,
            std::cos(horizontalAngle - PI / 2)
    );
    // Up vector.
    up = glm::cross(right, heading);

    // Update the View matrix.
    View = glm::lookAt(position, position + heading, up);

    // If the World is in the 'edit' state, update the drawing cursor.
    if(world->state == edit) {
        updateDraw();
    }
}

/**
 * Camera.updateDraw()
 * Updates the location of the drawing cursor.
 */
void Camera::updateDraw() {
    // Determines how far away from the Camera the base point is.
    const float dist = 9.f;

    // Base cursor location will be the Cube containing this point.
    glm::vec3 base = position + dist * heading;

    // Cursor offset vector.
    glm::vec3 offset = drawOffset.x * right +
                       drawOffset.y * heading +
                       drawOffset.z * up;

    // Location of the draw cursor.
    glm::vec3 cursor = base + offset;

    // Get the coordinates of the Cube containing the cursor.
    float iscale = 1.f / (2.f * world->scale);
    drawCursor.x = static_cast<int>(std::round(cursor.x * iscale));
    drawCursor.y = static_cast<int>(std::round(cursor.y * iscale));
    drawCursor.z = static_cast<int>(std::round(cursor.z * iscale));

    // activeCubes index of the cursor location.
    long int idx = world->key(drawCursor.x, drawCursor.y, drawCursor.z);

    // Indicates whether the Cube at the cursor location is in activeCubes
    bool inMap = world->findActiveCubes(idx);

    // Initialize drawing.
    if(drawStart) {
        drawStart = false;

        if(inMap) {
            // The Cube under the cursor is already in activeCubes.
            Cube *c = world->activeCubes[idx];

            if(c->live) {
                // The Cube is live. Draw dead Cubes.
                drawDead = true;
            } else {
                // The Cube is dead. Draw live Cubes.
                drawLive = true;
            }
        } else {
            // The Cube under the cursor is not already in activeCubes. So it's
            // dead, so draw live Cubes.
            drawLive = true;
        }
    }

    // Draw live Cubes at the cursor.
    if(drawLive) {
        if(inMap) {
            // Access the Cube.
            Cube *c = world->activeCubes[idx];

            if(!c->live) {
                // Only do something if the Cube is dead.
                world->flip(c);
            }
        } else {
            // No Cube in the activeCubes at the cursor. Create it, then flip its
            // state.
            world->add(drawCursor.x, drawCursor.y, drawCursor.z);
            world->flip(world->activeCubes[idx]);
        }
    }

    // Draw dead Cubes at the cursor.
    if(drawDead) {
        if(inMap) {
            // Access the Cube.
            Cube *c = world->activeCubes[idx];

            if(c->live) {
                // Only do something if the Cube is alive.
                world->flip(c);
            }
        }
        // No need to add a new Cube if the Cube under the cursor isn't
        // in activeCubes, since it'd just be dead.
    }
}