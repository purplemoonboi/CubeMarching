include "./vendor/premake/premake_customisation/solutions_items.lua"

workspace "DX12Framework"
    architecture "x64"
    startproject "DX12Application"

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
group ""

include "DX12Framework"
include "DX12Application"
