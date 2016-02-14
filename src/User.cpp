//
// Created by matt on 1/28/16.
//
#include "User.h"

#include <algorithm>

#include <glm/gtc/matrix_transform.hpp>

User::User() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {};

/**
 * User.addSelectionPoint()
 * Adds a new selection point to selectedRegion at index 1 - lastSelectIdx.
 * @param point: Location of the new selection point.
 */
void User::addSelectionPoint(glm::ivec3 &point) {
    if(numSetSelections == 2) {
        numSetSelections = 0;
    }
    selectedRegion[numSetSelections] = point;
    numSetSelections++;
}

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
 * User.computeRegionBounds()
 * Computes the x, y, z bounds of the currentRegion in increasing order.
 * (Sets private variables x0, x1, y0, y1, z0, z1).
 */
void User::computeRegionBounds() {
    x0 = std::min(currentRegion[0].x, currentRegion[1].x);
    x1 = std::max(currentRegion[0].x, currentRegion[1].x);
    y0 = std::min(currentRegion[0].y, currentRegion[1].y);
    y1 = std::max(currentRegion[0].y, currentRegion[1].y);
    z0 = std::min(currentRegion[0].z, currentRegion[1].z);
    z1 = std::max(currentRegion[0].z, currentRegion[1].z);
}

/**
 * User.copy()
 * Copies the non-dead Cubes in the currently-selected region to the clipBoard.
 */
void User::copy() {
    if(state == selection) {
        // Clear the clipBoard.
        clipBoard.clear();

        // The current Object.
        Object *obj = (*activeObj);

        // Sets x0, x1, y0, y1, z0, z1.
        computeRegionBounds();

        // Check each Cube in the currentRegion, indexed by their centers.
        glm::ivec3 center;
        for (int x = x0; x <= x1; ++x) {
            center.x = x;
            for (int y = y0; y <= y1; ++y) {
                center.y = y;
                for (int z = z0; z <= z1; ++z) {
                    center.z = z;
                    // Save any non-dead Cubes, in region-relative coordinates.
                    if (obj->findIn(obj->drawCubes, center)) {
                        clipBoard.insert({center - currentRegion[0], obj->activeCubes[center]->state == 2});
                    }
                }
            }
        }
    }
}

/**
 * User.cut()
 * Cuts the currently-selected region (copy then delete).
 */
void User::cut() {
    copy();
    deleteRegion();
}

/**
 * User.deleteRegion()
 * Deletes the region indicated in currentRegion.
 */
void User::deleteRegion() {
    if(state == selection) {
        // The current object.
        auto obj = dynamic_cast<CellularAutomaton *>(*activeObj);

        // Get the bounds of the current region.
        computeRegionBounds();

        // Iterate through the region. Remove any non-dead Cubes.
        glm::ivec3 center;
        for (int x = x0; x <= x1; ++x) {
            center.x = x;
            for (int y = y0; y <= y1; ++y) {
                center.y = y;
                for (int z = z0; z <= z1; ++z) {
                    center.z = z;
                    if (obj->findIn(obj->drawCubes, center)) {
                        obj->setCube(obj->activeCubes[center], 0);
                    }
                }
            }
        }
    }
}

/**
 * User.draw()
 * Currently, just draws the edit cursor.
 */
