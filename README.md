# game-1
Game 1

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
