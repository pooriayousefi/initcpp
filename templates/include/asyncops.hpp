
#pragma once
#include <stdexcept>
#include <exception>
#include <iostream>
#include <coroutine>
#include <vector>
#include <forward_list>
#include <thread>
#include <variant>
#include <utility>
#include <semaphore>
#include <memory>
#include <cassert>

/**********************************************************************************************
*
*                   			Asynchronous Operations
*                   			-----------------------
*    			This header provides utilities for asynchronous programming
*    			using C++20 coroutines. It includes:
*    			- A Generator class template for creating coroutine-based generators.
*    			- A GeneratorFactory class template for managing pools of objects.
*    			- An awaitable Task class template for defining asynchronous tasks.
*    			- A SyncWaitTask class template and sync_wait function for synchronously
*    			  waiting on asynchronous tasks to complete.
*
*                   			Developed by: Pooria Yousefi
*				   				Date: 2025-06-26
*				   				License: MIT
*
**********************************************************************************************/

// anonymous namespace
namespace
{
    template<class T> 
	struct Generator
	{
		struct Promise
		{
			T current_value;
			inline decltype(auto) initial_suspend() { return std::suspend_always{}; }
			inline decltype(auto) final_suspend() noexcept { return std::suspend_always{}; }
			inline decltype(auto) get_return_object() { return Generator{ std::coroutine_handle<Promise>::from_promise(*this) }; }
			inline decltype(auto) return_void() { return std::suspend_never{}; }
			inline decltype(auto) yield_value(T&& value) noexcept { current_value = value; return std::suspend_always{}; }
			inline void unhandled_exception() { std::terminate(); }
		};
        using promise_type = Promise;
		struct Sentinel {};
		struct Iterator
		{
			using iterator_category = std::input_iterator_tag;
			using value_type = T;
			using difference_type = ptrdiff_t;
			using pointer = T*;
			using reference = T&;
			using const_reference = const T&;
			std::coroutine_handle<promise_type> handle;
			explicit Iterator(std::coroutine_handle<promise_type>& h) :handle{ h } {}
			inline Iterator& operator++()
			{
				handle.resume();
				return *this;
			}
			inline void operator++(int) { (void)operator++(); }
			inline reference operator*() { return handle.promise().current_value; }
			inline pointer operator->() { return std::addressof(operator*()); }
			inline const_reference operator*() const { return handle.promise().current_value; }
			inline pointer operator->() const { return std::addressof(operator*()); }
			inline bool operator==(Sentinel) { return handle.done(); }
			inline bool operator==(Sentinel) const { return handle.done(); }
		};
		std::coroutine_handle<promise_type> handle;
		explicit Generator(std::coroutine_handle<promise_type> h) :handle{ h } {}
		~Generator() { if (handle) handle.destroy(); }
		Generator(const Generator&) = delete;
		Generator(Generator&& other) noexcept :handle(other.handle) { other.handle = nullptr; }
		constexpr Generator& operator=(const Generator&) = delete;
		constexpr Generator& operator=(Generator&& other) noexcept { handle = other.handle; other.handle = nullptr; return *this; }
		inline T get_value() { return handle.promise().current_value; }
		inline bool next() { handle.resume(); return !handle.done(); }
		inline bool resume() { handle.resume(); return !handle.done(); }
		inline decltype(auto) begin()
		{
			handle.resume();
			return Iterator{ handle };
		}
		inline decltype(auto) end() { return Sentinel{}; }
		inline T get_next_value()
		{
			next();
			if (handle.done()) throw std::out_of_range{ "Generator exhausted" };
			return get_value();
		}
	};

    template<class T, size_t N = 128> 
	class GeneratorFactory
	{
	public:

		// type alises
        using Pool = std::vector<T>;
        using Pools = std::forward_list<Pool>;

		// fixed number of objects in each pool
		// can be adjusted as needed
		// default is 128
		// this can be changed to a template parameter if needed
		// to allow different pool sizes for different types
		// but for simplicity, we use a fixed size here
		// note: the pool size should be a reasonable number
		// too small a pool size will lead to frequent allocations
		// too large a pool size will lead to wasted memory
		// so choose a size that balances memory usage and allocation frequency
		// for most use cases, 128 is a good starting point
		// you can adjust it based on your specific needs
		// for example, if you know that you will be creating many objects
		// you might want to increase the pool size to reduce allocation overhead
		// conversely, if you are creating few objects, a smaller pool size might be sufficient
		// ultimately, the optimal pool size will depend on your application's usage patterns
		// and memory constraints.
        static constexpr inline size_t number_of_objects_in_each_pool = N;

