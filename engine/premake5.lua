function use_engine()
    links "Engine"
    includedirs "%{wks.location}/engine/includes"
end

project "Engine"
    kind "StaticLib"
    language "C++"
    cppdialect "C++20"
    targetname "engine"
    targetdir("../bin/" .. outputdir)
    objdir("../bin/" .. outputdir .. "/obj/%{prj.name}")
    debugdir ("../bin/" .. outputdir)
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

    includedirs {        
        "src/",
        "includes/"
    }