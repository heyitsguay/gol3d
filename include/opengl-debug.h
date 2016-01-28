//
// Created by mattguay on 1/26/16.
//

#ifndef PLAYGROUND_OPENGL_DEBUG_H
#define PLAYGROUND_OPENGL_DEBUG_H
#pragma once

void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
//        abort();
    }
}

#ifdef _DEBUG
#define GL_CHECK(stmt) do { \
            stmt; \
            CheckOpenGLError(#stmt, __FILE__, __LINE__); \
        } while (0)
#else
#define GL_CHECK(stmt) stmt
#endif

#endif //PLAYGROUND_OPENGL_DEBUG_H
