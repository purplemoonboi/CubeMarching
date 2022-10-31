project "Framework"
		kind "StaticLib"
			language "C++"
			cppdialect "C++17"
			staticruntime "on"

			targetdir ("%{wks.location}/bin/" ..outputdir.. "/%{prj.name}")
			objdir ("%{wks.location}/bin-int/" ..outputdir.. "/%{prj.name}")

			pchheader "cmpch.h"
			pchsource "src/cmpch.cpp"

			files
			{
				"src/**.h",
				"src/**.cpp"
			}

			includedirs
			{
				"src",
				"vendor/spdlog/include",
				"vendor/DX12/include"
			}

			links
			{
			   "d3dcompiler",
			   "D3D12",
			   "dxgi"
			}

			flags { "NoPCH" }

			filter "system:windows"
				systemversion "latest"

			defines
			{
				"_CRT_SECURE_NO_WARNINGS",
				"CM_WINDOWS_PLATFORM"
			}

			filter "configurations:Debug"
			defines	"CM_DEBUG"
			runtime "Debug"
			symbols "on"

			filter "configurations:Release"
			defines	"CM_RELEASE"
			runtime "Release"
			optimize "on"







