-- premake5.lua
workspace "lucid"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux"}

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
      "misc/include", 
      "scene/include",
      "resources/include" 
   }

   includedirs { 
      "libs/glm", 
      "libs/stb", 
      "libs/glew/include", 
      "libs/SDL2/include",
      "libs/assimp/include" 
   }
   
   libdirs { 
      "libs/glew/lib/x64", 
      "libs/SDL2/lib/x64",
      "libs/assimp/lib/x64" 
   }

   links { 
      "SDL2",
      "assimp"
   }

   files { 
      "libs/stb/stb_init.cpp",
      "main.cpp", 
      "devices/src/gpu/**.cpp",
      "devices/src/gpu/gl/**.cpp",
      "platform/src/sdl/**.cpp",
      "platform/src/**.cpp",
      "common/src/**.cpp",
      "misc/src/**.cpp",
      "scene/src/**.cpp",
      "resources/src/**.cpp"
   }

   filter "platforms:Win64"
      architecture "x86_64"
   
   filter "platforms:Win32"
      architecture "x86"
   
   filter "platforms:Linux"
      architecture "x86_64"

   filter "system:linux"
      links { 
         "GLEW", 
         "GL", 
      }

   filter "system:windows"
      links { 
         "glew32", 
         "opengl32", 
      }
   
   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      optimize "Off"

   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"
   
   filter {}