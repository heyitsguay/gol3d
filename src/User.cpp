//
// Created by matt on 1/28/16.
//

#include <glm/gtc/matrix_transform.hpp>

#include "User.h"

User::User() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {};

/**
 * User.computeHeadingBasis()
 * Computes an orthonormal basis of 3D vectors comprised of the current heading,
 * and two vectors pointing to the right of and upward from that heading.
 */
void User::computeHeadingBasis() {
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
}

/**
 * User.draw()
 * Currently, just draws the edit cursor.
 */
void User::draw() {
    // Only draw if the world is in edit mode.
    if(world->state == edit) {
        // Always going to start with the identity Model matrix.
        const glm::mat4 Model(1.f);

        // Scaling vector.
        glm::vec3 vScale(world->scale);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glUseProgram(*programCursor);

        // Compute the cursor Cube translation.
        auto translation = 2.f * glm::vec3(drawCursor) * vScale;

        glm::mat4 translatedModel = glm::scale(glm::translate(Model, translation), vScale);
        glm::mat4 MVP = cam.VP * translatedModel;

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);

        glDrawArrays(GL_TRIANGLES, 0, 36);
    }
}

/**
 * User.handleInput()
 * Handles user input.
 */
void User::handleInput() {
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
    myclamp(verticalAngle, -PI/2, PI/2);
    // Keep the horizontal angle in [0, 2*PI)
    horizontalAngle = float(pmod(horizontalAngle, 2*PI));

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
            position += up * dTime * speed;
        } else {
            position += heading * dTime * speed;
        }
    }
    // Move backward (or downward if left Shift is pressd)
    if(io.pressed(GLFW_KEY_DOWN)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            position -= up * dTime * speed;
        } else {
            position -= heading * dTime * speed;
        }
    }
    // Strafe right
    if(io.pressed(GLFW_KEY_RIGHT)) {
        position += right * dTime * speed;
    }
    // Strafe left
    if(io.pressed(GLFW_KEY_LEFT)) {
        position -= right * dTime * speed;
    }

    // Check for a reset.
    if(io.pressed(GLFW_KEY_T)) {
        reset_frames_left = num_reset_frames;
        speed = 0.f;
        reset_dposition = (position - position0) / float(num_reset_frames);
        reset_dhorizontal = (horizontalAngle - horizontalAngle0) / float(num_reset_frames);
        reset_dvertical = (verticalAngle - verticalAngle0) / float(num_reset_frames);
    }

    // Shift cursor left.
    if(io.pressed(GLFW_KEY_J)) {
        cursorOffset.x -= cursorSpeed;
    }
    // Shift cursor right.
    if(io.pressed(GLFW_KEY_L)) {
        cursorOffset.x += cursorSpeed;
    }
    // Shift cursor upward (or forward if left shift is held).
    if(io.pressed(GLFW_KEY_I)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            cursorOffset.z += cursorSpeed;
        } else {
            cursorOffset.y += cursorSpeed;
        }
    }
    // Shift cursor downward (or backward if left shift is held).
    if(io.pressed(GLFW_KEY_K)) {
        if(io.pressed(GLFW_KEY_LEFT_SHIFT)) {
            cursorOffset.z -= cursorSpeed;
        } else {
            cursorOffset.y -= cursorSpeed;
        }
    }
    // Reset the cursor offset.
    if(io.pressed(GLFW_KEY_P)) {
        cursorOffset = glm::vec3(0, 0, 0);
    }
    // Clamp the offset.
    myclamp(cursorOffset.x, -cursorBound, cursorBound);
    myclamp(cursorOffset.y, -5.f, cursorBound);
    myclamp(cursorOffset.z, -cursorBound, cursorBound);
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
 * User.init()
 * Initializes the User.
 * @param world_: Pointer to the TempName.
 * @param position_: User initial position.
 * @param horizontalAngle_: User initial heading horizontal angle.
 * @param verticalAngle_: User initial heading vertical angle.
 */
