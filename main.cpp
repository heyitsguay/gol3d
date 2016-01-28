#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <unistd.h>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>

//#define _DEBUG

#include "Camera.h"
#include "Skybox.h"
#include "User.h"

#include "load_obj.h"
#include "opengl-debug.h"

/* Fun rule sets discovered so far:
S9/B4: Fractal growth of certain patterns, lots of gliders.
S3/B4: Similar to S9/B4 but with a paired linear puffer.
*/

// GOL rules.
int stay_arr[] = {9};
int born_arr[] = {4};

// Spatial scale.
float scale = 0.5f;

// Maximum number of Cubes in the World.
const unsigned int maxCubes = 1000000;

// Half-width of one side of the initial cube of Cubes.
int hwidth = 5;

int main() {
    if( !glfwInit() ){
        fprintf(stderr, "Failed to initialize GLFW\n");
        return -1;
    }

    // Antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Get monitor info
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    GLFWmonitor* useMonitor = monitors[count-1];
    // Calculate monitor resolution.
    const GLFWvidmode *mode = glfwGetVideoMode(useMonitor);
    int window_width = mode->width;
    int window_height = mode->height;

    GLFWwindow* window;
    window = glfwCreateWindow(window_width, window_height, " ", useMonitor, NULL);
    if( window == NULL ) {
        fprintf(stderr, "Failed to open GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glewExperimental = (GLboolean)true;
    if( glewInit() != GLEW_OK) {
        fprintf(stderr, "Failed to initialize GLEW\n");
        return -1;
    }

    GL_CHECK( true );

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set initial clear color
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Read in a .obj file.
    std::vector<glm::vec3> g_vertices;
    std::vector<glm::vec3> g_normals;
    std::string path = "../obj/cube.obj";
    if(!load_obj(path.c_str(), g_vertices, g_normals)) {
        printf("Could not load %s\n", path.c_str());
    }

    // Vertex position attribute setup.
    // Create VAO for vertices.
    GLuint CubeVAO;
    glGenVertexArrays(1, &CubeVAO);
    glBindVertexArray(CubeVAO);
    // Create position VBO.
    // Array buffer for vertices.
    GLuint vertexbuffer;
    glGenBuffers(1, &vertexbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
    glBufferData(GL_ARRAY_BUFFER, g_vertices.size() * sizeof(glm::vec3), &g_vertices[0], GL_STATIC_DRAW);
    // Set up the position vertex attribute.
    glVertexAttribPointer(
            0, // attribute 0
            3, // number of vertices
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // array buffer offset
    );
    glEnableVertexAttribArray(0);

    // Create normal VBO.
    // Array buffer for normals.
    GLuint normalbuffer;
    glGenBuffers(1, &normalbuffer);
    glBindBuffer(GL_ARRAY_BUFFER, normalbuffer);
    glBufferData(GL_ARRAY_BUFFER, g_normals.size() * sizeof(glm::vec3), &g_normals[0], GL_STATIC_DRAW);
    // Set up the normal vertex attribute.
    glVertexAttribPointer(
            1, // attribute number.
            3, // size
            GL_FLOAT, // type
            GL_FALSE, // normalized?
            0, // stride
            (void*)0 // array buffer offset
    );
    glEnableVertexAttribArray(1);


    // Load shaders.
    GLuint SP_world = LoadShaders("../glsl/world.vert", "../glsl/world.frag");
    GLuint SP_cursor = LoadShaders("../glsl/cursor.vert", "../glsl/cursor.frag");
    GLuint SP_skybox = LoadShaders("../glsl/skybox.vert", "../glsl/skybox.frag");

    // Model matrix.
    glm::mat4 Model = glm::mat4(1.f);

    // MVP matrix.
    glm::mat4 MVP;

    // Uniform handles for SP_cursor.
    // Handle for the MVP uniform.
    GLuint uMVP_cursor = (GLuint) glGetUniformLocation(SP_cursor, "u_MVP");

    // Set the alpha blend function.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // Set the depth test function.
    glDepthFunc(GL_LEQUAL);

    // Frame rate stuff.
    double lastTime = glfwGetTime();
    int nbFrames = 0;

    // I/O handler setup.
    IO &io = IO::getInstance();
    io.init(window);

    // GOL3D setup.
    // Stay rule values.
    std::vector<int> stay (stay_arr, stay_arr + sizeof(stay_arr) / sizeof(int));
    // Born rule values.
    std::vector<int> born (born_arr, born_arr + sizeof(born_arr) / sizeof(int));
    // Frames between each update of the World's activeCubes.
    int frames_per_draw = 10;
    // The World.
    auto world = World();
    world.init(&CubeVAO, &SP_world, scale, frames_per_draw, maxCubes);
    world.setRule(stay, born);
    // Initialize to have some randomly-activated Cubes.
    world.cubeCube(hwidth, 0.1);

    // Camera setup
    Camera &cam = Camera::getInstance();
    cam.init();

    // User setup.
    glm::vec3 position0(0, 0, 80);
    float horizontalAngle0 = (float)PI;
    float verticalAngle0 = 0.f;
    auto user = User();
    user.init(&world, position0, horizontalAngle0, verticalAngle0);

    // Skybox setup. MUST come after Camera setup.
    const float skyscale = 10000.f;
    auto skybox = Skybox();
    skybox.init(&SP_skybox, &cam, skyscale, true);


    // MAIN LOOP ///////////////////////////////////////////////////////////////////////////////////////////////////////
    while(!glfwWindowShouldClose(window)) {

        // Frame rate stuff.
        double t = glfwGetTime();
        nbFrames++;
        if(t - lastTime >= 1.0) {
//            printf("%f ms/frame\n", 1000./double(nbFrames));
            nbFrames = 0;
            lastTime += 1.0;
        }

        // Simulated time.
//        t = (float)std::fmod(t+dt, 1000.);

        // Check for a program exit.
        if(io.pressed(GLFW_KEY_ESCAPE)) {
            glfwSetWindowShouldClose(window, GL_TRUE);
        }

        // Update the User.
        user.update(t);

        // Update the World.
        world.update();

        // Update the Camera.
        cam.update();

        // Can compute MVP and camera_pos uniforms.
        MVP = cam.VP;

        float dcube = 2.f * world.scale;
        glm::vec3 vscale(world.scale, world.scale, world.scale);


        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        world.draw((float)t);

        // Draw the skybox.
        skybox.draw();

        // If the World is in the 'edit' state, render the draw cursor.
        if(world.state == edit) {

            glEnable(GL_BLEND);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_CULL_FACE);
            glUseProgram(SP_cursor);

            auto translation = glm::vec3(user.drawCursor.x * dcube,
                                         user.drawCursor.y * dcube,
                                         user.drawCursor.z * dcube);

            glm::mat4 translatedModel = glm::scale(glm::translate(Model, translation), vscale);
            glm::mat4 MVP2 = cam.VP * translatedModel;

            glUniformMatrix4fv(uMVP_cursor, 1, GL_FALSE, &MVP2[0][0]);

            glDrawArrays(GL_TRIANGLES, 0, (GLsizei)g_vertices.size());
        }

        glfwSwapBuffers(window);

        glfwPollEvents();
    }

    usleep(250000);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);

    // Cleanup VBO and shader
    glDeleteBuffers(1, &vertexbuffer);
    glDeleteBuffers(1, &normalbuffer);
    glDeleteProgram(SP_world);
    glDeleteProgram(SP_cursor);
    glDeleteProgram(SP_skybox);
    glDeleteVertexArrays(1, &CubeVAO);

    return 0;
}