void User::draw() {
    const static glm::mat4 Model(1.f);
    // Draw a single white Cube if the User is in edit state.
    if(state == edit || state == selection) {

        // Scaling vector.
        glm::vec3 vScale((*activeObj)->scale);

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glUseProgram(*programCursor);

        // Compute the cursor Cube translation.
        auto translation = 2.f * glm::vec3(drawCursor) * vScale;

        glm::mat4 newModel = glm::scale(glm::translate(Model, translation), vScale);
        glm::mat4 MVP = cam.VP * newModel;

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);
        if(state == edit) {
            glUniform1i(uColorState, 0);
        } else {
            glUniform1i(uColorState, 3);
        }

        glDrawArrays(GL_TRIANGLES, 0, 36);

    }
    if(state == selection && numSetSelections > 0) {
        // Scaling vector.
        glm::vec3 vScale = (*activeObj)->scale * regionScale;

        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glUseProgram(*programCursor);

        auto translation = 2.f * (*activeObj)->scale * regionCenter;

        glm::mat4 newModel = glm::scale(glm::translate(Model, translation), vScale);
        glm::mat4 MVP = cam.VP * newModel;


        // Indicates which color to use to draw the Cursor cube.
        int colorState = numSetSelections;

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);
        glUniform1i(uColorState, colorState);

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

    // Enter or exit edit mode.
    if(io.toggled(GLFW_KEY_F)) {
        (*activeObj)->state = stop;
        if(state == move) {
            state = edit;
        } else {
            state = move;
        }
    } else if(io.pressed(GLFW_KEY_3)) {
        state = move;
    }
    // Enter select mode, if already in edit mode.
    if(state == edit && io.toggled(GLFW_KEY_LEFT_CONTROL)) {
        state = selection;
        numSetSelections = 0;
    }
    // Exit select mode, go to edit mode.
    if(state == selection && io.released(GLFW_KEY_LEFT_CONTROL)) {
        state = edit;
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

    // Start drawing, if in edit mode.
    // Add a selection point, if in select mode.
    if(io.toggled(GLFW_KEY_SPACE)) {
        if(state == edit) {
            drawStart = true;

        } else if(state == selection) {
            addSelectionPoint(drawCursor);
        }
    }
    // Stop drawing.
    if(io.released(GLFW_KEY_SPACE)) {
        drawLive = false;
        drawDead = false;
        drawDying = false;
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

    // Delete the currently-selected region.
    if(io.toggled(GLFW_KEY_Z)) {
        deleteRegion();
    // Cut the currently-selected region to the clipBoard.
    } else if(io.toggled(GLFW_KEY_X)) {
        cut();
    // Copy the currently-selected region to the clipBoard.
    } else if(io.toggled(GLFW_KEY_C)) {
        copy();
    // Paste the region in the clipBoard to the activeObj.
    } else if(io.toggled(GLFW_KEY_V)) {
        paste();
    }

    // Controls how quickly cube density increases and decreases.
    const float dCubeP = 0.002f;
    // Increase Cube cube density.
    if(io.pressed(GLFW_KEY_PERIOD)) {
        cubeP = std::min(cubeP + dCubeP, 1.f);
    // Decrease Cube cube density.
    } else if(io.pressed(GLFW_KEY_COMMA)) {
        cubeP = std::max(cubeP - dCubeP, 0.f);
    // Reset Cube cube density.
    } else if(io.toggled(GLFW_KEY_SLASH)) {
        cubeP = 0.1f;
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
void User::init(World *world_,
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

    activeObj = &(world->activeObject);

    // Initialize state to just be moving (no editing).
    state = move;

    tPrev = 0.;
    speed = 0.f;

    // Computes the heading, right, and up vectors from heading angles.
    computeHeadingBasis();

    cursorOffset = glm::vec3(0, 0, 0);
    drawCursor = glm::ivec3(0, 0, 0);
    drawStart = false;
    drawLive = false;
    drawDead = false;
    drawDying = false;
    cubeHwidth = 4;

    // Initialize clipBoard stuff.
    for(int i = 0; i < 2; ++i) {
        selectedRegion[i] = glm::ivec3(0, 0, 0);
    }
    dRegion = glm::ivec3(0, 0, 0);
    regionCenter = glm::vec3(0.f, 0.f, 0.f);
    regionScale = glm::vec3(1.f, 1.f, 1.f);
    numSetSelections = 0;
    clipBoard.clear();

    // OpenGL setup.
    uMVP = (GLuint)glGetUniformLocation(*programCursor, "u_MVP");
    uColorState = (GLuint) glGetUniformLocation(*programCursor, "u_colorState");
}

/**
 * User.makeCubes()
 * Makes a Cube cube - randomly activated Cubes within a cubic spatial region centered at the
 * draw cursor.
 */
void User::makeCubes() {
    // Only do it in edit mode.
    if(state == edit) {
        auto obj = dynamic_cast<CellularAutomaton*>(*activeObj);
        obj->cubeCube(cubeHwidth, cubeP, drawCursor);
    }
}

/**
 * User.paste()
 * Pastes the clipBoard's contents to a region offset by the current
 * drawCursor.
 */
void User::paste() {
    if(clipBoard.size() > 0) {
        auto obj = dynamic_cast<CellularAutomaton*>(*activeObj);

        for(auto it = clipBoard.begin(); it != clipBoard.end(); ++it) {
            glm::ivec3 center = drawCursor + it->first;
            int cubeState = 1 + static_cast<int>(it->second);
            obj->add(center.x, center.y, center.z);
            obj->setCube(obj->activeCubes[center], cubeState);
        }
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
    if(state == edit || state == selection) {
        updateEdit();
    }
    if(state == selection) {
        updateSelect();
    }
}

/**
 * User.updateDraw()
 * Updates the drawing cursor, and performs any drawing actions.
 */
void User::updateEdit() {
    // Object we're drawing. Assume it's a CellularAutomaton for now. TODO: make this better.
    CellularAutomaton* obj = dynamic_cast<CellularAutomaton*>(*activeObj);

    // Base cursor location will be the Cube containing this point.
    glm::vec3 base = position + baseDrawDist * heading;

    // Cursor offset vector.
    glm::vec3 offset = cursorOffset.x * right +
                       cursorOffset.y * heading +
                       cursorOffset.z * up;

    // Location of the draw cursor.
    glm::vec3 cursor = base + offset;

    drawCursor = obj->centerFromPoint(cursor);

    // activeCubes index of the cursor location.
    auto key = glm::ivec3(drawCursor.x, drawCursor.y, drawCursor.z);

    // Indicates whether the Cube at the cursor location is in activeCubes
    bool inMap = obj->findIn(obj->activeCubes, key);

    // Initialize drawing.
    if(drawStart) {
        drawStart = false;

        if(inMap) {
            // The Cube under the cursor is already in activeCubes.
            Cube *c = obj->activeCubes[key];

            if(c->state == 1) {
                // Cube *c is live.
                if (obj->numStates == 2) {
                    // Game of Life mode, next state is dead.
                    drawDead = true;
                } else {
                    // Brian's brain mode, next state is dying.
                    drawDying = true;
                }

            } else if(c->state == 2) {
                // Cube *c is dying, next state is dead.
                drawDead = true;

            } else {
                // Cube *c is dead, next state is dying.
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
            Cube *c = obj->activeCubes[key];

            if(c->state != 1) {
                // Only do something if the Cube is not live.
                obj->setCube(c, 1);
            }
        } else {
            // No Cube in the activeCubes at the cursor. Create it, then set its
            // state.
            obj->add(drawCursor.x, drawCursor.y, drawCursor.z);
            obj->setCube(obj->activeCubes[key], 1);
        }

    // Draw dying Cubes at the Cursor.
    } else if(drawDying) {
        if(inMap) {
            // Access the Cube.
            Cube *c = obj->activeCubes[key];

            if(c->state != 2) {
                // Only do something if the Cube is not dying.
                obj->setCube(c, 2);
            }
        } else {
            // No Cube in activeCubes at the cursor. Create it, then set its state.
            obj->add(drawCursor.x, drawCursor.y, drawCursor.z);
            obj->setCube(obj->activeCubes[key], 2);
        }
    }

    // Draw dead Cubes at the cursor.
    if(drawDead) {
        if(inMap) {
            // Access the Cube.
            Cube *c = obj->activeCubes[key];

            if(c->state != 0) {
                // Only do something if the Cube isn't dead
                obj->setCube(c, 0);
            }
        }
        // No need to add a new Cube if the Cube under the cursor isn't
        // in activeCubes, since it'd just be dead.
    }
}

/**
 * User.updateSelect()
 * Updates the selection region.
 */
void User::updateSelect() {
    currentRegion[0] = selectedRegion[0];
    if(numSetSelections == 2) {
        // A complete user selection has been made.
        currentRegion[1] = selectedRegion[1];

    } else {
        // Only the first point has been selected, use drawCursor for
        // the second.
        currentRegion[1] = drawCursor;
    }
    // Update region info.
    regionCenter = glm::vec3(currentRegion[0] + currentRegion[1]) / 2.f;
    regionScale = glm::abs(glm::vec3(currentRegion[0] - currentRegion[1])) + glm::vec3(1., 1., 1.);
}