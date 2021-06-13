project "lucid_scene_editor"
   kind "ConsoleApp"
   language "C++"
   targetdir "bin/%{cfg.buildcfg}"
   cppdialect "C++17"

   includedirs { "." }

   includedirs {
      "engine",  
      "engine/platform/include", 
      "engine/devices/include", 
      "engine/common/include", 
      "engine/misc/include", 
      "engine/scene/include",
      "engine/resources/include", 
      "engine/schemas/include", 
      "libs/stb" 
   }

   includedirs {
      "tools/scene_editor/include", 
      "libs/glm", 
      "libs/SDL2/include",
      "libs/df_serialize",
      "libs/rapidjson",
      "libs/sole/include"
   }
   
   libdirs { 
      "engine/bin/Debug",
      "libs/SDL2/lib/x64",
      "libs/assimp/lib/x64" 
   }

   links { 
      "SDL2",
      "SDL2main",
      "assimp",
      "lucid_engine"
   }

   files {
      "tools/scene_editor/src/*.cpp"
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
      symbols "On"
      optimize "Off"

   filter "configurations:Release"
      defines { "NDEBUG" }
      symbols "Off"
      optimize "On"
   
   filter {}


--   @TODO Filter
    defines { "DEVELOPMENT=1" }


-- TODO include only in development
   includedirs {
        "engine/imgui/include"
   }
