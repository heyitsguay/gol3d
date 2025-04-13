//
// Created by matt on 1/28/16.
//
#include "Application.h"

#include <iostream>

#if defined(POSIX)
#include <GL/glxew.h>
#elif defined(_WIN32)
#include <GL/wglew.h>
#endif

#include "load_obj.h"
#include "opengl-debug.h"

const double frameRate = 1./60;

Application::Application() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {}

/**
 * Application.draw()
 * Draws all the components of the application.
 */
void Application::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world.draw((float)t);

    skybox.draw(getActiveCubes());

    user.draw();

    glfwSwapBuffers(window);

    glfwPollEvents();
}

/**
 * Application.freeGL()
 * Frees allocated OpenGL resources.
 */
void Application::freeGL() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(5);
    glDeleteBuffers(1, &vertexVBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteProgram(worldSP);
    glDeleteProgram(cursorSP);
    glDeleteProgram(skyboxSP);
    glDeleteVertexArrays(1, &cubeVAO);
}


int Application::getActiveCubes() const {
    return (int)world.activeObject->activeCubes.size();
}


std::vector<int> Application::getCubeStateCounts() const {
    auto gca = dynamic_cast<GeneralizedCellularAutomaton*>(world.activeObject);
    return gca->stateCounts;
}


std::string Application::getRuleString() const {
    auto gca = dynamic_cast<GeneralizedCellularAutomaton*>(world.activeObject);
    return gca->ruleString;
}


/**
 * Application.handleInput()
 * Handle user input to the Application.
 */
