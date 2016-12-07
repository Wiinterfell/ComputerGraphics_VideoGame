# 3D OpenGL video game

Demo : coming soon

## Description

The purpose of this project is to create a 3D OpenGL game. You will be a T-Rex and you will have to jump to dodge obstacles. 
My idea was to create the [Google offline mode T-Rex game](http://www.trex-game.skipser.com/), in 3 dimensions.

## Compile the project

To compile this project, you should update the properties before.
It is necessary to update the dependencies (glew, freeglut, glm and assimp).
- Right click on the project name in the solution explorer
- Go in C/C++ -> General -> Additional Include Directories 
- Change the full path for the directories (they are in the GitHub project so you can find them easily)
- Do the same for Linker -> General -> Additional Library Directories 
- Do the same for Linker -> Input ->  Additional Dependencies 
Then you can begin to use this project !

## Load the models

The model files are present in the assets directory. You can either copy them in your working directory (Debug or Release) or add the full path in main.cpp file.
They should be loaded correctly once you have done that step.

## Commands

Coming soon

## Previous steps

To learn a bit about how to render a scene, you can go and see [this project](https://github.com/Wiinterfell/ComputerGraphics_VideoGameScene).