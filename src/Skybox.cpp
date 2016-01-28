//
// Created by matt on 1/26/16.
//

#include <glm/gtc/matrix_transform.hpp>
#include <SOIL/SOIL.h>

#include "Skybox.h"

std::vector<const GLchar*> lowNames, highNames;

Skybox::Skybox() : io(IO::getInstance()) {};

/**
 * Skybox.draw()
 * Draws the Skybox using OpenGL.
 */
void Skybox::draw() {
    // Check for input before drawing.
    handleInput();

    if(drawState == DRAW_BOX) {
        glClearColor(0.f, 0.f, 0.f, 1.f);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
        glDisable(GL_CULL_FACE);
        glDepthMask(GL_FALSE);
        glUseProgram(program);

        // Translated and scaled model matrix.
        glm::mat4 Mts = glm::scale(glm::translate(M, cam->position), scaleVec);
        glm::mat4 MVP = cam->Projection * cam->View * Mts;

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, &MVP[0][0]);

        glBindTexture(GL_TEXTURE_CUBE_MAP, tex);

        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(GL_TRUE);

    } else if(drawState == DRAW_BLACK) {
        glClearColor(0.f, 0.f, 0.f, 1.f);

    } else if(drawState == DRAW_WHITE) {
        glClearColor(1.f, 1.f, 1.f, 1.f);
    }
}

/**
 * Skybox.handleInput()
 * Handles user input.
 */
void Skybox::handleInput() {
    // Toggle Skybox drawing.
    if(io.toggled(GLFW_KEY_G)) {
        drawState = char((drawState + 1) % 3);
    }
}

/**
 * Skybox.init()
 * Initializes the Skybox.
 * @param program_: ID of the Skybox's shader program.
 * @param cam_: Pointer to the Camera object.
 * @param scale_: Spatial scale of the rendered Skybox.
 * @param useHD: If true, use the HD Skybox texture.
 */
void Skybox::init(GLuint program_, Camera *cam_, float scale_, bool useHD) {
    program = program_;
    cam = cam_;
    scale = scale_;
    scaleVec = glm::vec3(scale, scale, scale);
    drawState = DRAW_BOX;

    // Set up Skybox cube map.
    // Path to the low-def textures.
    lowNames.push_back("../img/box1/front.jpg");
    lowNames.push_back("../img/box1/back.jpg");
    lowNames.push_back("../img/box1/up.jpg");
    lowNames.push_back("../img/box1/down.jpg");
    lowNames.push_back("../img/box1/right.jpg");
    lowNames.push_back("../img/box1/left.jpg");

    highNames.push_back("../img/box2/left.png");
    highNames.push_back("../img/box2/right.png");
    highNames.push_back("../img/box2/up.png");
    highNames.push_back("../img/box2/down.png");
    highNames.push_back("../img/box2/front.png");
    highNames.push_back("../img/box2/back.png");

    if(useHD) {
        tex = loadCubemap(highNames);
    } else {
        tex = loadCubemap(lowNames);
    }

    uMVP = (GLuint) glGetUniformLocation(program, "u_MVP");
}

GLuint Skybox::loadCubemap(std::vector<const GLchar*> faces)
{
    GLuint textureID;
    glGenTextures(1, &textureID);
    glActiveTexture(GL_TEXTURE0);

    int width,height;
    unsigned char* image;

    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);
    for(GLuint i = 0; i < faces.size(); i++)
    {
        image = SOIL_load_image(faces[i], &width, &height, 0, SOIL_LOAD_RGB);
        glTexImage2D(
                GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0,
                GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, image
        );
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);

    return textureID;
}
