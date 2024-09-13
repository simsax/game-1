# game-1
Cube puzzle game prototype

## Edit (6 months later)

While revisiting old projects, I found this prototype of a puzzle game that I wanted to develop. 
However, I eventually lost motivation and moved on to other things, since it ended up being way 
less interesting than I initially thought.
I'm making this public because why not.
Below a video of what it currently looks like, I only implemented the editor and basic
movement mechanics.

## How to build

The project expects GLAD to be present under `extern/glad`.
The GLAD files are generated from the [web interface](https://glad.dav1d.de/).
The filetree should look like this:

```
extern/
    glad/
        include/
        src/
        CMakeLists.txt
    
src/
    src files
CMakeLists.txt
```

This is the content of the CMakeLists.txt inside `extern/glad`:

```
cmake_minimum_required(VERSION 3.20)
project(glad)
add_library(glad include/glad/glad.h src/glad.c)
target_include_directories(glad PUBLIC include)
```

After including the GLAD files, create a `build` directory and then `./run.sh`.
