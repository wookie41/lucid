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
      "libs/stb/stb_init.hpp",
      "main.cpp", 
      "devices/src/**.cpp",
      "devices/include/**.hpp",
      "devices/include/**.tpp",
      "platform/src/**.cpp",
      "platform/include/**.hpp",
      "common/src/**.cpp",
      "common/include/**.hpp",
      "common/include/**.tpp",
      "misc/src/**.cpp",
      "misc/include/**.hpp",
      "scene/src/**.cpp",
      "scene/include/*.hpp",
      "resources/src/**.cpp",
      "resources/include/**.hpp"
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