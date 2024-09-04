project "Core"
   kind "StaticLib"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",
      "C:/VulkanSDK/1.3.290.0/Include",
      "Source/Vendor/glm",
      "Source/Vendor/glfw-3.4.WIN64/include",
      "Source/Vendor/assimp/Include",
   }

   libdirs 
   { 
      "C:/VulkanSDK/1.3.290.0/Lib", 
      "Source/Vendor/glfw-3.4.WIN64/lib", 
      "Source/Vendor/assimp/lib" 
   }

   links
   {
      "vulkan-1",
      "glfw3",
      "assimp-vc143-mt",
   }

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { }

   filter "configurations:Debug"
       defines { "_DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "NDEBUG"}
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"