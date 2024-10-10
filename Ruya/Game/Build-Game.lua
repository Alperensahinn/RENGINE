project "Game"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",

	  -- Include Core
      "../Core/Source/Vendor/Vulkan/Include",
	  "../Core/Source",
      "../Core/Source/Vendor/glfw-3.4.WIN64/include",
      "../Core/Source/Vendor/glm",
      "../Core/Source/Vendor/assimp/Include",
      "../Core/Source/Vendor/stb_image"
   }

   libdirs 
   { 
      "../Core/Source/Vendor/Vulkan/Lib", 
      "../Core/Source/Vendor/glfw-3.4.WIN64/lib", 
      "../Core/Source/Vendor/assimp/lib" 
   }

   links
   {
      "Core",
      "vulkan-1",
      "glfw3",
      "assimp-vc143-mt"
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "_DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "NDEBUG" }
       runtime "Release"
       optimize "On"
       symbols "Off"