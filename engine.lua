project "lucid_engine"
   kind "StaticLib"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"

   includedirs {
      ".",
      "engine/platform/include", 
      "engine/devices/include", 
      "engine/common/include", 
      "engine/misc/include", 
      "engine/scene/include",
      "engine/resources/include",
      "engine/schemas/include",
      "engine/imgui/include"       -- TODO include only in development
   }

   includedirs { 
      "libs/glm", 
      "libs/stb", 
      "libs/glew/include", 
      "libs/SDL2/include",
      "libs/SDL2/include/SDL2",
      "libs/assimp/include",
      "libs/rapidjson",
      "libs/df_serialize",
      "libs/sole/include"
   }
   
   libdirs { 
      "libs/glew/lib/x64", 
      "libs/SDL2/lib/x64",
      "libs/assimp/lib/x64" 
   }

   links { 
      "SDL2",
      "SDL2main",
      "assimp"
   }

   files { 
      "libs/stb/*",
      "engine/devices/src/**.cpp",
      "engine/devices/include/**.tpp",
      "engine/platform/src/**.cpp",
      "engine/platform/include/**.hpp",
      "engine/common/src/**.cpp",
      "engine/common/include/**.tpp",
      "engine/misc/src/**.cpp",
      "engine/scene/src/**.cpp",
      "engine/resources/src/**.cpp",
      "engine/resources/include/**.tpp",
      "engine/*.cpp",
      "engine/imgui/include/*.h",
      "engine/imgui/src/*.cpp",
      "libs/rapidjson/**.h",
      "libs/df_serialize/*.h" 
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
      symbols "Full"
      optimize "Off"

   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"
   
   filter {}
   
--   @TODO Filter
   defines { "DEVELOPMENT=1" }