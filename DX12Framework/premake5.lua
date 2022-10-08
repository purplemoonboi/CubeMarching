project "DX12Framework"
		kind "ConsoleApp"
			language "C++"
			cppdialect "C++20"
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
				"src"
			}

			links
			{
			   
			}


			filter "system:windows"
				systemversion "latest"

			defines
			{
				"_CRT_SECURE_NO_WARNINGS"
			}

			filter "configurations:Debug"
			defines	"CM_DEBUG"
			runtime "Debug"
			symbols "on"

			filter "configurations:Release"
			defines	"CM_RELEASE"
			runtime "Release"
			optimize "on"







