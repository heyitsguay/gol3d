#include <cstdio>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <sstream>
#include <vector>

#define POSIX

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
#include "Rule.h"
#include "nlohmann/json.hpp"

#define USEGENERALIZED

using json = nlohmann::json;
namespace fs = std::filesystem;

const bool headlessMode = true;
const bool computeStats = false || headlessMode;
const bool readInput = true;
std::vector<std::vector<int>> cubeStateLog;
std::vector<int> activeCubeLog;
std::vector<int> timeStepLog;
int activeCubesInit = 0;
int activeCubesNow = 0;
int prevActiveCubes = -1;
int prevPrevActiveCubes = -1;
int prevPrevPrevActiveCubes;
float activeCubesNowSmooth = 0;
const float smoothingFactor = 0.5;
const float populationGrowthThreshold = 35;
const float populationDecayThreshold = 0.005;
const int maxTimeSteps = 3000;
const int logEveryT = 5;
const std::string filePrefix = "output/2025-04-12/";

// Default GOL rules.
int bornArr[] = {4, 10};
int stayArr[] = {12};
bool useBBIn = true;

// Default GOL rules in generalized form
//std::vector<std::vector<std::string>> defaultRules =
//        {{"C", "9", "-", "5,8,10", "-"}, {"-", "6,7,8,9,10,11", "C", "-", "-"}, {"A", "-", "-", "-", "-"}, {"-", "-", "-", "5,8", "C"}, {"A", "-", "-", "-", "-"}};
//        {{"C", "8,10", "-"}, {"-", "4,5,6,7,8,9", "C"}, {"A", "-", "-"}};
//        {{"C", "6", "5"}, {"C", "8", "9,13"}, {"C", "-", "9"}};
//        {{"C", "4,10", "-"}, {"-", "5,12", "C"}, {"A", "-", "-"}};
//        {{"C", "8,9"}, {"C", "6,7,8,9"}};
//        {{"C", "4", "-", "-", "6"}, {"-", "6,7,8,9,10", "C", "-", "-"}, {"-", "-", "-", "A", "-"}, {"-", "7,8", "3,4,5,6", "-", "C"}, {"A", "-", "-", "-", "-"}};
//        {{"C", "10", "-", "14", "-"}, {"-", "6,7,8,9,10", "C", "-", "-"}, {"-", "-", "-", "A", "-"}, {"-", "7,8", "3,4,5,6", "-", "C"}, {"A", "-", "-", "-", "-"}};
//        {{"C", "4", "5", "3,6", "-"}, {"1", "2", "4", "4,7,14", "C"}, {"1", "-", "6", "C", "5,16"}, {"4,8,16", "-", "-", "-", "C"}, {"C", "-", "-", "-", "5"}};
//        {{"C", "4", "5,6", "2,3,7", "8,9"}, {"1", "3,4,6", "5,9", "7,14", "C"}, {"1", "4,6,9", "8,10", "C", "5,16"}, {"-", "-", "-", "4,5,6,7,8", "C"}, {"C", "-", "-", "4,6", "12"}};

//std::set<int> defaultLiveStates = {1, 2};

const int n_dims = 3;
const int n_states = 5;
const double L_live = 3.25;
const double L_sparse = 1.15;
std::mt19937 rng(53392);
Rule rule = generateRule(n_dims, n_states, L_live, L_sparse, rng);
auto defaultRules = rule.table;
auto defaultLiveStates = rule.liveStates;

std::vector<float> defaultCubeCubeProbs = {0.15f};

// Half-width of one side of the initial cube of Cubes.
int hwidth = 10;

void saveStateData(const std::string& ruleString,
                   const std::string& saveFile,
                   const std::string& endStatus) {

    // Create the JSON object
    json outputJson;

    // Add ruleString
    outputJson["ruleString"] = ruleString;

    // Add endStatus
    outputJson["endStatus"] = endStatus;

    outputJson["maxSteps"] = 3000;

    // Add liveStates (convert set to array)
    outputJson["liveStates"] = json::array();
    for (const auto& state : defaultLiveStates) {
        outputJson["liveStates"].push_back(state);
    }

    // Create populationRecord
    outputJson["populationRecord"] = json::object();

    // Process each time step
    for (size_t i = 0; i < cubeStateLog.size(); ++i) {
        const auto& stateVector = cubeStateLog[i];
        const int timeStep = timeStepLog[i];

        // Calculate metrics
        int numActiveCubes = std::accumulate(stateVector.begin(), stateVector.end(), 0);

        int numLiveCubes = 0;
        for (const int& liveState : defaultLiveStates) {
            if (liveState < stateVector.size()) {
                numLiveCubes += stateVector[liveState];
            }
        }

        int numDyingCubes = 0;
        for (size_t j = 1; j < stateVector.size(); ++j) {
            if (defaultLiveStates.find((int)j) == defaultLiveStates.end()) {
                numDyingCubes += stateVector[j];
            }
        }

        int numNonDeadCubes = numLiveCubes + numDyingCubes;

        // Create the entry for this time step
        json timeStepEntry;
        timeStepEntry["stateCounts"] = stateVector;
        timeStepEntry["numActiveCubes"] = numActiveCubes;
        timeStepEntry["numLiveCubes"] = numLiveCubes;
        timeStepEntry["numDyingCubes"] = numDyingCubes;
        timeStepEntry["numNonDeadCubes"] = numNonDeadCubes;

        // Add to populationRecord using timeStep as key
        outputJson["populationRecord"][std::to_string(timeStep)] = timeStepEntry;
    }

    // Create directories if needed
    fs::path filePath(saveFile);
    if (filePath.has_parent_path()) {
        fs::create_directories(filePath.parent_path());
    }

    // Save to file
    std::ofstream outFile(saveFile);
    if (!outFile.is_open()) {
        throw std::runtime_error("Failed to open file for writing: " + saveFile);
    }

    outFile << outputJson.dump(2); // Pretty print with 2-space indentation
    outFile.close();
}

