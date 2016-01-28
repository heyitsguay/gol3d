//
// Created by matt on 1/26/16.
//

#include <cstdio>

#include "IO.h"

IO::IO() {};

IO::IO(IO const&) {};

void IO::operator=(IO const&) {};

/**
 * IO.cursorPosCallback()
 * The callback function for cursor motion events in GLFW.
 * @param window: Pointer to the window that the callback is associated with.
 * @param xpos: Cursor x position, relative to the window's left edge.
 * @param ypos: Cursor y position, relative to the window's top edge.
 */
void IO::cursorPosCallback(GLFWwindow *window, double xpos, double ypos) {
    // Do nothing for now.
}

/**
 * IO.init()
 * Initializes the IO object, sets the needed GLFW callback functions.
 * @param window: Pointer to the window that the IO object interacts with.
 */
void IO::init(GLFWwindow *window) {
    // Initialize the flag arrays.
    for(int i = 0; i < N_KEYS; ++i) {
        pflag[i] = false;
        tflag[i] = true;
    }

    glfwSetKeyCallback(window, baseKeyCallback);
}

/**
 * IO.keyCallback()
 * The callback function for key events in GLFW.
 * @param window: Pointer to the window that the callback is associated with.
 * @param key: Key identifier.
 * @param scancode: Platform-specific key scancode.
 * @param action: Indicates whether the key was pressed or released.
 * @param mods: Indicates whether modifier keys (Shift, Ctrl, Alt) were held down.
 */
void IO::keyCallback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // The key is pressed.
    if(action == GLFW_PRESS) {
        pflag[key] = true;
    }

    // The key is released.
    if(action == GLFW_RELEASE) {
        pflag[key] = false;
        // Reset the toggled value for the key.
        tflag[key] = true;
    }
}

/**
 * IO.mouseButtonCallback()
 * The callback function for mouse button events in GLFW.
 * @param window: Pointer to the window that the callback is associated with.
 * @param button: Mouse button identifier.
 * @param action: Indicates whether the button was pressed or released.
 * @param mods: Indicates whether modifier keys (Shift, Ctrl, Alt) were held down.
 */
void IO::mouseButtonCallback(GLFWwindow *window, int button, int action, int mods) {
    // Do nothing for now.
}

/**
 * IO.pressed()
 * Checks whether (key) is pressed.
 * @param key: Key value to check.
 */
bool IO::pressed(int key) {
    return pflag[key];
}

/**
 * IO.released()
 * Checks whether (key) is released.
 * @param key: Key value to check.
 */
bool IO::released(int key) {
    return !pflag[key];
}

/**
 * IO.toggled()
 * Checks whether (key) has been toggled - i.e. state just changed from
 * released to pressed. Handles the necessary logic for that situation.
 * @param key: Key value to check.
 */
bool IO::toggled(int key) {
    bool t = pflag[key] && tflag[key];
    if(t) {
        tflag[key] = false;
    }
    return t;
}
