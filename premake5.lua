-- Building
-- MsBuild XPlayer.sln /p:configuration=Release

workspace "DeadlineDash"
   architecture "x64"
   configurations { "Debug", "Release", "Dist" }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

includeDirs = {}
includeDirs["glfw"] = "packages/glfw/include"
includeDirs["SpdLog"] = "packages/spdlog/include"
includeDirs["ImGui"] = "packages/imgui"

-- /MP -- Multithreaded build 
-- /MT -- Static Linking. Defines _MT 
-- /MD -- Dynamic Linking. Defines _MT and _DLL 
include "packages/glfw"
include "packages/imgui"

project "deadlinedash"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++17"
   targetdir "bin"
   objdir "bin/obj"
   pchheader "pch.h"
   pchsource "src/pch.cpp"

   links {
      "glfw", "ImGui", "opengl32", "dwmapi", "Shlwapi", "winmm", "userenv", "Shell32"
   }

   includedirs {
      "src",
      "%{includeDirs.glfw}",
      "%{includeDirs.ImGui}",
      "%{includeDirs.SpdLog}"
   }

   files { 
      "src/**.cpp"
   }

   -- Add your manifest and resource file
   files { 'setup.rc', '**.ico' }
   vpaths { ['./*'] = { '*.rc', '*.manifest', '**.ico' } }

   filter "system:windows"
      systemversion "latest"

   -- Post-build command to embed the manifest file
   -- postbuildcommands {
   --    'mt.exe -manifest "%{wks.location}DeadlineDash.manifest" -outputresource:"%{cfg.targetdir}/DeadlineDash.txt;1"'
   -- }

   filter "configurations:Debug"
      runtime "Debug"
      symbols "On"
      staticruntime "On"
      optimize "Off"
      buildoptions { "/MP", "/DEBUG:FULL" }
      defines { "GL_DEBUG" }

   filter "configurations:Release"
      runtime "Release"
      optimize "On"
      symbols "Off"
      characterset ("MBCS")
      staticruntime "On"
      buildoptions { "/MP", "/utf-8" }
      defines { "GL_DEBUG", "_CRT_SECURE_NO_WARNINGS" }

   filter "configurations:Dist"
      kind "WindowedApp"
      runtime "Release"
      optimize "On"
      symbols "Off"
      characterset ("MBCS")
      staticruntime "On"
      buildoptions { "/MP", "/utf-8" }
      linkoptions { "/ENTRY:mainCRTStartup" }
