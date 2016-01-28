//
// Created by matt on 1/26/16.
//

#ifndef PLAYGROUND_IO_H
#define PLAYGROUND_IO_H
#pragma once

#include <GLFW/glfw3.h>

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

    void cursorPosCallback(GLFWwindow *window, double xpos, double ypos);

    void init(GLFWwindow *window);

    void keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods);

    void mouseButtonCallback(GLFWwindow *window, int button, int action, int mods);

    bool pressed(int key);

    bool released(int key);

    bool toggled(int key);
};

#endif //PLAYGROUND_IO_H
