include "./vendor/premake/premake_customisation/solutions_items.lua"
include "Dependencies.lua"

workspace "Engine"
    architecture "x64"
    startproject "Application"

    configurations
    {
        "Debug",
        "Release"
    }

    solutions_items
    {
       ".editorconfig"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "vendor/premake"
include "Framework/vendor/ImGui"
include "Framework/vendor/yaml-cpp"
group ""

include "Framework"
include "Application"

