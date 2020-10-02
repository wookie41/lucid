-- premake5.lua
workspace "lucid"
   configurations { "Debug", "Release" }
   
   platforms { "Shared64" }
   filter "platforms:Static64"
      kind "StaticLib"
      architecture "x64"

   filter "platforms:Shared64"
      kind "SharedLib"
      architecture "x64"

project "lucid"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"

   includedirs { "." }

   includedirs { 
      "platform/include", 
      "devices/include", 
      "common/include", 
      "graphics/include", 
      "canvas/include",
      "scene/include" 
   }

   includedirs { 
      "libs/glm", 
      "libs/stb", 
      "libs/glew/include", 
      "libs/SDL2/include" 
   }
   
   libdirs { 
      "libs/glew/lib/x64", 
      "libs/SDL2/lib/x64" 
   }
   
   links { 
      "glew32", 
      "opengl32", 
      "SDL2", 
      "SDL2main"
   }

   files { 
      "main.cpp", 
      "devices/src/gpu/**.cpp",
      "devices/src/gpu/gl/**.cpp",
      "platform/src/sdl/**.cpp",
      "platform/src/**.cpp",
      "common/src/**.cpp",
      "graphics/src/**.cpp",
      "canvas/src/**.cpp",
      "scene/src/**.cpp"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"

   filter "configurations:Release"
      defines { "NDEBUG" }
      optimize "On"

   