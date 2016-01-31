//
// Created by mattguay on 1/31/16.
//
#include "Object.h"

/**
 * Object()
 * Generic Object initialization.
 * @constructor
 */
Object::Object() : io(IO::getInstance()) {};

/**
 * ~Object()
 * Generic Object destructor.
 * @destructor
 */
Object::~Object() {
    freeMemory();
}

/**
 * Object.add()
 * Adds a Cube to activeCubes at (logical) location (x, y, z).
 * @param x: Cube x logical coordinate.
 * @param y: Cube y logical coordinate.
 * @param z: Cube z logical coordinate.
 */
void Object::add(int x, int y, int z) {
    auto center = glm::ivec3(x, y, z);

    // If it's not empty, add a Cube from limbo.
    if(!limbo.empty()) {
        // Add the Cube if it's not already in activeCubes.
        if(!findIn(activeCubes, center)) {
            Cube *c = limbo.back();
            limbo.pop_back();

            // Set the Cube up.
            c->setup(x, y, z);

            // Add the Cube to activeCubes.
            activeCubes.insert({center, c});

        } else {
            // Limbo is empty, create a new Cube.
            Cube *c = new Cube();
            c->setup(x, y, z);
            activeCubes.insert({center, c});
        }
    }
}

/**
 * Object.checkPoint()
 * Checks whether the input vector (in World coordinates) is contained within
 * a Cube in activeCubes.
 * @param point: Point to check.
 */
bool Object::checkPoint(glm::vec3 &point) {
    // Get the center of the Cube containing the point.
    glm::ivec3 center = centerFromPoint(point);
    return findIn(activeCubes, center);
}

/**
 * Object.findIn
 * Checks whether a Cube with logical center (center) is contained in the hashmap
 * (map).
 * @param map: The hashmap to check in.
 * @param center: The key to search for.
 */
template<typename T>
bool Object::findIn(std::unordered_map<glm::ivec3, T, KeyFuncs, KeyFuncs> &map, glm::ivec3 center) {
    return (map.find(center) != map.end());
}
template bool Object::findIn<Cube*>(cubeMap_t &map, glm::ivec3 center);
template bool Object::findIn<bool>(boolMap_t &map, glm::ivec3 center);

/**
 * Object.freeMemory
 * Frees memory allocated to Cubes.
 */
void Object::freeMemory() {
    // Delete Cubes in limbo.
    for(auto it = limbo.begin(); it != limbo.end(); ++it) {
        delete (*it);
    }
    limbo.clear();

    // Delete Cubes in activeCubes.
    for(auto it = activeCubes.begin(); it != activeCubes.end(); ++it) {
        delete it->second;
    }
    activeCubes.clear();

    // Clear the other *Cubes list objects, since they all held pointers to
    // now-deleted Cubes.
    drawCubes.clear();
    addCubes.clear();
    removeCubes.clear();
}

/**
 * Object.init()
 * Generic Object initializer.
 * @param origin_: Object spatial origin.
 * @param scale_: Object spatial scale.
 * @param initNumCubes_: Initial number of Cubes to allocate.
 */
void Object::init(glm::vec3 origin_, float scale_, int initNumCubes_) {
    origin = origin_;

    scale = scale_;
    scale2 = 2 * scale;

    initNumCubes = initNumCubes_;

    state = stop;

    cycle_stage = 0;

    reset();
}

/**
 * Object.remove()
 * Removes a Cube with center (center) from activeCubes, if it's there.
 * @param center: Center logical coordinate of the Cube to remove.
 */
void Object::remove(glm::ivec3 &center) {
    if(findIn(activeCubes, center)) {
        // Cube was found. Push it back to limbo.
        Cube *c = activeCubes[center];
        limbo.push_back(c);
        activeCubes.erase(center);
    }
}

/**
 * Object.reset()
 * Resets the Object.
 */
void Object::reset() {
    freeMemory();

    // Reset cycle_stage.
    cycle_stage = 0;

    // Construct new Cubes.
    for(int i = 0; i < initNumCubes; ++i) {
        limbo.push_back(new Cube());
    }
}