bool updateCubeStats(Application &app, const std::string &saveFile) {
    cubeStateLog.push_back(app.getCubeStateCounts());
    int numActiveCubes = app.getActiveCubes();
    activeCubeLog.push_back(numActiveCubes);
    timeStepLog.push_back(app.numSteps);

    prevPrevPrevActiveCubes = prevPrevActiveCubes;
    prevPrevActiveCubes = prevActiveCubes;
    prevActiveCubes = numActiveCubes;

    if (activeCubesInit == 0) {
        activeCubesInit = numActiveCubes;
        activeCubesNow = numActiveCubes;
    } else {
        activeCubesNow = numActiveCubes;
    }

    float populationRatio = (float)activeCubesNow / (float)activeCubesInit;
    std::string endStatus;
    bool explosion = populationRatio > populationGrowthThreshold;
    bool extinction = populationRatio < populationDecayThreshold;
    bool flatline = (prevPrevPrevActiveCubes == prevPrevActiveCubes) && (prevPrevActiveCubes == prevActiveCubes) && (prevActiveCubes == numActiveCubes);
    bool reachedEnd = app.numSteps >= maxTimeSteps;
    if (explosion) {
        endStatus = "explosion";
    } else if (extinction) {
        endStatus = "extinction";
    } else if (flatline) {
        endStatus = "flatline";
    } else {
        endStatus = "continue";
    }
    bool shouldStop = explosion || extinction || flatline || reachedEnd;
    if (shouldStop) {
        std::cout << "\n" << endStatus << "\n";
        std::string ruleString = app.getRuleString();
        saveStateData(ruleString, saveFile, endStatus);
    }
    return shouldStop;
}

/*
 * Convert a (typically rule-)string to a filename.
 */
std::string stringToJSONFilename(const std::string& prefix, const std::string& input) {
    std::string filename = prefix;
    for (char c: input) {
        if (std::isalnum(c) || c == '{' || c == '}' || c == ',' || c == '-') {
            filename += c;
        } else if (c == '/') {
            filename += '|';
        } else if (std::isspace(c)) {
            filename += ' ';
        }
        if (filename.length() >= 64) {
            filename += ".json";
            return filename;
        }
    }
    filename += ".json";

    return filename;
}

std::vector<std::string> &split(const std::string&, char, std::vector<std::string>&);
void processInputs(int, char**, bool&, std::vector<int>&, std::vector<int>&, bool&);
std::tuple<const Rule, const std::string> processGCAInputs(int, char**);

void ThreadSleep(unsigned long numMicroseconds) {
    /*
     * Sleep function. Thread calling this will sleep for the
     * specified number of milliseconds.
     *
     * Inputs:
     * numMilliseconds    Number of milliseconds to sleep
     */
    usleep(numMicroseconds);
}

int main(int argc, char **argv) {
    // Used for setting up the CA rules.
    bool valgrindTest = false;
    std::string saveFile;
#ifdef USEGENERALIZED

    if (readInput) {
        auto [aRule, aSaveFile] = processGCAInputs(argc, argv);
        defaultRules = aRule.table;
        defaultLiveStates = aRule.liveStates;
        saveFile = aSaveFile;
    }
#else
    std::vector<int> born, stay;
    bool useBB;

    // Use inputs to put the application into test mode for valgrind.
    processInputs(argc, argv, valgrindTest, born, stay, useBB);
#endif
    // Create and initialize the Application.
    Application &app = Application::getInstance();
//    app.init(1, QUALITY_LAPTOP);
    app.init(1, QUALITY_HIGH, 2, headlessMode, &defaultCubeCubeProbs);

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

    std::string ruleString = gol.ruleString;
    std::cout << ruleString << "\n";
    if (!readInput) {
        saveFile = stringToJSONFilename(filePrefix, ruleString);
    }
#else
    gol.setRule(born, stay, useBB);
    gol.cubeCube(hwidth, 0.1, origin);
#endif

    app.world.objects.push_back(&gol);
    app.world.activate(app.world.objects[0]);

#ifdef USEGENERALIZED
    activeCubesInit = app.getActiveCubes();
#endif

    if (headlessMode) {
        gol.state = run;
    }

    // If stat computations detect a blowup or dieoff, this bool will indicate it
    bool closeDueToStats = false;

    // Main loop.
    if(valgrindTest) {
        int testCycle = 100;
        while(testCycle--) {
            app.update();
            app.draw();
        }
    } else {
        bool close_condition = glfwWindowShouldClose(app.window);
        while (!close_condition) {
            close_condition = glfwWindowShouldClose(app.window);
            app.update();
            if (!app.headlessMode) {
                app.draw();
            }

#ifdef USEGENERALIZED

            if (computeStats && (app.numSteps % logEveryT == 1)) {
                closeDueToStats = updateCubeStats(app, saveFile);
            }
            close_condition = close_condition || closeDueToStats;

#endif

        }
    }

    // Sleep for a quarter second to stop some bug that was happening with GLFW destruction
    ThreadSleep(250 * 1000);

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

std::tuple<const Rule, const std::string> processGCAInputs(
        int argc,
        char **argv
) {
    const std::string jsonFile = argv[1];
    const std::string saveFile = argv[2];

    const Rule _rule = parseRuleFromJson(jsonFile);

    return std::make_tuple(_rule, saveFile);
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