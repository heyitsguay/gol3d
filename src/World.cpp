//
// Created by mattguay on 1/23/16.
//

#include <chrono>
#include <cmath>
#include <cstdlib>
#include <random>
#include <tuple>
#include <unordered_map>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <SOIL/SOIL.h>

#include "Cube.h"
#include "World.h"

/**
 * World()
 * Initializes the World running a 3D version of Conway's Game of Life.
 * @constructor
 */
World::World() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {};

/**
 * ~World()
 * @destructor
 */
World::~World() {
    freeMemory();

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);

    glDeleteBuffers(1, &translationVBO);
    glDeleteBuffers(1, &scaleVBO);
};

/**
 * World.add()
 * Adds a Cube to activeCubes with index idx and state (state).
 * @param idx: Unique index to a 3D spatial location for a Cube.
 * @param state: Cube state.
 */
void World::add(int x, int y, int z) {

    // Only add a Cube if limbo isn't empty.
    if(!limbo.empty()) {
        long int k = key(x, y, z);
        // Add the Cube if it's not already in activeCubes.
        if(!findActiveCubes(k)) {
            // Remove a Cube from limbo.
            Cube *c = limbo.front();
            limbo.pop();

            // Set the Cube up.
            c->setup(x, y, z, false);

            // Add the Cube to activeCubes.
            activeCubes.insert({k, c});

            // Increment nCubes.
            nCubes++;
        }
    } else {
        printf("You ran out of spare Cubes!\n");
    }
}

/**
 * World.cubeCube()
 * Creates a cube active Cubes to the World at each (x, y, z) location
 * in the cube center + [-hwidth, hwidth]^3 with probability p.
 * @param hwidth: Setup volume half-width.
 * @param p: Cube activation probability.
 * @param center: Center of the cube of Cubes.
 */
void World::cubeCube(int hwidth, float p, glm::ivec3 center) {
    // Use a RNG to randomly add active cells to a cube of dimensions
    // [-hwidth, hwdith]^3.
    // obtain a seed from the system clock:
    auto seed1 = static_cast<unsigned int>(std::chrono::system_clock::now().time_since_epoch().count());

    std::mt19937 gen(seed1);
    std::uniform_real_distribution<float> u((float)0., (float)1.);

    int cx = center.x;
    int cy = center.y;
    int cz = center.z;

    for(int x = cx-hwidth; x <= cx+hwidth; ++x) {
        for(int y = cy-hwidth; y <= cy+hwidth; ++y) {
            for(int z = cz-hwidth; z <= cz+hwidth; ++z) {
                float v = u(gen);
                // Add an active Cube at (x,y,z) with probability p.
                if(v < p) {
                    add(x, y, z);
                    flip(activeCubes[key(x, y, z)]);
                }
            }
        }
    }
}

/**
 * World.draw()
 * Draws the World.
 * @param t: Simulation time.
 * @param camPosition: Position of the Camera.
 */
void World::draw(float t) {
    // Tracks the number of Cubes to draw.
    drawCount = 0;

    // Clear the translation and scale data arrays.
    translations.clear();
    scales.clear();

    // Update drawn Cube translation and scale info.
    for(auto it = drawCubes.begin(); it != drawCubes.end(); ++it) {
        Cube *c = it->second;
        auto translation = glm::vec3(c->x * scale2, c->y * scale2, c->z * scale2);
        glm::vec3 v_to_camera = translation - cam.position;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
        float d2_to_camera = glm::dot(v_to_camera, v_to_camera);
#pragma clang diagnostic pop
        if(d2_to_camera < cam.drawDistance * cam.drawDistance) {
            // Close enough to the Camera, should be drawn.
            translations.push_back(translation);
            scales.push_back(scale);

            drawCount++;
        }
    }

    // Prepare to draw.
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_CULL_FACE);
    glUseProgram(*program);

    // Draw.
    if(drawCount > 0) {
        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &(cam.VP)[0][0]);
        glUniform3fv(ucamera_pos, 1, &cam.position[0]);
        glUniform1f(ut, t);
        glUniform1i(sAtlas, 1);

        glBindBuffer(GL_ARRAY_BUFFER, translationVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(glm::vec3) * drawCount,
                     &translations[0][0],
                     GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(float) * drawCount,
                     &scales[0],
                     GL_DYNAMIC_DRAW);

        glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)drawCount);
    }

}

/**
 * World.findActiveCubes()
 * Returns true if the Cube with index (idx) is contained in activeCubes.
 * @param idx: Index of the Cube to look up.
 */
bool World::findActiveCubes(long int idx) {
    return (activeCubes.find(idx) != activeCubes.end());
}

/**
 * World.findAddCubes()
 * Returns true if the Cube with index (idx) is contained in addCubes.
 * @param idx: Index of the Cube to look up.
 */
bool World::findAddCubes(long int idx) {
    return (addCubes.find(idx) != addCubes.end());
}

/**
 * World.flip()
 * Toggles the state of Cube *c, then adds its neighbors to activeCubes.
 * @param c: Pointer to the Cube whose state is being toggled.
 */
