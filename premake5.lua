-- premake5.lua
workspace "lucid"
   configurations { "Debug", "Release" }
   platforms { "Win32", "Win64", "Linux"}

project "lucid"
   kind "WindowedApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"

   includedirs { "." }

   includedirs { 
      "platform/include", 
      "devices/include", 
      "common/include", 
      "misc/include", 
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
      "SDL2", 
      "SDL2main"
   }

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

   filter { "platforms:Win64" }
      architecture "x86_64"
   
   filter { "platforms:Win32" }
      architecture "x86"
   
   filter { "platforms:Linux" }
      architecture "x86_64"

   files { 
      "main.cpp", 
      "devices/src/gpu/**.cpp",
      "devices/src/gpu/gl/**.cpp",
      "platform/src/sdl/**.cpp",
      "platform/src/**.cpp",
      "common/src/**.cpp",
      "misc/src/**.cpp",
      "scene/src/**.cpp"
   }

   filter "configurations:Debug"
      defines { "DEBUG" }
      symbols "On"
      optimize "Off"

   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"

   