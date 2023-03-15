-- Conan package manager
include("conanbuildinfo.premake.lua")

workspace "Mill Engine"
    conan_basic_setup()    
    
    configurations { "Debug", "Release", "Distro" }
    platforms { "Win64", "Linux" }    
    
    filter "configurations:Debug"
        defines { "DEBUG", "MILL_DEBUG" }
        symbols "On"
    
    filter "configurations:Release"
        defines { "NDEBUG", "MILL_RELEASE" }
        
    filter "configurations:Distro"
        defines { "NDEBUG", "MILL_DISTRO" }
        optimize "On"
        
    filter "platforms:Win64"
        system "windows"
        architecture "x64"
        defines { "MILL_WINDOWS" }
        
    filter "platforms:Linux"
        system "linux"
        architecture "x64"
        defines { "MILL_LINUX" }
        
    filter ""

    outputdir = "%{cfg.architecture}-%{cfg.system}-%{cfg.buildcfg}"

    group "Engine"
        include "engine"
    group "Apps"
        include "apps/sandbox"
    group ""