void Application::handleInput() {
    // Check for an application exit.
    if(io.pressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Set the Application up to print performance info.
    if(io.toggled(GLFW_KEY_B)) {
        printPerfInfo = true;
    }
}

/**
 * Application.init()
 * Initializes the OpenGL resources and application objects.
 * @param monitorID: Indicates which monitor to use.
 * @param quality: Indicates the graphics quality.
 * @param aaSamples: Number of samples to use for multisampling (antialiasing).
 */
void Application::init(
        int monitorID,
        int quality,
        int aaSamples,
        bool headlessMode_,
        std::vector<float> *cubeCubeProbs) {
    headlessMode = headlessMode_;

    // Initialize OpenGL resources.
    initGL(monitorID, quality, aaSamples);

    // I/O handler setup.
    io.init(window);

    // World setup.
    world.init(&cubeVAO, &worldSP);

    // Camera setup.
    cam.init();

    // User setup.
    glm::vec3 position0(0, 0, 80);
    auto horizontalAngle0 = (float)PI;
    float verticalAngle0 = 0.f;
    user.init(&world, &cursorSP, position0, horizontalAngle0, verticalAngle0, cubeCubeProbs);

    // Skybox setup.
    const float skyscale = 10000.f;
    bool useHD = true;
    skybox.init(&skyboxSP, &cam, skyscale, useHD);

    // Get the time.
    tPrev = glfwGetTime();
    t = glfwGetTime();
    numSteps = 0;
}

/**
 * Application.initGL()
 * Initializes the OpenGL resources needed for the application.
 * Parameters are the same as in Application.init().
 */
void Application::initGL(int monitorID, int quality, int aaSamples) {
    // Initialize GLFW.
    if(!glfwInit()) {
        printf("Failed to initialize GLFW\n");
        abort();
    }

    // GLFW window setup. Require OpenGL 3.3, core profile.
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    // Antialiasing. Only allow 2x, 4x, 8x multisampling - all other
    // values result in multisampling being disabled.
    if(aaSamples == 2 || aaSamples == 4 || aaSamples == 8) {
        glfwWindowHint(GLFW_SAMPLES, aaSamples);
    } else {
        glfwWindowHint(GLFW_SAMPLES, 0);
    }

    // Get monitor info. Use the monitor indicated by monitorID if available,
    // otherwise use the primary monitor.
    int count;
    GLFWmonitor **monitors = glfwGetMonitors(&count);
    GLFWmonitor *useMonitor;
    if(monitorID < count) {
        useMonitor = monitors[monitorID];
    } else {
        useMonitor = monitors[0];
    }

    // Calculate logical window size. If input (quality) is a valid value
    // (see the top of Application.h for value aliases), adjust the resolution
    // accordingly, else just use the monitor resolution.
    const GLFWvidmode *mode = glfwGetVideoMode(useMonitor);
    // Monitor resolution.
    int xResolution = mode->width;
    int yResolution = mode->height;

    int window_width, window_height;

    if (headlessMode) {
        window_width = 1;
        window_height = 1;
        useMonitor = nullptr;
        glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    } else if(quality == QUALITY_LAPTOP) {
        // Always 1366x768, a personal indulgence.
        window_width = 1366;
        window_height = 768;
    } else {
        // 100% scale. Ideally, you selected QUALITY_BEST to get here.
        window_width = xResolution;
        window_height = yResolution;
    }

    // Create the GLFW window, make its context current.
    window = glfwCreateWindow(window_width, window_height, " ", useMonitor, nullptr);
    if( window == nullptr ) {
        printf("Failed to open GLFW window");
        glfwTerminate();
        abort();
    }
    glfwMakeContextCurrent(window);
//    glfwHideWindow(window);

    // Initialize GLEW.
    glewExperimental = (GLboolean)true;
    if( glewInit() != GLEW_OK) {
        printf("Failed to initialize GLEW\n");
        abort();
    }

    // Used for OpenGL debugging. Clears a spurious OpenGL error that glewInit()
    // might cause.
    GL_CHECK( true );

    // Disable the cursor.
//    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set up vsync
    glfwSwapInterval(1);
//    if(GLXEW_EXT_swap_control) {
//        Display *dpy = glXGetCurrentDisplay();
//        GLXDrawable drawable = glXGetCurrentDrawable();
//        if (drawable) {
//            glXSwapIntervalEXT(dpy, drawable, 1);
//        }
//    }

    // Set initial clear color to black.
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Read in the cube.obj file.
    auto *path = (char*)"data/obj/cubeuv2.obj";
    if(!load_textured_obj(path, cubeVertices, cubeNormals, cubeUVs)) {
        printf("Could not load %s\n", path);
        abort();
    }

    // Renormalize UV coordinates.
    // Scaling factor
    float fac = float(cubeTexSize) / (2 * float(texAtlasSize));
    // Scaling vector.
    glm::vec2 uvRescale(fac);
    // Scale
    for(auto &uv : cubeUVs) {
        uv = uv * uvRescale + uvRescale;
    }

    // Set up the cube VAO.
    glGenVertexArrays(1, &cubeVAO);
    glBindVertexArray(cubeVAO);

    // Set up the cube vertex position VBO.
    glGenBuffers(1, &vertexVBO);
    glBindBuffer(GL_ARRAY_BUFFER, vertexVBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), &cubeVertices[0], GL_STATIC_DRAW);
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

    // Set up the cube vertex normal VBO.
    glGenBuffers(1, &normalVBO);
    glBindBuffer(GL_ARRAY_BUFFER, normalVBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec3), &cubeNormals[0], GL_STATIC_DRAW);
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

    // Set up the cube UV coordinate VBO.
    glGenBuffers(1, &uvVBO);
    glBindBuffer(GL_ARRAY_BUFFER, uvVBO);
    glBufferData(GL_ARRAY_BUFFER, 36 * sizeof(glm::vec2), &cubeUVs[0], GL_STATIC_DRAW);
    glVertexAttribPointer(
            5,
            2,
            GL_FLOAT,
            GL_FALSE,
            0,
            (void*)0
    );
    glEnableVertexAttribArray(5);

    // Load shaders.
    worldSP = LoadShaders("glsl/world.vert", "glsl/world.frag");
    cursorSP = LoadShaders("glsl/cursor.vert", "glsl/cursor.frag");
    skyboxSP = LoadShaders("glsl/skybox.vert", "glsl/skybox.frag");

    // Set the alpha blend function.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // Set the depth test function.
    glDepthFunc(GL_LEQUAL);

}

/**
 * Application.perfInfo()
 * Calculates performance info (framerate, number of Cubes, etc.)
 */
void Application::perfInfo() {
    static double lastTime = 0;
    static int numFrames = 0;
    static float frameRate = 0;

    numFrames++;
    if(t - lastTime >= 1.0) {
        frameRate = float(1000. / numFrames);
        numFrames = 0;
        lastTime += 1.0;
    }

    if(printPerfInfo) {
        int numActiveCubes = (int)world.activeObject->activeCubes.size();
        printf("%g ms/frame.\n %i active Cubes, %i Cubes drawn this frame.\n",
            frameRate, numActiveCubes, world.drawCount);

        printPerfInfo = false;
    }
}

/**
 * Application.terminate()
 * Runs the application shutdown processes.
 */
void Application::terminate() {
    // Just frees OpenGL object memory for now.
    freeGL();
}

/**
 * Application.update()
 * Updates the application objects.
 */
void Application::update() {
    // Update the time variable.
//    double tNew = glfwGetTime();
    t = glfwGetTime();
    numSteps++;

    // Handle user input.
    handleInput();

    user.update(t);

    if (t - tPrev > 0){//frameRate) {
        world.update();
        tPrev = glfwGetTime();
    }

    cam.update();

    perfInfo();
}

