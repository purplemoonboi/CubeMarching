project "Application"
		kind "ConsoleApp"
			language "C++"
			cppdialect "C++17"
			staticruntime "on"

			targetdir ("%{wks.location}/bin/" ..outputdir.. "/%{prj.name}")
			objdir ("%{wks.location}/bin-int/" ..outputdir.. "/%{prj.name}")

			files
			{
				"src/**.h",
				"src/**.cpp"
			}

			includedirs 
			{
				"%{wks.location}/Framework/vendor/spdlog/include",
				"%{wks.location}/Framework/vendor",
				"%{wks.location}/Framework/src"
				
			}

			links
			{
				"Framework"
			}

			filter "system:windows"
				systemversion "latest"

			defines
			{
				"CM_PLATFORM_WINDOWS"
			}

			filter "configurations:Debug"
			defines	"FD_DEBUG"
			runtime "Debug"
			symbols "on"

			filter "configurations:Release"
			defines	"FD_RELEASE"
			runtime "Release"
			optimize "on"