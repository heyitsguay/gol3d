//
// Created by matt on 1/31/16.
//
#include "World.h"

#include <SOIL/SOIL.h>

#include "global.h"
#include "opengl-debug.h"

World::World() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {}

World::~World() {
    glDisableVertexAttribArray(2);
    glDisableVertexAttribArray(3);
    glDisableVertexAttribArray(4);

    glDeleteBuffers(1, &translationVBO);
    glDeleteBuffers(1, &scaleVBO);
    glDeleteBuffers(1, &typeVBO);
}

/**
 * World.activate()
 * Sets the input Object as the current active Object in the World.
 * @param obj: The Object to set as active.
 */
void World::activate(Object *obj) {
    activeObject = obj;
    obj->active = true;
}

/**
 * World.draw()
 * Draws all the non-dead Cubes in all the Objects in the World.
 * @param t: Current simulation time.
 */
void World::draw(float t) {
    // Tracks the number of Cubes to draw.
    drawCount = 0;

    // TODO: Make more extensible
    // Texture to apply to dying cubes.
    const glm::ivec2 state2Tex = typeBase[T_GRAD];
    const glm::ivec2 state3Tex = typeBase[T_BORDERDYING];
    const glm::ivec2 state4Tex = typeBase[T_GRADDYING];

    const float camDist2 = cam.drawDistance * cam.drawDistance;

    // Clear the Cube data arrays.
    translations.clear();
    scales.clear();
    types.clear();

    // Iterate through the Objects in objects.
    for(auto &obj : objects) {
        for(auto it = obj->drawCubes.begin(); it != obj->drawCubes.end(); ++it) {
            Cube *c = it->second;
            auto translation = obj->origin + glm::vec3(c->center) * obj->scale2;
            glm::vec3 vecToCamera = translation - cam.position;
#pragma clang diagnostic push
#pragma ide diagnostic ignored "IncompatibleTypes"
            float d2ToCamera = glm::dot(vecToCamera, vecToCamera);
#pragma clang diagnostic pop

            // Draw *c if it's close enough to the Camera.
            if(d2ToCamera < camDist2) {
                translations.push_back(translation);
                scales.push_back(obj->scale);
                // Different texture for dying Cubes.
                if(c->state == 2) {
                    types.push_back(state2Tex);
                } else if (c->state == 3) {
                    types.push_back(state3Tex);
                } else if (c->state == 4) {
                    types.push_back(state4Tex);
                } else {
                    types.push_back(c->texBase);
                }

                drawCount++;
            }
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
        glUniform1f(uhBase, baseCubeH);
        glUniform1f(uvaryColor, varyColor);
        glUniform3fv(ucameraPos, 1, &cam.position[0]);
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

        GL_CHECK(glDrawArraysInstanced(GL_TRIANGLES, 0, 36, (GLsizei) drawCount));
    }
}

/**
 * World.handleInput()
 * Handles all user input to the World.
 */
void World::handleInput() {
    // Hue change increment.
    float dH = 0.01;
    // Increase Cube base hue.
    if(io.pressed(GLFW_KEY_EQUAL)) {
        baseCubeH = (float)pmod(baseCubeH + dH, 1.f);

    // Decrease Cube base hue.
    } else if(io.pressed(GLFW_KEY_MINUS)) {
        baseCubeH = (float)pmod(baseCubeH - dH, 1.f);
    }

    // Toggle flat shading.
    if(io.toggled(GLFW_KEY_N)) {
        varyColor = 1.f - varyColor;
    }
}

/**
 * World.init()
 * Initializes the World.
 * @param cubeVAO_: Pointer to the main Cube VAO.
 * @param program_: Pointer to the World's shader program.
 */
void World::init(GLuint *cubeVAO_, GLuint *program_) {
    cubeVAO = cubeVAO_;
    program = program_;

    initGL();
}

/**
 * World.initGL()
 * Initializes the OpenGL resources needed by the World.
 */
void World::initGL() {
    // Get GLSL uniform pointers.
    uMVP = (GLuint) glGetUniformLocation(*program, "u_MVP");
    uvaryColor = (GLuint) glGetUniformLocation(*program, "u_vary_color");
    uhBase = (GLuint) glGetUniformLocation(*program, "u_h_base");
    ucameraPos = (GLuint) glGetUniformLocation(*program, "u_camera_pos");
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
    unsigned char *image = SOIL_load_image("data/img/cubeatlas.png",
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
 * World.update()
 * Updates all the Objects in the World.
 */
void World::update() {
    handleInput();

    for(auto &obj : objects) {
        obj->update();
    }
}