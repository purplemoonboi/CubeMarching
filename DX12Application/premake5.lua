project "DX12Application"
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
				"%{wks.location}/DX12Framework/vendor/spdlog/include",
				"%{wks.location}/DX12Framework/vendor",
				"%{wks.location}/DX12Framework/src"
				
			}

			links
			{
				"DX12Framework"
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
