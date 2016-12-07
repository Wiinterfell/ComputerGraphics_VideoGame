# 3D OpenGL T-REX video game

Demo : coming soon

## Description

The purpose of this project is to create a 3D OpenGL game. You will be a T-Rex and you will have to jump to dodge obstacles (cactus and enemies). 
My idea was to create the [Google offline mode T-Rex game](http://www.trex-game.skipser.com/), in 3 dimensions.

## Compile the project

To compile this project, you should update the properties before.
It is necessary to update the dependencies (glew, freeglut, glm, assimp and soil).
- Right click on the project name in the solution explorer
- Go in C/C++ -> General -> Additional Include Directories 
- Change the full path for the directories (they are in the GitHub project so you can find them easily)
- Do the same for Linker -> General -> Additional Library Directories 
- Do the same for Linker -> Input ->  Additional Dependencies 
Then you can begin to use this project !

Note: if you have any problem adding the soil library and it does not compile, you should recompile the project available in the soil folder, with you own version of Visual Studio.

## Load the models

The model files are present in the assets directory. You can either copy them in your working directory (Debug or Release) or add the full path in main.cpp file.
They should be loaded correctly once you have done that step.
Do not forget to also add the sounds assets.

## Commands

you are a T-Rex in the desert and your purpose is to go as far as you can!
Space: jump.
C: go left.
V: go right.
If you lose, just tap "space" to begin a new game.

## Previous steps

To learn a bit about how to render a scene, you can go and see [this project](https://github.com/Wiinterfell/ComputerGraphics_VideoGameScene).

## Release

You can play by downloading the .exe file on the release part of this repository!
Have fun ;)