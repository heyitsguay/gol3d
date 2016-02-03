#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <sstream>
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
 * [Born live neighbor counts/Stay live neighbor counts/Use Brian's brain mode]
 B4/S9/F: Fractal growth of certain patterns, lots of gliders.
 B4/S3/F: Similar to S9/B4 but with a paired linear puffer.
 B4,9/S5,10/T: Fancy gliders and puffers and more. Slightly explosive.
 B4,9/S5,12/T: Similar to B4,9/S5,10/T, but less unstable so far.
 B4/S1,9/T: Several interesting puffers.
*/

// GOL rules.
int bornArr[] = {6};
int stayArr[] = {};
bool useBBIn = true;

// Half-width of one side of the initial cube of Cubes.
int hwidth = 5;

// http://stackoverflow.com/a/236803/5719731
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

void processInputs(int argc,
                   char **argv,
                   bool &valgrindTest,
                   std::vector<int> &born,
                   std::vector<int> &stay,
                   bool &useBB) {
    if(argc == 2) {
        valgrindTest = (atoi(argv[1]) > 0);

    } else {
        valgrindTest = false;
    }

    // Three inputs - you're probably specifying which rule to use.
    if(argc == 4) {
        // User specified rules, as strings.
        std::string bornStr = argv[1];
        std::string stayStr = argv[2];
        // Third argument determines whether to use Brian's brain mode.
        int useBBArg = atoi(argv[3]);
        useBB = (useBBArg > 0);


        // Split into numbers, with "," as a delimiter.
        std::vector<std::string> bornEls, stayEls;
        split(bornStr, ',', bornEls);
        split(stayStr, ',', stayEls);

        // Convert those string vectors into int vectors.
        for(auto &x : bornEls) {
            int xi = stoi(x);
            if(xi >= 0 && xi < 27) {
                born.push_back(xi);
            }
        }
        for(auto &x : stayEls) {
            int xi = stoi(x);
            if(xi >= 0 && xi < 27) {
                stay.push_back(xi);
            }
        }

    } else {
        // Use default args.
        int bornArrSize = sizeof(bornArr) / sizeof(int);
        int stayArrSize = sizeof(stayArr) / sizeof(int);

        born.insert(born.end(), &bornArr[0], &bornArr[bornArrSize]);
        stay.insert(stay.end(), &stayArr[0], &stayArr[stayArrSize]);
        useBB = useBBIn;
    }
}

int main(int argc, char **argv) {
    // Used for setting up the CA rules.
    std::vector<int> born, stay;
    bool useBB;

    // Use inputs to put the application into test mode for valgrind.
    bool valgrindTest;

    processInputs(argc, argv, valgrindTest, born, stay, useBB);

    // Create and initialize the Application.
    Application &app = Application::getInstance();
//    app.init(1, QUALITY_LAPTOP);
    app.init(1);

    // GOL3D setup.
    auto gol = CellularAutomaton();

    auto stupid = glm::ivec3(0, 0, 0);
    gol.init(glm::vec3(0, 0, 0), 0.5, 1000000);
    gol.setRule(born, stay, useBB);
    gol.cubeCube(hwidth, 0.1, stupid);
    
    app.world.objects.push_back(&gol);
    app.world.activate(app.world.objects[0]);

    // Main loop.
    if(valgrindTest) {
        int testCycle = 100;
        while(testCycle--) {
            app.update();
            app.draw();
        }
    } else {
        while (!glfwWindowShouldClose(app.window)) {
            app.update();
            app.draw();
        }
    }

    usleep(250000);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    app.terminate();

    return 0;
}