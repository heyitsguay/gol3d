//
// Created by mattguay on 10/9/15.
//
#pragma once
#include <vector>
#include <glm/glm.hpp>

bool load_obj(
        const char *path,
        std::vector<glm::vec3> &out_vertices,
        std::vector<glm::vec3> &out_normals
);

bool load_textured_obj(
        const char *path,
        std::vector<glm::vec3> &out_vertices,
        std::vector<glm::vec3> &out_normals,
        std::vector<glm::vec2> &out_uvs
);