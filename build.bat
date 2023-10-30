@echo off

g++ main.cpp glad.c -Llib -Iinclude -lglfw3 -lopengl32 -luser32 -lgdi32 -lshell32 -o build\main.exe
cd build
.\main
cd ..