void World::flip(Cube *c) {
    // Flip the Cube state.
    c->live = !(c->live);

    // Update the Cube's position in drawCubes.
    long int idx = key(c->x, c->y, c->z);
    if(!c->live) {
        // Cube is now dead, remove from drawCubes.
        drawCubes.erase(idx);
    } else {
        // Cube is now live, add to drawCubes.
        drawCubes.insert({idx, c});
    }

    // Add neighbors to addCubes if they're not they're already.
    for (int dx = -1; dx <= 1; ++dx) {
        int X = c->x + dx;
        for (int dy = -1; dy <= 1; ++dy) {
            int Y = c->y + dy;
            for (int dz = -1; dz <= 1; ++dz) {
//                if (!(dx == 0 && dy == 0 && dz == 0)) {
                    int Z = c->z + dz;

                    // Add neighbor if its coordinate is in bounds.
//                    if( true
////                            (X >= -bound/2 && X < bound/2) &&
////                            (Y >= -bound/2 && Y < bound/2) &&
////                            (Z >= -bound/2 && Z < bound/2)
//                            ) {
                    long int k = key(X, Y, Z);
                    auto coord = std::make_tuple(X, Y, Z);
                    if (!findAddCubes(k)) {
                        addCubes.insert({k, coord});
                    }
//                    }
//                }
            }
        }
    }
}

/**
 * World.freeMemory()
 * Frees all memory associated with Cubes (deletes them).
 */
void World::freeMemory() {
// Delete Cubes in the limbo queue.
    auto ls = (int)limbo.size();
    for(int i = 0; i < ls; ++i) {
        Cube *c = limbo.front();
        delete c;
        limbo.pop();
    }

    // Delete Cubes in activeCubes.
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        delete it->second;
    }
}

/**
 * World.handleInput()
 * Handles events triggered by user input.
 */
void World::handleInput() {
    // World state changes use keys 1, 2, 3.
    if(io.toggled(GLFW_KEY_1)) {
        state = stop;

    } else if(io.toggled(GLFW_KEY_2)) {
        state = edit;

        // Push to the end of the update cycle to make sure newly-inserted Cubes
        // are updated properly.
        fcount = 3;

    } else if(io.toggled(GLFW_KEY_3)) {
        state = run;

    } else if(io.toggled(GLFW_KEY_E)) {
        // Only increment if we're not in the middle of another step, or running.
        if(stepCounter == 0 && state != run) {
            // Advance the World one step.
            stepCounter = frames_per_draw;
        }
    }



    // Check for a reset command.
    if(io.pressed(GLFW_KEY_R)) {
        reset();
    }
}

/**
 * World.init()
 * Initializes the World.
 * @param cubeVAO_: Pointer to the main cube VAO.
 * @param program_: Pointer to the ID of the World's shader program.
 * @param MVP_: Pointer to the Model-View-Projection matrix.
 * @param scale_: Spatial scale of the World's Cubes.
 * @param frames_per_draw_: Number of frames that the World's update cycle takes.
 * @param maxCubes_: Maximum number of simultaneously active Cubes.
 * @param bound_: Cubes are constrained to lie in the box [-bound_, bound_]^3.
 */
void World::init(
        GLuint *cubeVAO_,
        GLuint *program_,
        float scale_,
        int frames_per_draw_,
        int maxCubes_,
        int bound_
    ) {

    cubeVAO = cubeVAO_;

    program = program_;

    scale = scale_;
    scale2 = 2 * scale;

    frames_per_draw = frames_per_draw_;

    maxCubes = maxCubes_;

    bound = bound_;

    state = stop;

    initGL();

    reset();
}

/**
 * World.initGL()
 * Initializes the OpenGL elements associated with the World.
 */
void World::initGL() {
    // Set up the translation and scale data arrays.
    translations.reserve((unsigned long)maxCubes / 64);
    scales.reserve((unsigned long)maxCubes / 64);

    // Get GLSL uniform pointers.
    uMVP = (GLuint) glGetUniformLocation(*program, "u_MVP");
    ucamera_pos = (GLuint) glGetUniformLocation(*program, "u_camera_pos");
    ut = (GLuint) glGetUniformLocation(*program, "u_t");

    // Create the translation VBO.
    glGenBuffers(1, &translationVBO);
    glBindBuffer(GL_ARRAY_BUFFER, translationVBO);
    glVertexAttribPointer(
            2, // attribute number.
            3, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // offset pointer.
    );
    glVertexAttribDivisor(2, (GLuint)1);
    glEnableVertexAttribArray(2);

    // Create the scale VBO.
    glGenBuffers(1, &scaleVBO);
    glBindBuffer(GL_ARRAY_BUFFER, scaleVBO);
    glVertexAttribPointer(
            3,
            1,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
    );
    glVertexAttribDivisor(3, (GLuint)1);
    glEnableVertexAttribArray(3);

    // Create texture, load image data.
    glGenTextures(1, &atlasTex);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, atlasTex);
    // Image width and height.
    int width, height;
    unsigned char *image = SOIL_load_image("../../data/img/cubeatlas.png",
                                           &width,
                                           &height,
                                           0,
                                           SOIL_LOAD_RGBA);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, image);
    SOIL_free_image_data(image);
    // Set texture params.
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glGenerateMipmap(GL_TEXTURE_2D);

    sAtlas = (GLuint)glGetUniformLocation(*program, "s_atlas");
}

