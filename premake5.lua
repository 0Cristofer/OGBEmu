workspace "OGBEmu"
	architecture "x64"
	startproject "OGBEmu"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["SDL3"] = "vendor/SDL3/include"

project "OGBEmu"
	location "OGBEmu"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
	}

	defines
	{
	}

	includedirs
	{
		"%{prj.name}/src",
		"%{IncludeDir.SDL3}",
	}

	libdirs
	{
		"vendor/SDL3/lib/x64",
	}

	links 
	{
		"SDL3"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"

project "Tests"
	location "tests"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++20"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"tests/**.h",
		"tests/**.cpp",
		"OGBEmu/src/**.h",
		"OGBEmu/src/**.cpp",
	}
	
	removefiles
	{
		"OGBEmu/src/main.cpp",
		"OGBEmu/src/Emulator/Screen.cpp",
		"OGBEmu/src/Emulator/Ppu.cpp",
		"OGBEmu/src/Emulator/Device.cpp",
	}

	includedirs
	{
		"OGBEmu/src",
		"%{IncludeDir.SDL3}",
	}

	libdirs
	{
		"vendor/SDL3/lib/x64",
	}

	links 
	{
		"SDL3"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "HZ_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "HZ_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "HZ_DIST"
		runtime "Release"
		optimize "on"