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
#include "TempName.h"

#include "opengl-debug.h"

/**
 * TempName()
 * Initializes the TempName running a 3D version of Conway's Game of Life.
 * @constructor
 */
TempName::TempName() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {};

/**
 * ~TempName()
 * @destructor
 */
TempName::~TempName() {
    freeMemory();

    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glDeleteBuffers(1, &translationVBO);
    glDeleteBuffers(1, &scaleVBO);
    glDeleteBuffers(1, &typeVBO);
};

/**
 * TempName.add()
 * Adds a Cube to activeCubes with index idx and state (state).
 * @param idx: Unique index to a 3D spatial location for a Cube.
 * @param state: Cube state.
 */
void TempName::add(int x, int y, int z) {

    auto k = glm::ivec3(x, y, z);

    // If it's not empty, add a Cube from limbo.
    if(!limbo.empty()) {
        // Add the Cube if it's not already in activeCubes.
        if(!findIn(activeCubes, k)) {
            // Remove a Cube from limbo.
            Cube *c = limbo.back();
            limbo.pop_back();

            // Set the Cube up.
            c->setup(x, y, z);

            // Add the Cube to activeCubes.
            activeCubes.insert({k, c});

            // Increment nCubes.
            nCubes++;
        }
    } else {
        Cube *c = new Cube();
        c->setup(x, y, z);
        activeCubes.insert({k, c});
    }
}

/**
 * TempName.cubeCube()
 * Creates a cube active Cubes to the TempName at each (x, y, z) location
 * in the cube center + [-hwidth, hwidth]^3 with probability p.
 * @param hwidth: Setup volume half-width.
 * @param p: Cube activation probability.
 * @param center: Center of the cube of Cubes.
 */
void TempName::cubeCube(int hwidth, float p, glm::ivec3 center) {
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
                    flip(activeCubes[glm::ivec3(x, y, z)]);
                }
            }
        }
    }
}

/**
 * TempName.draw()
 * Draws the TempName.
 * @param t: Simulation time.
 * @param camPosition: Position of the Camera.
 */
void TempName::draw(float t) {
    // Tracks the number of Cubes to draw.
    drawCount = 0;

    // Clear the translation and scale data arrays.
    translations.clear();
    scales.clear();
    types.clear();

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
            types.push_back(c->texBase);

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

        glBindBuffer(GL_ARRAY_BUFFER, typeVBO);
        glBufferData(GL_ARRAY_BUFFER,
                     sizeof(glm::ivec2) * drawCount,
                     &types[0][0],
                     GL_DYNAMIC_DRAW);

        GL_CHECK( glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei)drawCount) );
    }

}

/**
 * TempName.findIn()
 * Returns true if the Cube with index (idx) is contained in the hashmap (map).
 * @param map: The hashmap to search for idx in.
 * @param idx: The hashmap key to search for.
 */
template<typename T> bool TempName::findIn(std::unordered_map<glm::ivec3, T, KeyFuncs, KeyFuncs> &map, glm::ivec3 idx) {
    return (map.find(idx) != map.end());
}
template bool TempName::findIn<Cube*>(cubeMap_t &map, glm::ivec3 idx);
template bool TempName::findIn<bool>(boolMap_t &map, glm::ivec3 idx);

/**
 * TempName.flip()
 * Toggles the state of Cube *c, then adds its neighbors to activeCubes.
 * @param c: Pointer to the Cube whose state is being toggled.
 */
void TempName::flip(Cube *c) {
    // Flip the Cube state.
    c->live = !(c->live);

    // Update the Cube's position in drawCubes.
    auto key = glm::ivec3(c->x, c->y, c->z);
    if(!c->live) {
        // Cube is now dead, remove from drawCubes.
        drawCubes.erase(key);
    } else {
        // Cube is now live, add to drawCubes.
        drawCubes.insert({key, c});
    }

    // Add neighbors to addCubes if they're not they're already.
    for (int dx = -1; dx <= 1; ++dx) {
        int X = c->x + dx;
        for (int dy = -1; dy <= 1; ++dy) {
            int Y = c->y + dy;
            for (int dz = -1; dz <= 1; ++dz) {
                int Z = c->z + dz;

                auto center = glm::ivec3(X, Y, Z);
                if (!findIn(addCubes, center)) {
                    addCubes.insert({center, true});
                }
            }
        }
    }
}

/**
 * TempName.freeMemory()
 * Frees all memory associated with Cubes (deletes them).
 */
void TempName::freeMemory() {
    // Delete Cubes in limbo.
    for(auto it = limbo.begin(); it != limbo.end(); ++it) {
        delete (*it);
    }

    // Delete Cubes in activeCubes.
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        delete it->second;
    }
}

/**
 * TempName.handleInput()
 * Handles events triggered by user input.
 */