		// defaut constructor
		GeneratorFactory():m_pools{}, m_object_counter{ 0 }
		{
			m_pools.emplace_front(Pool(number_of_objects_in_each_pool, T{}));
		}

		// destructor
		virtual ~GeneratorFactory()
		{
			for (auto& pool : m_pools)
			{
				pool.clear();
			}
			m_pools.clear();
		}

		// generate method
		// returns a generator that yields shared pointers to objects of type T
		// it creates new pools of objects as needed
		// each pool contains N objects of type T
		// when an object is requested, it is returned as a shared pointer
		// if all objects in the current pool are used, a new pool is created
		// the generator will yield objects indefinitely
		// example usage:
		// GeneratorFactory<MyClass> factory;
		// auto gen = factory.generate();
		// for (auto obj : gen) { ... }
		// this will yield shared pointers to MyClass objects
		// the generator will create new pools of MyClass objects as needed
		// each pool will contain N MyClass objects
		// the generator will yield objects indefinitely
		// note: the objects are default constructed
		// if T does not have a default constructor, this will not compile
		// if you need to pass parameters to the constructor, you will need to modify this
		// to accept constructor parameters and forward them to the object creation
		// this is a simple implementation for demonstration purposes
		// it can be extended to support more features as needed
		// such as custom object initialization, object recycling, etc.
		// also note that this implementation is not thread-safe
		// if you need thread safety, you will need to add synchronization mechanisms
		// such as mutexes or atomic operations
		// be aware that adding thread safety may impact performance
		// so consider your use case carefully before adding such features
		// this implementation is intended for single-threaded use cases
		// or where the generator is used in a controlled manner
		// without concurrent access from multiple threads
		// if you need a thread-safe version, consider using a thread-safe queue
		// or other concurrency primitives to manage access to the pools and objects
		// this is left as an exercise for the reader
		// happy coding!
		inline Generator<std::shared_ptr<T>> generate()
		{
			while (true)
			{
                if (m_object_counter < number_of_objects_in_each_pool)
                {
                    co_yield std::make_shared<T>(m_pools.begin()->data()[m_object_counter++]);
                }
                else
                {
                    m_pools.emplace_front(Pool(number_of_objects_in_each_pool, T{}));
                    m_object_counter = 0;
                }
			}
		}
	private:

		// private members
		Pools m_pools;
		size_t m_object_counter;
	};

	// awaitable Task data structure
	template<class T> 
    struct Task
	{
		struct promise_type
		{
			std::variant<std::monostate, T, std::exception_ptr> result;
			std::coroutine_handle<> continuation;
			constexpr decltype(auto) get_return_object() noexcept { return Task{ *this }; }
			constexpr void return_value(T value) { result.template emplace<1>(std::move(value)); }
			constexpr void unhandled_exception() noexcept { result.template emplace<2>(std::current_exception()); }
			constexpr decltype(auto) initial_suspend() { return std::suspend_always{}; }
			struct awaitable
			{
				constexpr bool await_ready() noexcept { return false; }
				constexpr decltype(auto) await_suspend(std::coroutine_handle<promise_type> h) noexcept
				{
					return h.promise().continuation;
				}
				constexpr void await_resume() noexcept {}
			};
			constexpr decltype(auto) final_suspend() noexcept { return awaitable{}; }
		};
		std::coroutine_handle<promise_type> handle;
		explicit Task(promise_type& p) noexcept :handle{ std::coroutine_handle<promise_type>::from_promise(p) } {}
		Task(Task&& t) noexcept :handle{ t.handle } {}
		~Task() { if (handle) handle.destroy(); }
		constexpr bool await_ready() { return false; }
		constexpr decltype(auto) await_suspend(std::coroutine_handle<> c)
		{
			handle.promise().continuation = c;
			return handle;
		}
		constexpr T await_resume()
		{
			auto& result = handle.promise().result;
			if (result.index() == 1)
				return std::get<1>(std::move(result));
			else
				std::rethrow_exception(std::get<2>(std::move(result)));
		}
	};
	template<> 
    struct Task<void>
	{
		struct promise_type
		{
			std::exception_ptr e;
			std::coroutine_handle<> continuation;
			inline decltype(auto) get_return_object() noexcept { return Task{ *this }; }
			constexpr void return_void() {}
			inline void unhandled_exception() noexcept { e = std::current_exception(); }
			constexpr decltype(auto) initial_suspend() { return std::suspend_always{}; }
			struct awaitable
			{
				constexpr bool await_ready() noexcept { return false; }
				inline decltype(auto) await_suspend(std::coroutine_handle<promise_type> h) noexcept
				{
					return h.promise().continuation;
				}
				constexpr void await_resume() noexcept {}
			};
			constexpr decltype(auto) final_suspend() noexcept { return awaitable{}; }
		};
		std::coroutine_handle<promise_type> handle;
		explicit Task(promise_type& p) noexcept :handle{ std::coroutine_handle<promise_type>::from_promise(p) } {}
		Task(Task&& t) noexcept :handle{ t.handle } {}
		~Task() { if (handle) handle.destroy(); }
		constexpr bool await_ready() { return false; }
		inline decltype(auto) await_suspend(std::coroutine_handle<> c)
		{
			handle.promise().continuation = c;
			return handle;
		}
		inline void await_resume()
		{
			if (handle.promise().e)
				std::rethrow_exception(handle.promise().e);
		}
	};

