//
// Created by mattguay on 10/9/15.
//
#include "load_obj.h"

#include <cstdio>
#include <string>
#include <cstring>

bool load_obj(
        const char *path,
        std::vector<glm::vec3> &out_vertices,
        std::vector<glm::vec3> &out_normals
) {
//    printf("Loading OBJ file %s...\n", path);

    std::vector<unsigned int> vertexIndices, normalIndices;
    std::vector<glm::vec3> temp_vertices;
    std::vector<glm::vec3> temp_normals;

    FILE *file = fopen(path, "r");
    if(file == NULL) {
        printf("Cannot open OBJ file.\n");
        getchar();
        return false;
    }

    while(true) {
        char lineHeader[128];
        // read the first word of the line.
        int res = fscanf(file, "%s", lineHeader);
        if(res == EOF){
            break;
        }

        if(strcmp(lineHeader, "v") == 0) {
            glm::vec3 vertex;
            fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
            temp_vertices.push_back(vertex);
//        } else if(strcmp(lineHeader, "vt") == 0) {
//            glm::vec2 uv;
//            fscanf(file, "%f %f\n", &uv.x, &uv.y);
//            uv.y = -uv.y;
//            temp_uvs.push_back(uv);
        } else if(strcmp(lineHeader, "vn") == 0) {
            glm::vec3 normal;
            fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
            temp_normals.push_back(normal);
        } else if(strcmp(lineHeader, "f") == 0) {
            unsigned int vertexIndex[3], normalIndex[3];
            int matches = fscanf(file, "%d//%d %d//%d %d//%d\n",
            &vertexIndex[0], &normalIndex[0],
            &vertexIndex[1], &normalIndex[1],
            &vertexIndex[2], &normalIndex[2]);

//            if(matches != 9){
//                printf("File can't be read by this simple parser.\n");
//                return false;
//            }
            vertexIndices.push_back(vertexIndex[0]);
            vertexIndices.push_back(vertexIndex[1]);
            vertexIndices.push_back(vertexIndex[2]);
            normalIndices.push_back(normalIndex[0]);
            normalIndices.push_back(normalIndex[1]);
            normalIndices.push_back(normalIndex[2]);
        } else { // Probably a comment, ignore.
            char commentBuffer[1000];
            fgets(commentBuffer, 1000, file);
        }

    }

    // For each vertex of each triangle...
    for(int i=0; i<vertexIndices.size(); i++) {
        // ...get the indices of its attributes.
        unsigned int vertexIndex = vertexIndices[i];
        unsigned int normalIndex = normalIndices[i];

        // ...get the attributes
        glm::vec3 vertex = temp_vertices[vertexIndex - 1];
        glm::vec3 normal = temp_normals[normalIndex - 1];

        // ...put the attributes in buffers.
        out_vertices.push_back(vertex);
        out_normals.push_back(normal);
    }

    return true;
}