void TempName::handleInput() {
    // TempName state changes use keys 1, 2, 3.
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
            // Advance the TempName one step.
            stepCounter = frames_per_draw;
        }
    }



    // Check for a reset command.
    if(io.pressed(GLFW_KEY_R)) {
        reset();
    }
}

/**
 * TempName.init()
 * Initializes the TempName.
 * @param cubeVAO_: Pointer to the main cube VAO.
 * @param program_: Pointer to the ID of the TempName's shader program.
 * @param MVP_: Pointer to the Model-View-Projection matrix.
 * @param scale_: Spatial scale of the TempName's Cubes.
 * @param frames_per_draw_: Number of frames that the TempName's update cycle takes.
 * @param maxCubes_: Maximum number of simultaneously active Cubes.
 * @param bound_: Cubes are constrained to lie in the box [-bound_, bound_]^3.
 */
void TempName::init(
        GLuint *cubeVAO_,
        GLuint *program_,
        float scale_,
        int frames_per_draw_,
        int initNumCubes_
    ) {

    cubeVAO = cubeVAO_;

    program = program_;

    scale = scale_;
    scale2 = 2 * scale;

    frames_per_draw = frames_per_draw_;

    initNumCubes = initNumCubes_;

    state = stop;

    initGL();

    reset();
}

/**
 * TempName.initGL()
 * Initializes the OpenGL elements associated with the TempName.
 */
void TempName::initGL() {
    // Set up the translation and scale data arrays.
    translations.reserve((unsigned long)initNumCubes / 64);
    scales.reserve((unsigned long)initNumCubes / 64);

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

    // Create the type VBO.
    glGenBuffers(1, &typeVBO);
    glBindBuffer(GL_ARRAY_BUFFER, typeVBO);
    glVertexAttribIPointer(
            4,
            2,
            GL_INT,
            0,
            (void*)0
    );
    glVertexAttribDivisor(4, (GLuint)1);
    glEnableVertexAttribArray(4);

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
 * TempName.remove()
 * Removes a Cube with center (key) from activeCubes and pushes it back
 * to limbo if it's dead.
 * @param key: Center coordinates of the Cube.
 */
void TempName::remove(glm::ivec3 key) {
    if(findIn(activeCubes, key)) {
        // Push dead Cubes back to limbo.
        Cube *c = activeCubes[key];
        if(!c->live) {
            limbo.push_back(c);
        }
        activeCubes.erase(key);

        nCubes--;
    }
}

/**
 * TempName.reset()
 * Resets the TempName.
 */
void TempName::reset() {
    // Clear all the containers.
    freeMemory();
    limbo.clear();
    activeCubes.clear();
    drawCubes.clear();
    addCubes.clear();
    removeCubes.clear();

    // Initialize fcount.
    fcount = 0;

    // Construct all the Cubes.
    for(int i = 0; i < initNumCubes; ++i) {
        limbo.push_back(new Cube());
    }

    // Number of active Cubes.
    nCubes = 0;
}

/**
 * TempName.setRule()
 * Sets the update rule for the Cube states.
 * @param stay_vals: A live Cube stays live if its live neighbor count is an element of this vector.
 * @param born_vals: A dead Cube becomes live if its live neighbor count is an element of this vector.
 */
void TempName::setRule(std::vector<int> stay_vals, std::vector<int> born_vals) {
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
 * TempName.update()
 * Updates the Cubes in activeCubes.
 * @param t: Current frame's count within [0, frames_per_draw).
 */
void TempName::update() {

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
 * TempName.updateActiveCubes()
 * Processes removeCubes and addCubes to update activeCubes.
 */
void TempName::updateActiveCubes() {
    // First, remove inactive Cubes from activeCubes.
    for (auto &center : removeCubes) {
        remove(center);
    }

    // Second, add newly-active Cubes to activeCubes.
    for (auto it = addCubes.begin(); it != addCubes.end(); ++it) {
        glm::ivec3 center = it->first;
        add(center.x, center.y, center.z);
    }

    // Clear addCubes and removeCubes.
    addCubes.clear();
    removeCubes.clear();
}

/**
 * TempName.updateNeighborCount()
 * Counts the number of live Cubes neighboring each Cube in activeCubes.
 */
void TempName::updateNeighborCount() {
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
                            auto key = glm::ivec3(X, Y, Z);
                            if(findIn(activeCubes, key)) {
                                activeCubes[key]->live_neighbors++;
                            }
                        }

                    }
                }
            }
        }
    }
}

/**
 * TempName.updateResetCount()
 * Resets the live_neighbors property to 0 for all Cubes in activeCubes.
 */
void TempName::updateResetCount() {
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        Cube *c = it->second;

        c->live_neighbors = 0;
    }
}

/**
 * TempName.updateState()
 * Updates the state of each Cube in activeCubes.
 */
void TempName::updateState() {
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
                removeCubes.push_back(glm::ivec3(c->x, c->y, c->z));
            }
        }
    }
}
// End TempName.cpp
