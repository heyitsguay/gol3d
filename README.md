# Game of Life 3D
Currently tested only on 64-bit Ubuntu 15.10. Build using CMake, run from the terminal. Requires OpenGL 3.3, GLEW, GLFW 3, SOIL, and pkg-config.

Check out patterns.txt for some examples of interesting rule sets.

## Example
Run an instance using the B4,9/S1,10 rule set with Brian's brain mode active:
`./gol3d 4,9 1,10 1`

Run an instance using the B4/S3 rule set without Brian's brain mode active:
`./gol3d 4 3 0`