/**
 * World.key()
 * Computes a hash key from a coord_t element.
 * @param c: (x, y, z) coordinate.
 */
long int World::key(int x, int y, int z) {
    int b2 = bound / 2;
    return (long int)(x + b2) * (long int) bound * (long int)bound
           + (long int)(y + b2) * (long int)bound
            + (long int)(z + b2);
}

/**
 * World.remove()
 * Removes a Cube with center (x, y, z) from activeCubes and pushes it back
 * to limbo if it's dead.
 * @param x: Center x coordinate.
 * @param y: Center y coordinate.
 * @param z: Center z coordinate.
 */
void World::remove(long int idx) {
    if(findActiveCubes(idx)) {
        // Push dead Cubes back to limbo.
        Cube *c = activeCubes[idx];
        if(!c->live) {
            limbo.push(c);
        }
        activeCubes.erase(idx);

        nCubes--;
    }
}

/**
 * World.reset()
 * Resets the World.
 */
void World::reset() {
    // Clear all the containers.
    freeMemory();
    activeCubes.clear();
    drawCubes.clear();
    addCubes.clear();
    removeCubes.clear();

    // Initialize fcount.
    fcount = 0;

    // Construct all the Cubes.
    for(int i = 0; i < maxCubes; ++i) {
        limbo.push(new Cube());
    }

    // Number of active Cubes.
    nCubes = 0;
}

/**
 * World.setRule()
 * Sets the update rule for the Cube states.
 * @param stay_vals: A live Cube stays live if its live neighbor count is an element of this vector.
 * @param born_vals: A dead Cube becomes live if its live neighbor count is an element of this vector.
 */
void World::setRule(std::vector<int> stay_vals, std::vector<int> born_vals) {
    // Reset the rule arrays.
    for(int i = 0; i < 27; ++i) {
        stay[i] = false;
        born[i] = false;
    }

    // Rule setup.
    for(auto& s : stay_vals) {
        if(s >= 0 && s < 27) {
            stay[s] = true;
        }
    }
    for(auto& b : born_vals) {
        if(b >= 0 && b < 27) {
            born[b] = true;
        }
    }
}

/**
 * World.update()
 * Updates the Cubes in activeCubes.
 * @param t: Current frame's count within [0, frames_per_draw).
 */
void World::update() {

    // Handle user input.
    handleInput();

    if (state == run || stepCounter > 0) {

        // Decrement stepCounter.
        if(stepCounter > 0) {
            stepCounter--;
        }

        // Spread the update process over successive frames.
        fcount = (fcount + 1) % frames_per_draw;

        if(fcount == 1) {
            updateActiveCubes();

        } else if(fcount == 2) {
            updateNeighborCount();

        } else if(fcount == 3) {
            updateState();

        } else if(fcount == 4) {
            updateResetCount();
        }
    }
}

/**
 * World.updateActiveCubes()
 * Processes removeCubes and addCubes to update activeCubes.
 */
void World::updateActiveCubes() {
    // First, remove inactive Cubes from activeCubes.
    for (auto &center : removeCubes) {
        remove(center);
    }

    // Second, add newly-active Cubes to activeCubes.
    for (auto it = addCubes.begin(); it != addCubes.end(); ++it) {
        std::tuple<int, int, int> center = it->second;
        int x = std::get<0>(center);
        int y = std::get<1>(center);
        int z = std::get<2>(center);
        add(x, y, z);
    }

    // Clear addCubes and removeCubes.
    addCubes.clear();
    removeCubes.clear();
}

/**
 * World.updateNeighborCount()
 * Counts the number of live Cubes neighboring each Cube in activeCubes.
 */
void World::updateNeighborCount() {
    // First pass: compute number of living neighbors for each Cube that
    // needs to be updated.
    for (auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        // If c is live...
        if (c->live) {
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
                            // its live_neighbors if it does.
                            long int k = key(X, Y, Z);
                            if (findActiveCubes(k)) {
                                activeCubes[k]->live_neighbors++;
                            }
                        }

                    }
                }
            }
        }
    }
}

/**
 * World.updateResetCount()
 * Resets the live_neighbors property to 0 for all Cubes in activeCubes.
 */
void World::updateResetCount() {
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        c->live_neighbors = 0;
    }
}

/**
 * World.updateState()
 * Updates the state of each Cube in activeCubes.
 */
void World::updateState() {
    for (auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        // Check if a live Cube should stay.
        if (c->live) {
            // Flip state if live_neighbors isn't a valid stay[] value.
            if (!stay[c->live_neighbors]) {
                flip(c);
            }

            // Check if a dead Cube should become live.
        } else {
            // Cube becomes live.
            if (born[c->live_neighbors]) {
                flip(c);
            } else {
                // Dead cell stays dead.
                removeCubes.push_back(key(c->x, c->y, c->z));
            }
        }

        // Reset live_neighbors.
//        c->live_neighbors = 0;
    }
}

// End World.cpp
