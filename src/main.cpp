#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <vector>

#include <unistd.h>

#pragma clang diagnostic push
#pragma ide diagnostic ignored "UnusedImportStatement"
#include <GL/glew.h>
#pragma clang diagnostic pop
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#include "Application.h"

/* Fun rule sets discovered so far:
S9/B4: Fractal growth of certain patterns, lots of gliders.
S3/B4: Similar to S9/B4 but with a paired linear puffer.
*/

// GOL rules.
int born_arr[] = {3};
int stay_arr[] = {};

// Half-width of one side of the initial cube of Cubes.
int hwidth = 5;

int main() {
    // Create and initialize the Application.
    Application &app = Application::getInstance();
//    app.init(1, QUALITY_LAPTOP);
    app.init(1);

    // GOL3D setup.
    // Stay rule values.
    std::vector<int> stay (stay_arr, stay_arr + sizeof(stay_arr) / sizeof(int));
    // Born rule values.
    std::vector<int> born (born_arr, born_arr + sizeof(born_arr) / sizeof(int));

    auto gol = CellularAutomaton();

    auto stupid = glm::ivec3(0, 0, 0);
    gol.init(glm::vec3(0, 0, 0), 0.5, 1000000);
    gol.setRule(born, stay, true);
    gol.cubeCube(hwidth, 0.1, stupid);
    app.world.objects.push_back(&gol);
    app.world.activate(app.world.objects[0]);

//    app.world.setRule(stay, born);
//    // Initialize to have some randomly-activated Cubes.
//    app.world.cubeCube(hwidth, 0.1);
//    app.world.state = run;

    // Main loop.
    while(!glfwWindowShouldClose(app.window)) {
        app.update();
        app.draw();
    }

    usleep(250000);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    app.terminate();

    return 0;
}