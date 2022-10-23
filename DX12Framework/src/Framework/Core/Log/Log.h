#pragma once
#include "Framework/Core/core.h"
#include "spdlog/spdlog.h"
#include <spdlog/fmt/ostr.h >



namespace DX12Framework
{
	class Log
	{
	public:
		
		static void Init();

		// @brief Returns a pointer to the engine's debug log.
		inline static std::shared_ptr<spdlog::logger>& GetCoreLog() { return CoreLog; }
		//@brief Returns a pointer to the application's debug log.
		inline static std::shared_ptr<spdlog::logger>& GetClientLog() { return ClientLog; }

	private:

		static std::shared_ptr<spdlog::logger> CoreLog;
		static std::shared_ptr<spdlog::logger> ClientLog;

	};

}

// @brief - Captures D3D failures outputting windows error code, function signature, file name and line number.
class CoreException
{
public:
	CoreException() = default;
	CoreException(HRESULT h, const std::wstring& funcName, const std::wstring& fileName, INT32 lineNumber)
		:
		ErrCode(h),
		FuncName(funcName),
		FileName(fileName),
		LineNumber(lineNumber)
	{}

	[[nodiscard]] std::wstring ToString() const;
	HRESULT ErrCode = S_OK;
	std::wstring FuncName;
	std::wstring FileName;
	INT32 LineNumber = -1;
};

#define WIDE2(x) L##x
#define WIDE1(x) WIDE2(x)
#define WFILE(x) WIDE1(x)//converts to wide format

#define THROW_ON_FAILURE(x) \
{\
HRESULT h = (x);\
std::wstring fN = WFILE(__FILE__);\
if(FAILED(h)){throw CoreException(h, L#x, fN, __LINE__);}\
}\


/**Core log MACROS**/
#define CORE_ERROR(...)   ::DX12Framework::Log::GetCoreLog()->error(__VA_ARGS__);
#define CORE_WARNING(...) ::DX12Framework::Log::GetCoreLog()->warn(__VA_ARGS__);
#define CORE_INFO(...)    ::DX12Framework::Log::GetCoreLog()->info(__VA_ARGS__);
#define CORE_TRACE(...)   ::DX12Framework::Log::GetCoreLog()->trace(__VA_ARGS__);

/**Client log MACROS**/
#define APP_ERROR(...)   ::DX12Framework::Log::GetClientLog()->error(__VA_ARGS__);
#define APP_WARNING(...) ::DX12Framework::Log::GetClientLog()->warn(__VA_ARGS__);
#define APP_INFO(...)    ::DX12Framework::Log::GetClientLog()->info(__VA_ARGS__);
#define APP_TRACE(...)   ::DX12Framework::Log::GetClientLog()->trace(__VA_ARGS__);


// @brief Assertion checks - for debugging core code only, 			 
// remove assertion checks for optimised builds.			
#if CM_DEBUG

	#define CORE_ASSERT(expr, ...) {if(!expr){CORE_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak();}}

	#define APP_ASSERT(expr, ...) {if(!expr){APP_ERROR("Assertion failed: {0}", __VA_ARGS__); __debugbreak();}}

#else

	//Do nothing - effectively strip assertions from code in optimised builds.
	#define CORE_ASSERT(expr, ...)
	#define APP_ASSERT(expr, ...)

#endif