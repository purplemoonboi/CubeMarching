#pragma once
#include <stdint.h>
#include <assert.h>
#include <functional>
#include <typeinfo>
#include <memory>
#include <utility>
#include <mutex>


// Disable exception warning
#pragma warning(disable: 4530)

// BIT shift by 'x' amount
#define BIT(x) (1 << (x))

// Bind callback function
#define BIND_DELEGATE(f) std::bind(&f, this, std::placeholders::_1)

// Disable copy semantics
#ifndef DISABLE_COPY
#define DISABLE_COPY(T)						\
			explicit T(const T&) = delete;	\
			T& operator=(const T&)=delete;	
#endif

// Disable move semantics
#ifndef DISABLE_MOVE
#define DISABLE_MOVE(T)						\
			explicit T(T&&) = delete;		\
			T& operator=(T&&)=delete;	
#endif


// Disable copy and move semantics 
#ifndef DISABLE_COPY_AND_MOVE
#define DISABLE_COPY_AND_MOVE(T) DISABLE_COPY(T) DISABLE_MOVE(T)
#endif

// Generate copy semantics
#ifndef GENERATE_COPY
#define GENERATE_COPY(T)\
	explicit T(const T&);\
	T& operator=(const T&);
#endif

// Generate move semantics
#ifndef GENERATE_MOVE
#define GENERATE_MOVE(T)\
	explicit T(T&&);\
	T& operator=(T&&);
#endif

// Generate copy and move semantics
#ifndef GENERATE_COPY_AND_MOVE
#define GENERATE_COPY_AND_MOVE(T) GENERATE_COPY(T) GENERATE_MOVE(T)
#endif



namespace Foundation
{
	class Event;

	// templated swap function
	template<typename T>
	void Swap(T& a, T& b)
	{
		T tmp = std::move(a);
		a = std::move(b);
		b = std::move(tmp);
		
	}

	// typedef function delegate
	using Delegate = std::function<void(Event&)>;

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