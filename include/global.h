//
// Created by matt on 1/28/16.
//

#ifndef GOL3D_GLOBAL_H
#define GOL3D_GLOBAL_H
#pragma once

// Contains common definitions, macros, and functions used throughout the project.

const double PI = 3.1415926535897;

#define mod(m, n) fmod(fmod(m, n) + n, n)
#define clamp(n, lower, upper) if(n < lower) n = lower; else if(n > upper) n = upper

#endif //GOL3D_GLOBAL_H
