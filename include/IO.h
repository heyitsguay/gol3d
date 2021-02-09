//
// Created by matt on 1/26/16.
//

#ifndef GOL3D_IO_H
#define GOL3D_IO_H
#pragma once

#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

#define N_KEYS 1024

class IO {
private:
    IO(); // private constructor, allow only 1 instance.

    IO(IO const&); // prevent copies.
    void operator=(IO const&); // prevent assignments.

    static void baseCursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
        getInstance().cursorPosCallback(window, xpos, ypos);
    }

    static void baseKeyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
        getInstance().keyCallback(window, key, scancode, action, mods);
    }

    static void baseMouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
        getInstance().mouseButtonCallback(window, button, action, mods);
    }

public:
    static IO &getInstance() {
        static IO instance;
        return instance;
    }

    // Tracks which keys are currently pressed.
    bool pflag[N_KEYS];

    // Tracks which keys have been toggled from released to pressed.
    bool tflag[N_KEYS];

    // Mouse position (in pixels, from the top-left corner) during the
    // previous frame.
    double pmouseX = 0.;
    double pmouseY = 0.;

    // Mouse position during the current frame.
    double mouseX = 0.;
    double mouseY = 0.;

    // Difference in cursor position between this and previous frame.
    double dmouseX = 0.;
    double dmouseY = 0.;

    void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);

    void init(GLFWwindow *window);

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    bool pressed(int key);

    bool released(int key);

    bool toggled(int key);

    void update();
};

#endif //GOL3D_IO_H
