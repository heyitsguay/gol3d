//
// Created by mattguay on 10/8/15.
//
#include "load_bmp.h"

#include <cstdio>
#include <cstdlib>
#include <string>

GLuint load_bmp(char **dest, const char *imagePath) {
    /*
     * Loads data from a BMP file into a character array
     *
     * Inputs:
     * dest         Pointer to the char array that should hold the BMP output.
     * imagePath    Name of the image file, relative to the pwd.
     */

    // Data read from the BMP header.
    unsigned char header[54]; // BMP header
    unsigned int dataPos; // Position in the file where the actual data begins.
    unsigned int width, height;
    unsigned int imageSize; // width * height * 3
    unsigned char *data; // Actual RGB data;

    // Open the file
    FILE *file = fopen(imagePath, "rb");
    if(!file) {
        printf("Image could not be opened.\n");
        return 0;
    }

    // Check that the header is right.
    if(fread(header, 1, 54, file)!=54) {
        printf("Not a correct BMP file.\n");
        return 0;
    }
    if(header[0] != 'B' || header[1] != 'M'){
        printf("Not a correct BMP file.\n");
        return 0;
    }

    // Read ints from the byte array.
    dataPos = *(unsigned int*)&(header[0x0A]);
    imageSize = *(unsigned int*)&(header[0x22]);
    width = *(unsigned int*)&(header[0x12]);
    height = *(unsigned int*)&(header[0x16]);

    // Some BMP files are misformatted, guess missing information.
    if(imageSize == 0) imageSize = width * height * 3;
    if(dataPos == 0) dataPos = 54;

    // Create a buffer for the data.
    data = new unsigned char[imageSize];

    // Read the data from the file.
    fread(data, 1, imageSize, file);

    fclose(file);

    // Create an OpenGL texture.
    GLuint textureID;
    glGenTextures(1, &textureID);

    // Bind the new texture.
    glBindTexture(GL_TEXTURE_2D, textureID);

    // Give the image to OpenGL.
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, data);

    // OpenGL has the data, so free the CPU version.
    delete [] data;

    // Filtering
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glGenerateMipmap(GL_TEXTURE_2D);

    return textureID;

}
