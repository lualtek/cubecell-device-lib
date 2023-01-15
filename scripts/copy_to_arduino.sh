#!/bin/bash

# This script copies the main .cpp file and headers into the Arduino documents folder

# Ensure lualtek-cubecell dir inside Arduino libraries
mkdir -p ~/Documents/Arduino/libraries/lualtek-cubecell

cp src/LualtekCubecellLib.cpp ~/Documents/Arduino/libraries/lualtek-cubecell
cp src/LualtekCubecellLib.h ~/Documents/Arduino/libraries/lualtek-cubecell
cp library.json ~/Documents/Arduino/libraries/lualtek-cubecell
