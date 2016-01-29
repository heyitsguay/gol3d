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

#include "Framework.h"

/* Fun rule sets discovered so far:
S9/B4: Fractal growth of certain patterns, lots of gliders.
S3/B4: Similar to S9/B4 but with a paired linear puffer.
*/

// GOL rules.
int stay_arr[] = {9};
int born_arr[] = {4};

// Half-width of one side of the initial cube of Cubes.
int hwidth = 5;

int main() {
    // Create and initialize the Framework.
    Framework &frm = Framework::getInstance();
    frm.init();

    // GOL3D setup.
    // Stay rule values.
    std::vector<int> stay (stay_arr, stay_arr + sizeof(stay_arr) / sizeof(int));
    // Born rule values.
    std::vector<int> born (born_arr, born_arr + sizeof(born_arr) / sizeof(int));
    frm.world.setRule(stay, born);
    // Initialize to have some randomly-activated Cubes.
    frm.world.cubeCube(hwidth, 0.1);

    // Main loop.
    int thang = 10;
    while(thang--) {
//    while(!glfwWindowShouldClose(frm.window)) {
        frm.update();
        frm.draw();
    }

    usleep(250000);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    frm.freeGL();

    return 0;
}
#pragma clang diagnostic pop