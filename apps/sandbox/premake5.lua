project "Sandbox"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    targetname "app"
    targetdir("../../bin/" .. outputdir)
    objdir("../../bin/" .. outputdir .. "/obj/%{prj.name}")
    debugdir ("../../bin/" .. outputdir)
    staticruntime "Off"

    flags
    {
        "MultiProcessorCompile",
        "FatalCompileWarnings",
    }
    warnings "High"
    externalwarnings "Off"
    externalanglebrackets "On"
    
    linkoptions { conan_exelinkflags }

    files {
        "src/**"
    }

    use_engine()
