function use_engine()
    links {"Engine", "$(VULKAN_SDK)/Lib/vulkan-1"}
    includedirs "%{wks.location}/engine/includes"
    defines {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",        
        "GLM_FORCE_LEFT_HANDED"        
    }
end

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetname "engine"
    targetdir("../bin/" .. outputdir)
    objdir("../bin/" .. outputdir .. "/obj/%{prj.name}")
    staticruntime "Off"

    flags
    {
        "MultiProcessorCompile",
        "FatalCompileWarnings",
    }
    warnings "High"
    externalwarnings "Off"
    externalanglebrackets "On"

    files {
        "src/**.cpp",
        "src/**.hpp",
        "src/**.h",
        "includes/**"
    }
    removefiles { "src/platform/**" }
    files { "src/platform/*.hpp" }

    includedirs {        
        "src/",
        "src/directx-headers/include/",
        "src/directx-headers/include/directx",
        "includes/"
    }

    defines {
        "GLM_FORCE_DEPTH_ZERO_TO_ONE",        
        "GLM_FORCE_LEFT_HANDED"        
    }

    filter "platforms:Win64"
        files { 
            "src/platform/glfw/**",
            "src/platform/vulkan/**"
            -- "src/platform/d3d12/**"
        }

    filter "platforms:Linux"
        files { 
            "src/platform/glfw/**",
            "src/platform/vulkan/**"
        }

    filter ""