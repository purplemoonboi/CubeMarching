#include "Framework/cmpch.h"
#include "Log.h"
#include "spdlog/sinks/stdout_color_sinks.h"

namespace DX12Framework
{

	std::shared_ptr<spdlog::logger> Log::CoreLog;
	std::shared_ptr<spdlog::logger> Log::ClientLog;

	void Log::Init()
	{
		spdlog::set_pattern("%^ [%T]: %v%$");

		CoreLog = spdlog::stdout_color_mt("DX12Application");

		CoreLog->set_level(spdlog::level::trace);

		ClientLog = spdlog::stdout_color_mt("Application");
		ClientLog->set_level(spdlog::level::trace);

		
	}

}