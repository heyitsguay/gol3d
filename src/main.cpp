#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#if defined(POSIX)
#include <unistd.h>
#endif

#if defined(_WIN32)
#include <Windows.h>
#endif

//#define GLEW_STATIC

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>


#include "Application.h"

#define USEGENERALIZED

bool headlessMode = false;
std::vector<int> activeCubeLog;
int activeCubesInit = 0;
int activeCubesNow = 0;
float activeCubesNowSmooth = 0;

// Default GOL rules.
int bornArr[] = {4, 10};
int stayArr[] = {12};
bool useBBIn = true;

// Default GOL rules in generalized form
std::vector<std::vector<std::string>> defaultRules =
//    {
//        {"C", "9", "-", "5,8,10", "-"},
//        {"-", "6,7,8,9,10,11", "C", "-", "-"},
//        {"A", "-", "-", "-", "-"},
//        {"-", "-", "-", "5,8", "C"},
//        {"A", "-", "-", "-", "-"}
//};
//        {{"C", "8,10", "-"}, {"-", "4,5,6,7,8,9", "C"}, {"A", "-", "-"}};
//        {{"C", "6", "5"}, {"C", "8", "9,13"}, {"C", "-", "9"}};
//        {{"C", "4,10", "-"}, {"-", "5,12", "C"}, {"A", "-", "-"}};
//        {{"C", "8,9"}, {"C", "6,7,8,9"}};
        {{"C", "4", "-", "-", "-"}, {"-", "6,7,8,9,10", "C", "-", "-"}, {"-", "-", "-", "A", "-"}, {"-", "7,8", "3,4,5,6", "-", "C"}, {"A", "-", "-", "-", "-"}};
std::set<int> defaultLiveStates = {1};
//std::vector<float> defaultCubeCubeProbs = {0.05f, 0, 0.05f, 0};
std::vector<float> defaultCubeCubeProbs = {0.15f};

// Half-width of one side of the initial cube of Cubes.
int hwidth = 17;

const int updatesPerFrame = 1000;

std::vector<std::string> &split(const std::string&, char, std::vector<std::string>&);
void processInputs(int, char**, bool&, std::vector<int>&, std::vector<int>&, bool&);

void ThreadSleep(unsigned long numMicroseconds) {
    /*
     * Cross-platform sleep function. Thread calling this will sleep for the
     * specified number of milliseconds.
     *
     * Inputs:
     * numMilliseconds    Number of milliseconds to sleep
     */
#if defined(_WIN32)
    Sleep(numMicroseconds);
#elif defined(POSIX)
    usleep(numMicroseconds);
#endif
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
    app.init(0, QUALITY_HIGH, 2, headlessMode, &defaultCubeCubeProbs);

    // GOL3D setup.
#ifdef USEGENERALIZED
    auto gol = GeneralizedCellularAutomaton();
#else
    auto gol = CellularAutomaton();
#endif



    auto origin = glm::ivec3(0, 0, 0);
    gol.init(glm::vec3(0, 0, 0), 0.5, 1000000);
#ifdef USEGENERALIZED
    gol.setRule(defaultRules, defaultLiveStates);
    gol.cubeCube(hwidth, defaultCubeCubeProbs, origin);
#else
    gol.setRule(born, stay, useBB);
    gol.cubeCube(hwidth, 0.1, origin);
#endif

    app.world.objects.push_back(&gol);
    app.world.activate(app.world.objects[0]);

    if (headlessMode) {
        gol.state = run;
    }

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
            if (!app.headlessMode) {
                app.draw();
            } else {
                int numActiveCubes = app.getActiveCubes();
                activeCubeLog.push_back(numActiveCubes);
                if (activeCubesInit == 0) {
                    activeCubesInit = numActiveCubes;
                    activeCubesNow = numActiveCubes;
                    activeCubesNowSmooth = numActiveCubes;
                } else {
                    activeCubesNow = numActiveCubes;
                    activeCubesNowSmooth =
                            0.5f * activeCubesNowSmooth + 0.5f * numActiveCubes;
                }

                if (activeCubesNowSmooth / activeCubesInit > 3) {
                    std::stringstream reportStringStream;
                    reportStringStream << gol.ruleString << ": Blowup in "
                        << app.numSteps << " steps.";
                    std::string reportString = reportStringStream.str();
                    std::cout << reportString << std::endl;

                    std::ofstream reportFile;
                    reportFile.open("report.txt", std::ofstream::out | std::ofstream::app);
                    reportFile << reportString << "\n";
                    reportFile.close();

                    // Sleep for a quarter second to stop some bug that was happening with GLFW destruction
                    ThreadSleep(250 * 1000);

                    // Close OpenGL window and terminate GLFW
                    glfwTerminate();

                    app.terminate();

                    return 0;

                }

            }

        }
    }

    // Sleep for a quarter second to stop some bug that was happening with GLFW destruction
    ThreadSleep(250 * 1000);

    // Close OpenGL window and terminate GLFW
    glfwTerminate();

    app.terminate();

    return 0;
}

// http://stackoverflow.com/a/236803/5719731
std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while (std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}

// TODO: Deal with this
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