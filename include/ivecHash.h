//
// Created by matt on 1/30/16.
//

#ifndef GOL3D_IVECHASH_H
#define GOL3D_IVECHASH_H
#pragma once

#include <cstddef>
#include <functional>

#include <glm/glm.hpp>

inline void hash_combine(size_t &seed, size_t hash) {
    hash += 0x9e3779b9 + (seed << 6) + (seed >> 2);
    seed ^= hash;
}

struct KeyFuncs {
    size_t operator()(const glm::ivec3 &k)const {
        size_t seed = 0;
        hash_combine(seed, std::hash<int>()(k.x));
        hash_combine(seed, std::hash<int>()(k.y));
        hash_combine(seed, std::hash<int>()(k.z));
        return seed;
    }

    bool operator()(const glm::ivec3 &a, const glm::ivec3 &b)const {
        return a.x == b.x && a.y == b.y && a.z == b.z;
    }
};



#endif //GOL3D_IVECHASH_H
