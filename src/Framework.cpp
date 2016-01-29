//
// Created by matt on 1/28/16.
//
#include "Framework.h"

#include <GL/glxew.h>

#include "load_obj.h"
#include "opengl-debug.h"

// Maximum number of Cubes that can be processed per frame.
const unsigned int maxCubes = 1000000;

// Cube spatial scale.
const float scale = 0.5f;

// World frames per update cycle.
const int framesPerCycle = 10;

Framework::Framework() :
        io(IO::getInstance()),
        cam(Camera::getInstance()) {}

/**
 * Framework.draw()
 * Draws all the parts of the application.
 */
void Framework::draw() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    world.draw((float)t);

    skybox.draw();

    user.draw();

    glfwSwapBuffers(window);

    glfwPollEvents();
}

/**
 * Framework.freeGL()
 * Frees allocated OpenGL resources.
 */
void Framework::freeGL() {
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDeleteBuffers(1, &vertexVBO);
    glDeleteBuffers(1, &normalVBO);
    glDeleteProgram(worldSP);
    glDeleteProgram(cursorSP);
    glDeleteProgram(skyboxSP);
    glDeleteVertexArrays(1, &cubeVAO);
}

/**
 * Framework.handleInput()
 * Handle user input to the Framework.
 */
void Framework::handleInput() {
    // Check for an application exit.
    if(io.pressed(GLFW_KEY_ESCAPE)) {
        glfwSetWindowShouldClose(window, GL_TRUE);
    }

    // Set the Framework up to print performance info.
    if(io.toggled(GLFW_KEY_B)) {
        printPerfInfo = true;
    }
}

/**
 * Framework.init()
 * Initializes the OpenGL resources and application objects.
 */
void Framework::init() {
    // Initialize OpenGL resources.
    initGL();

    // I/O handler setup.
    io.init(window);

    // World setup.
    world.init(&cubeVAO, &worldSP, scale, framesPerCycle, maxCubes);

    // Camera setup.
    cam.init();

    // User setup.
    glm::vec3 position0(0, 0, 80);
    float horizontalAngle0 = (float)PI;
    float verticalAngle0 = 0.f;
    user.init(&world, &cursorSP, position0, horizontalAngle0, verticalAngle0);

    // Skybox setup.
    const float skyscale = 10000.f;
    bool useHD = true;
    skybox.init(&skyboxSP, &cam, skyscale, useHD);

    // Get the time.
    t = glfwGetTime();
}

/**
 * Framework.initGL()
 * Initializes the OpenGL resources needed for the application.
 */
void Framework::initGL() {
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

    // Antialiasing.
    glfwWindowHint(GLFW_SAMPLES, 2);

    // Get monitor info. Use the secondary monitor if it's available.
    int count;
    GLFWmonitor** monitors = glfwGetMonitors(&count);
    GLFWmonitor* useMonitor = monitors[count-1];
    // Calculate monitor resolution.
    const GLFWvidmode *mode = glfwGetVideoMode(useMonitor);
    int window_width = mode->width;
    int window_height = mode->height;

    // Create the GLFW window, make its context current.
    window = glfwCreateWindow(window_width, window_height, " ", useMonitor, NULL);
    if( window == NULL ) {
        printf("Failed to open GLFW window");
        glfwTerminate();
        abort();
    }
    glfwMakeContextCurrent(window);

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
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    // Set up vsync.
    Display *dpy = glXGetCurrentDisplay();
    GLXDrawable drawable = glXGetCurrentDrawable();
    if(drawable) {
        glXSwapIntervalEXT(dpy, drawable, 1);
    }

    // Set initial clear color to black.
    glClearColor(0.f, 0.f, 0.f, 1.f);

    // Read in the cube.obj file.
    auto *path = (char*)"../obj/cube.obj";
    if(!load_obj(path, cubeVertices, cubeNormals)) {
        printf("Could not load %s\n", path);
        abort();
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

    // Load shaders.
    worldSP = LoadShaders("../glsl/world.vert", "../glsl/world.frag");
    cursorSP = LoadShaders("../glsl/cursor.vert", "../glsl/cursor.frag");
    skyboxSP = LoadShaders("../glsl/skybox.vert", "../glsl/skybox.frag");

    // Set the alpha blend function.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glBlendEquation(GL_FUNC_ADD);

    // Set the depth test function.
    glDepthFunc(GL_LEQUAL);
}

/**
 * Framework.perfInfo()
 * Calculates performance info (framerate, number of Cubes, etc.)
 * @param display: If true, prints the performance info.
 */
void Framework::perfInfo() {
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
        int numActiveCubes = (int)world.activeCubes.size();
        printf("%g ms/frame.\n %i active Cubes, %i Cubes drawn this frame.\n",
            frameRate, numActiveCubes, world.drawCount);

        printPerfInfo = false;
    }
}

/**
 * Framework.update()
 * Updates the application objects.
 */
void Framework::update() {
    // Update the time variable.
    t = glfwGetTime();

    // Handle user input.
    handleInput();

    user.update(t);

    world.update();

    cam.update();

    perfInfo();
}