void User::init(TempName *world_,
                GLuint *programCursor_,
                glm::vec3 position_,
                float horizontalAngle_,
                float verticalAngle_) {
    world = world_;
    programCursor = programCursor_;
    position = position_;
    horizontalAngle = horizontalAngle_;
    verticalAngle = verticalAngle_;
    // Set the reset state.
    position0 = position;
    horizontalAngle0 = horizontalAngle;
    verticalAngle0 = verticalAngle;

    tPrev = 0.;
    speed = 0.f;

    // Computes the heading, right, and up vectors from heading angles.
    computeHeadingBasis();

    cursorOffset = glm::vec3(0, 0, 0);
    drawCursor = glm::ivec3(0, 0, 0);

    drawStart = false;
    drawLive = false;
    drawDead = false;
    cubeHwidth = 4;

    // OpenGL setup.
    uMVP = (GLuint)glGetUniformLocation(*programCursor, "u_MVP");
}

/**
 * User.makeCubes()
 * Makes a Cube cube - randomly activated Cubes within a cubic spatial region centered at the
 * draw cursor.
 */
void User::makeCubes() {
    // Only do it in edit mode.
    if(world->state == edit) {
        world->cubeCube(cubeHwidth, 0.1, drawCursor);
    }
}

/**
 * User.update()
 * Updates the User.
 * @param t: Current time since the simulation began.
 */
void User::update(double t) {
    // Update time variables.
    dTime = float(t - tPrev);
    tPrev = t;

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
    computeHeadingBasis();

    // Update the Camera's position, heading, and up vectors.
    cam.position = position;
    cam.heading = heading;
    cam.up = up;

    // If the TempName is in the 'edit' state, update the drawing cursor.
    if(world->state == edit) {
        updateDraw();
    }
}

/**
 * User.updateDraw()
 * Updates the drawing cursor, and performs any drawing actions.
 */
void User::updateDraw() {
    // Base cursor location will be the Cube containing this point.
    glm::vec3 base = position + baseDrawDist * heading;

    // Cursor offset vector.
    glm::vec3 offset = cursorOffset.x * right +
                       cursorOffset.y * heading +
                       cursorOffset.z * up;

    // Location of the draw cursor.
    glm::vec3 cursor = base + offset;

    // Get the coordinates of the Cube containing the cursor.
    float iscale = 1.f / (2.f * world->scale);
    drawCursor.x = static_cast<int>(std::round(cursor.x * iscale));
    drawCursor.y = static_cast<int>(std::round(cursor.y * iscale));
    drawCursor.z = static_cast<int>(std::round(cursor.z * iscale));

    // activeCubes index of the cursor location.
    auto key = glm::ivec3(drawCursor.x, drawCursor.y, drawCursor.z);

    // Indicates whether the Cube at the cursor location is in activeCubes
    bool inMap = world->findIn(world->activeCubes, key);

    // Initialize drawing.
    if(drawStart) {
        drawStart = false;

        if(inMap) {
            // The Cube under the cursor is already in activeCubes.
            Cube *c = world->activeCubes[key];

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
            Cube *c = world->activeCubes[key];

            if(!c->live) {
                // Only do something if the Cube is dead.
                world->flip(c);
            }
        } else {
            // No Cube in the activeCubes at the cursor. Create it, then flip its
            // state.
            world->add(drawCursor.x, drawCursor.y, drawCursor.z);
            world->flip(world->activeCubes[key]);
        }
    }

    // Draw dead Cubes at the cursor.
    if(drawDead) {
        if(inMap) {
            // Access the Cube.
            Cube *c = world->activeCubes[key];

            if(c->live) {
                // Only do something if the Cube is alive.
                world->flip(c);
            }
        }
        // No need to add a new Cube if the Cube under the cursor isn't
        // in activeCubes, since it'd just be dead.
    }

}