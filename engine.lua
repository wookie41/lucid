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
      "libs/glad/include", 
      "libs/SDL2/include",
      "libs/SDL2/include/SDL2",
      "libs/assimp/include",
      "libs/rapidjson",
      "libs/df_serialize",
      "libs/sole/include",
      "libs/simplex_noise/include"
   }
   
   libdirs { 
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
      "engine/devices/include/**.hpp",
      "engine/platform/src/**.cpp",
      "engine/platform/include/**.hpp",
      "engine/common/src/**.cpp",
      "engine/common/include/**.tpp",
	  "engine/common/include/**.hpp",
      "engine/misc/src/**.cpp",
      "engine/misc/include/**.tpp",
      "engine/misc/include/**.hpp",
      "engine/scene/src/**.cpp",
      "engine/scene/include/**.tpp",
      "engine/scene/include/**.hpp",
      "engine/resources/src/**.cpp",
      "engine/resources/include/**.tpp",
      "engine/resources/include/**.hpp",
      "engine/schemas/src/**.cpp",
      "engine/schemas/include/**.tpp",
      "engine/schemas/include/**.hpp",
      "engine/*.cpp",
      "engine/imgui/include/*.h",
      "engine/imgui/src/*.cpp",
      "libs/rapidjson/**.h",
      "libs/df_serialize/*.h",
      "libs/glad/src/glad.c",
      "libs/simplex_noise/src/*.cpp"  
   }

   filter "platforms:Win64"
      architecture "x86_64"
   
   filter "platforms:Win32"
      architecture "x86"
   
   filter "platforms:Linux"
      architecture "x86_64"

   filter "system:linux"
      links { 
         "GL", 
      }

   filter "system:windows"
      links { 
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