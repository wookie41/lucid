-- premake5.lua
workspace "lucid"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux"}
   include "engine"
   include "scene_editor"