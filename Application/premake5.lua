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
				"src/**.cpp",
				"vendor/Microsoft/**.h",
				"vendor/Microsoft/**.cpp",
				"vendor/FDLuna/**.h",
				"vendor/FDLuna/**.cpp",
			}

			includedirs 
			{
				"%{wks.location}/Framework/vendor/spdlog/include",
				"%{wks.location}/Framework/vendor/Microsoft",
				"%{wks.location}/Framework/vendor/FDLuna",
				"%{wks.location}/Framework/src",
				"%{IncludeDir.entt}",
				"%{IncludeDir.ImGuizmo}"
			}

			links
			{
				"Framework"
			}

			filter "system:windows"
				systemversion "latest"

			defines
			{
				"CM_PLATFORM_WINDOWS",
				"USE_PIX"
			}

			filter "configurations:Debug"
			defines	"FD_DEBUG"
			runtime "Debug"
			symbols "on"

			filter "configurations:Release"
			defines	"FD_RELEASE"
			runtime "Release"
			optimize "on"
