cmake_minimum_required(VERSION 3.23)
project(RenderFontGlyphs C)

set(CMAKE_C_STANDARD 99)

add_executable(RenderFontGlyphs src/main.c src/reshaper.c)