	// result data structure
	template<class T> using ResultType = decltype(std::declval<T&>().await_resume());

	/*
	The forced chaining of coroutines has the interesting effect that we finally get to the
	main() function of the program, which the C++ standard says is not allowed to be
	a coroutine. This needs to be addressed somehow, and the proposed solution is to
	provide at least one function that synchronously waits on the asynchronous chains
	to complete. For example, the CppCoro library includes the function sync_wait(),
	which has this effect of breaking the chain of coroutines, which makes it possible
	for an ordinary function to use coroutines.
	*/
	// implementing SyncWaitTask helper data structure
	template<class T> 
    struct SyncWaitTask
	{
		struct promise_type
		{
			T* value{ nullptr };
			std::exception_ptr error{ nullptr };
			std::binary_semaphore sema4{ 0 };
			inline SyncWaitTask get_return_object() noexcept { return SyncWaitTask{ *this }; }
			constexpr void unhandled_exception() noexcept { error = std::current_exception(); }
			constexpr decltype(auto) yield_value(T&& x) noexcept
			{
				value = std::addressof(x);
				return final_suspend();
			}
			constexpr decltype(auto) initial_suspend() noexcept { return std::suspend_always{}; }
			struct awaitable
			{
				constexpr bool await_ready() noexcept { return false; }
				constexpr void await_suspend(std::coroutine_handle<promise_type> h) noexcept { h.promise().sema4.release(); }
				constexpr void await_resume() noexcept {}
			};
			constexpr decltype(auto) final_suspend() noexcept { return awaitable{}; }
			constexpr void return_void() noexcept { assert(false); }
		};
		std::coroutine_handle<promise_type> handle;
		explicit SyncWaitTask(promise_type& p) noexcept :handle{ std::coroutine_handle<promise_type>::from_promise(p) } {}
		SyncWaitTask(SyncWaitTask&& t) noexcept :handle{ t.handle } {}
		~SyncWaitTask() { if (handle) handle.destroy(); }
		inline T&& get()
		{
			auto& p = handle.promise();
			handle.resume();
			p.sema4.acquire();
			if (p.error)
				std::rethrow_exception(p.error);
			return static_cast<T&&>(*p.value);
		}
	};

	// implementing sync_wait()
	template<class T> ResultType<T> sync_wait(T&& Task)
	{
		if constexpr (std::is_void_v<ResultType<T>>)
		{
			struct empty_type {};
			auto coro = [&]() -> SyncWaitTask<empty_type>
				{
					co_await std::forward<T>(Task);
					co_yield empty_type{};
					assert(false);
				};
			coro().get();
		}
		else
		{
			auto coro = [&]() -> SyncWaitTask<ResultType<T>>
				{
					co_yield co_await std::forward<T>(Task);
					assert(false);
				};
			return coro().get();
		}
	}
}
                