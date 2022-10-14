#pragma once
#include <memory>
#include <utility>


// @brief A macro for shifting bits by 'x' amount. (very lazy)
#define BIT(x) (1 << x)

// @brief A macro for binding function addresses to events
#define BIND_DELEGATE(f) std::bind(&f, this, std::placeholders::_1)

namespace DX12Framework
{
	// templated swap function
	template<typename T>
	void Swap(T& a, T& b)
	{
		T tmp = std::move(a);
		a = std::move(b);
		b = std::move(tmp);
	}

	// typedef unique pointer
	template<typename T>
	using ScopePointer = std::unique_ptr<T>;

	template<typename T, typename ... Args>
	constexpr ScopePointer<T> CreateScope(Args&& ... args)
	{
		return std::make_unique<T>(std::forward<Args>(args)...);
	}

	// typedef shared pointer
	template<typename T>
	using RefPointer = std::shared_ptr<T>;

	template<typename T, typename ... Args>
	constexpr RefPointer<T> CreateRef(Args&& ... args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	// typedef weak pointer
	template<typename T>
	using WeakPointer = std::weak_ptr<T>;

	template<typename T, typename ... Args>
	constexpr WeakPointer<T> CreateWeak(Args&& ... args)
	{
		return std::weak_ptr<T>(std::forward<Args>(args)...);
	}
}