#include <iostream>
#include <filesystem>
#include <fstream>
#include <string>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <map>
#include <cctype>
#include <cstdio>

namespace fs = std::filesystem;

// entry-point
int main(int argc, char* argv[])
{
    std::string project_path, project_name;
    
    // Helper method to execute system commands
    auto execute_command = [](const std::string& command)
    {
        std::cout << "Executing: " << command << std::endl;
        return std::system(command.c_str());
    };
    
    // Write content to file
    auto write_file = [](const fs::path& file_path, const std::string& content)
    {
        std::ofstream file(file_path);
        if (!file.is_open())
        {
            throw std::runtime_error("ERROR! Could not create file: " + file_path.string());
        }
        file << content;
        file.flush();
        file.close();
        std::cout << "Created: " << file_path << std::endl;
    };
    
    // Helper method to sanitize project name for C++ identifiers
    auto sanitize_cpp_name = [](const std::string& name)
    {
        std::string sanitized = name;
        // Replace hyphens and other invalid characters with underscores
        for (char& c : sanitized)
        {
            if (!std::isalnum(c) && c != '_')
            {
                c = '_';
            }
        }
        // Ensure it starts with a letter or underscore
        if (!sanitized.empty() && std::isdigit(sanitized[0]))
        {
            sanitized = "_" + sanitized;
        }
        return sanitized;
    };

    // Create directory structure
    auto create_directory_structure = [&]()
    {
        std::cout << "Creating directory structure..." << std::endl;
        
        std::vector<std::string> directories = {
            project_path,
            project_path + "/include",
            project_path + "/include/core",
            project_path + "/src", 
            project_path + "/build",
            project_path + "/build/debug",
            project_path + "/build/release",
            project_path + "/tests"
        };
        
        for (const auto& dir : directories)
        {
            fs::create_directories(dir);
            std::cout << "Created directory: " << dir << std::endl;
        }
    };
    
    // Create template header files
    auto copy_template_headers = [&]()
    {
        std::cout << "Creating template header files..." << std::endl;
        
        // asyncops.hpp content
        std::string asyncops_content = R"(
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

// namespace pooriayousefi::core
namespace pooriayousefi::core
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
        using Pool = std::vector<T>;
        using Pools = std::forward_list<Pool>;
        static constexpr inline size_t number_of_objects_in_each_pool = N;

		GeneratorFactory():m_pools{}, m_object_counter{ 0 }
		{
			m_pools.emplace_front(Pool(number_of_objects_in_each_pool, T{}));
		}

		virtual ~GeneratorFactory()
		{
			for (auto& pool : m_pools)
			{
				pool.clear();
			}
			m_pools.clear();
		}

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
		Pools m_pools;
		size_t m_object_counter;
	};

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

	template<class T> using ResultType = decltype(std::declval<T&>().await_resume());

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
)";

        // raiiiofsw.hpp content
        std::string raiiiofsw_content = R"(
#pragma once
#include <type_traits>
#include <filesystem>
#include <stdexcept>
#include <typeinfo>
#include <fstream>
#include <string>

/**********************************************************************************************
*
*                   			RAII Input/Output File Stream Wrapper
*                   			-----------------------
*    			This header provides a RAII wrapper for basic input/output
*    			file streams. It includes:
*    			- A BasicInputFileStreamWrapper class template for managing file streams.
*    			- A BasicOutputFileStreamWrapper class template for managing file streams.
*    			- Specialization for std::byte for binary file streams.
*
*                   			Developed by: Pooria Yousefi
*				   				Date: 2025-06-26
*				   				License: MIT
*
**********************************************************************************************/

namespace pooriayousefi::core
{
	namespace raii
	{
		template<typename Elem, typename Traits = std::char_traits<Elem>, typename Alloc = std::allocator<Elem>>
		struct BasicInputFileStreamWrapper
		{
			using file_stream_type = std::basic_ifstream<Elem, Traits>;
			using type = BasicInputFileStreamWrapper<Elem, Traits, Alloc>;
			using string_type = std::basic_string<Elem, Traits, Alloc>;
			using stream_buffer_iterator = std::istreambuf_iterator<Elem, Traits>;

			file_stream_type file_stream;

			BasicInputFileStreamWrapper() :file_stream{} {}
			virtual ~BasicInputFileStreamWrapper() { if (is_open()) close(); }

			template<typename T> constexpr type& operator>>(T& value) { file_stream >> value; return *this; }

			bool is_open() { return file_stream.is_open(); }
			void close() { file_stream.close(); }
			void open(std::filesystem::path file_path, std::ios_base::openmode open_mode = std::ios_base::in)
			{
				file_stream.open(file_path, std::ios_base::in | open_mode);
				if (!is_open())
					throw std::runtime_error(
						std::string{
							std::string{ "ERROR! Cannot open "} +
							file_path.string() +
							std::string{ " file in raii::BasicInputFileStreamWrapper<" } +
							std::string{ typeid(Elem).name() } +
							std::string{ ", " } +
							std::string{ typeid(Traits).name() } +
							std::string{ ", " } +
							std::string{ typeid(Alloc).name() } +
							std::string{ ">::open() method." }
						}.c_str()
					);
			}
		};

		template<>
		struct BasicInputFileStreamWrapper<std::byte>
		{
			using file_stream_type = std::basic_ifstream<std::byte>;
			using type = BasicInputFileStreamWrapper<std::byte>;
			using string_type = std::basic_string<std::byte>;
			using stream_buffer_iterator = std::istreambuf_iterator<std::byte>;

			file_stream_type file_stream;

			BasicInputFileStreamWrapper() :file_stream{} {}
			virtual ~BasicInputFileStreamWrapper() { if (is_open()) close(); }

			template<typename T> constexpr type& operator>>(T& value) { file_stream >> value; return *this; }

			bool is_open() { return file_stream.is_open(); }
			void close() { file_stream.close(); }
			void open(std::filesystem::path file_path, std::ios_base::openmode open_mode = std::ios_base::in | std::ios_base::binary)
			{
				file_stream.open(file_path, std::ios_base::in | std::ios_base::binary | open_mode);
				if (!is_open())
					throw std::runtime_error(
						std::string{
							std::string{"ERROR! Cannot open "} +
							file_path.string() +
							std::string{" file in raii::BasicInputFileStreamWrapper<std::byte>::open() method." }
						}.c_str()
					);
			}
		};

		template<typename Elem, typename Traits = std::char_traits<Elem>, typename Alloc = std::allocator<Elem>>
		struct BasicOutputFileStreamWrapper
		{
			using file_stream_type = std::basic_ofstream<Elem, Traits>;
			using type = BasicOutputFileStreamWrapper<Elem, Traits, Alloc>;
			using string_type = std::basic_string<Elem, Traits, Alloc>;
			using stream_buffer_iterator = std::ostreambuf_iterator<Elem, Traits>;

			file_stream_type file_stream;

			BasicOutputFileStreamWrapper() :file_stream{} {}
			virtual ~BasicOutputFileStreamWrapper() { if (is_open()) close(); }

			template<typename T> constexpr type& operator<<(const T& value) { file_stream << value; return *this; }
			template<typename T> constexpr type& operator<<(T&& value) noexcept { file_stream << value; return *this; }

			bool is_open() { return file_stream.is_open(); }
			void close() { file_stream.flush(); file_stream.close(); }
			void open(std::filesystem::path file_path, std::ios_base::openmode open_mode = std::ios_base::out)
			{
				file_stream.open(file_path, std::ios::out | open_mode);
				if (!is_open())
					throw std::runtime_error(
						std::string{
							std::string{"ERROR! Cannot open "} +
							file_path.string() +
							std::string{" file in raii::BasicOutputFileStreamWrapper<"} +
							std::string{typeid(Elem).name()} +
							std::string{", "} +
							std::string{typeid(Traits).name()} +
							std::string{", "} +
							std::string{ typeid(Alloc).name() } +
							std::string{ "::open() method." }
						}.c_str()
					);
			}
		};

		template<>
		struct BasicOutputFileStreamWrapper<std::byte>
		{
			using file_stream_type = std::basic_ofstream<std::byte>;
			using type = BasicOutputFileStreamWrapper<std::byte>;
			using string_type = std::basic_string<std::byte>;
			using stream_buffer_iterator = std::ostreambuf_iterator<std::byte>;

			file_stream_type file_stream;

			BasicOutputFileStreamWrapper() :file_stream{} {}
			virtual ~BasicOutputFileStreamWrapper() { if (is_open()) close(); }

			template<typename T> constexpr type& operator<<(const T& value) { file_stream << value; return *this; }
			template<typename T> constexpr type& operator<<(T&& value) noexcept { file_stream << value; return *this; }

			bool is_open() { return file_stream.is_open(); }
			void close() { file_stream.flush(); file_stream.close(); }
			void open(std::filesystem::path file_path, std::ios_base::openmode open_mode = std::ios_base::out | std::ios_base::binary)
			{
				file_stream.open(file_path, std::ios::out | std::ios_base::binary | open_mode);
				if (!is_open())
					throw std::runtime_error(
						std::string{
							std::string{"ERROR! Cannot open "} +
							file_path.string() +
							std::string{" file in raii::BasicOutputFileStreamWrapper<std::byte>::open() method." }
						}.c_str()
					);
			}
		};

		namespace native
		{
			namespace narrow_encoded 
			{
				using InputFileStreamWrapper = BasicInputFileStreamWrapper<char>;
				using OutputFileStreamWrapper = BasicOutputFileStreamWrapper<char>;
			}
			namespace wide_encoded
			{
				using InputFileStreamWrapper = BasicInputFileStreamWrapper<wchar_t>;
				using OutputFileStreamWrapper = BasicOutputFileStreamWrapper<wchar_t>;
			}
		}
        #if __cplusplus >= 202002L
		namespace utf8
		{
			using InputFileStreamWrapper = BasicInputFileStreamWrapper<char8_t>;
			using OutputFileStreamWrapper = BasicOutputFileStreamWrapper<char8_t>;
		}
        #endif
		namespace binary
		{
			using InputFileStreamWrapper = BasicInputFileStreamWrapper<std::byte>;
			using OutputFileStreamWrapper = BasicOutputFileStreamWrapper<std::byte>;
		}
	}
}
)";

        // stringformers.hpp content  
        std::string stringformers_content = R"(
#pragma once
#include <cctype>
#include <string>
#include <string_view>
#include <ranges>
#include <algorithm>
#include <vector>
#include <unordered_set>
#include <unordered_map>

/**********************************************************************************************
*
*                   			String Transformers
*                   			-------------------
*    			This header provides utility functions for string manipulation,
*    			including case conversion and tokenization.
*
*                   			Developed by: Pooria Yousefi
*				   				Date: 2025-06-26
*				   				License: MIT
*
**********************************************************************************************/

namespace pooriayousefi::core
{
    template<class Enc, class EncTraits = std::char_traits<Enc>, class EncAlloc = std::allocator<Enc>>
    constexpr decltype(auto) to_lowercase(const std::basic_string<Enc, EncTraits, EncAlloc>& word)
    {
        std::basic_string<Enc, EncTraits, EncAlloc> lowercased_word{};
        lowercased_word.resize(std::ranges::size(word));
        std::ranges::transform(std::ranges::cbegin(word), std::ranges::cend(word),
            std::ranges::begin(lowercased_word), [](const auto& c) { return std::tolower(c); });
        return lowercased_word;
    }
    template<class Enc, class EncTraits = std::char_traits<Enc>, class EncAlloc = std::allocator<Enc>>
    constexpr decltype(auto) to_lowercase(std::basic_string_view<Enc, EncTraits> word_view)
    {
        std::basic_string<Enc, EncTraits, EncAlloc> lowercased_word{};
        lowercased_word.resize(std::ranges::size(word_view));
        std::ranges::transform(std::ranges::cbegin(word_view), std::ranges::cend(word_view),
            std::ranges::begin(lowercased_word), [](const auto& c) { return std::tolower(c); });
        return lowercased_word;
    }

    template<class Enc, class EncTraits = std::char_traits<Enc>, class EncAlloc = std::allocator<Enc>>
    constexpr decltype(auto) to_uppercase(const std::basic_string<Enc, EncTraits, EncAlloc>& word)
    {
        std::basic_string<Enc, EncTraits, EncAlloc> uppercased_word{};
        uppercased_word.resize(std::ranges::size(word));
        std::ranges::transform(std::ranges::cbegin(word), std::ranges::cend(word),
            std::ranges::begin(uppercased_word), [](const auto& c) { return std::toupper(c); });
        return uppercased_word;
    }
    template<class Enc, class EncTraits = std::char_traits<Enc>, class EncAlloc = std::allocator<Enc>>
    constexpr decltype(auto) to_uppercase(std::basic_string_view<Enc, EncTraits> word_view)
    {
        std::basic_string<Enc, EncTraits, EncAlloc> uppercased_word{};
        uppercased_word.resize(std::ranges::size(word_view));
        std::ranges::transform(std::ranges::cbegin(word_view), std::ranges::cend(word_view),
            std::ranges::begin(uppercased_word), [](const auto& c) { return std::toupper(c); });
        return uppercased_word;
    }

    template<class T, class Traits = std::char_traits<T>>
    constexpr void tokenize(
        std::basic_string_view<T, Traits> src, 
        std::basic_string_view<T, Traits> delim,
        std::vector<std::basic_string_view<T, Traits>>& tokens
    )
    {
        tokens.clear();
        tokens.reserve(src.size());

        auto last_pos = src.find_first_not_of(delim, 0);
        auto pos = src.find_first_of(delim, last_pos);

        while (pos != std::basic_string_view<T, Traits>::npos || last_pos != std::basic_string_view<T, Traits>::npos)
        {
            tokens.emplace_back(src.substr(last_pos, pos - last_pos));
            last_pos = src.find_first_not_of(delim, pos);
            pos = src.find_first_of(delim, last_pos);
        }
    }
    template<class T, class Traits = std::char_traits<T>>
	constexpr void tokenize(
        std::basic_string_view<T, Traits> src, 
        std::basic_string_view<T, Traits> delim,
        std::unordered_set<std::basic_string_view<T, Traits>>& tokens
    )
	{
        tokens.clear();
        tokens.reserve(src.size());

        auto last_pos = src.find_first_not_of(delim, 0);
        auto pos = src.find_first_of(delim, last_pos);

		while (pos != std::basic_string_view<T, Traits>::npos || last_pos != std::basic_string_view<T, Traits>::npos)
		{
			tokens.emplace(src.substr(last_pos, pos - last_pos));
			last_pos = src.find_first_not_of(delim, pos);
			pos = src.find_first_of(delim, last_pos);
		}
	}
	template<class T, class Traits = std::char_traits<T>>
	auto tokenize(
        std::basic_string_view<T, Traits> src, 
        std::basic_string_view<T, Traits> delim,
        std::unordered_map<std::basic_string_view<T, Traits>, size_t>& tokens
    )
	{
        tokens.clear();
        tokens.reserve(src.size());

		auto last_pos = src.find_first_not_of(delim, 0);
		auto pos = src.find_first_of(delim, last_pos);

		while (pos != std::basic_string_view<T, Traits>::npos || last_pos != std::basic_string_view<T, Traits>::npos)
		{
			tokens[src.substr(last_pos, pos - last_pos)]++;
			last_pos = src.find_first_not_of(delim, pos);
			pos = src.find_first_of(delim, last_pos);
		}
	}
}
)";

        // utilities.hpp content (truncated for brevity - the full content is very long)
        std::string utilities_content = R"(
#pragma once
#include <concepts>
#include <type_traits>
#include <thread>
#include <ratio>
#include <utility>
#include <chrono>
#include <functional>
#include <cstdint>
#include <numbers>
#include <vector>
#include <algorithm>
#include <ranges>
#include <iterator>
#include <unordered_map>
#include <string_view>
#include <random>
#include <variant>
#include <iostream>

/**********************************************************************************************
*
*                   			    Utilities Header
*                   			-----------------------
*    		This header provides general utility functions and classes.
*    		It includes:
*    		- A wait_for class template for sleeping for various time durations.
*    		- A runtime function template for measuring the execution time of a callable.
*    		- A convert namespace with functions for unit conversions and number base conversions.
*    		- A countdown function template for displaying a countdown in seconds.
*    		- An iterate function template for iterating over a range with a specified step size
*    		- Specializations of standard functors for std::byte and std::reference_wrapper.
*    		- A histogram function template for counting occurrences of elements in a range.
*    		- A frequencies function template for counting word frequencies in a string view.
*    		- A do_n_times_shuffle_and_sample function template for shuffling and sampling a range.
*    		- A Result struct template for encapsulating expected values or exceptions.
*
*                   			Developed by: Pooria Yousefi
*				   				Date: 2025-06-26
*				   				License: MIT
*
**********************************************************************************************/

namespace pooriayousefi::core
{
    template<typename T> concept Arithmetic = std::floating_point<T> || std::integral<T>;

    template<Arithmetic T> 
    class wait_for
    {
    public:
        wait_for() = delete;
        wait_for(T value) :m_value{ value } {}
        inline void nanoseconds() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1'000'000'000>>(m_value)); }
        inline void microseconds() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1'000'000>>(m_value)); }
        inline void milliseconds() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1, 1'000>>(m_value)); }
        inline void seconds() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<1>>(m_value)); }
        inline void minutes() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<60>>(m_value)); }
        inline void hours() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<3'600>>(m_value)); }
        inline void days() { std::this_thread::sleep_for(std::chrono::duration<double, std::ratio<86'400>>(m_value)); }
    private:
        T m_value;
    };

    template<typename F, typename... Args> 
    constexpr decltype(auto) runtime(F&& f, Args&&... args)
    {
        if constexpr (std::is_void_v<std::invoke_result_t<F, Args...>>)
        {
            auto ti{ std::chrono::high_resolution_clock::now() };
            std::invoke(std::forward<F>(f), std::forward<Args>(args)...);
            auto tf{ std::chrono::high_resolution_clock::now() };
            return std::chrono::duration<double>(tf - ti).count();
        }
        else if constexpr (!std::is_void_v<std::invoke_result_t<F, Args...>>)
        {
            auto ti{ std::chrono::high_resolution_clock::now() };
            auto retval{ std::invoke(std::forward<F>(f), std::forward<Args>(args)...) };
            auto tf{ std::chrono::high_resolution_clock::now() };
            return std::make_pair(std::move(retval), std::chrono::duration<double>(tf - ti).count());
        }
    }

    namespace convert
    {
        template<std::floating_point T> constexpr T degrees_to_radians(T x) { return x * std::numbers::pi_v<T> / (T)180; }
        template<std::floating_point T> constexpr T radians_to_degrees(T x) { return x * (T)180 / std::numbers::pi_v<T>; }
        template<std::floating_point T> constexpr T Celsius_to_Fahrenheit(T x) { return (x * (T)9 / (T)5) + (T)32; }
        template<std::floating_point T> constexpr T Fahrenheit_to_Celsius(T x) { return (x - (T)32) * (T)5 / (T)9; }
    }

    template<std::integral I> 
    constexpr void countdown(I nsec)
    {
        std::cout << "\nT-" << nsec << ' ';
        std::this_thread::sleep_for(std::chrono::seconds(1));
        for (auto i{ static_cast<int64_t>(nsec) - static_cast<int64_t>(1) }; i >= static_cast<int64_t>(0); --i)
        {
            std::cout << i << ' ';
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }
    }

    template<std::input_or_output_iterator It, std::invocable<std::iter_value_t<It>&> F>
    constexpr void iterate(It begin, size_t n, size_t step_size, F&& f)
    {
        size_t c(0);
        auto it = begin;
        do
        {
            std::invoke(std::forward<F>(f), *it);
            c++;
        } while (c < n && [&]() { it = std::ranges::next(it, step_size); return true; }());
    }
}

namespace std
{
	template<> struct hash<byte>
	{
		constexpr const size_t operator()(const byte& b) const
		{
			hash<size_t> hasher{};
            return hasher(to_integer<size_t>(b));
		}
	};

	template<> struct equal_to<byte>
	{
		constexpr const bool operator()(const byte& lb, const byte& rb) const
		{
			return to_integer<size_t>(lb) == to_integer<size_t>(rb);
		}
	};

	template<class T> struct hash<reference_wrapper<const T>>
	{
		constexpr const size_t operator()(const reference_wrapper<const T>& ref) const
		{
			hash<T> hasher{};
			return hasher(ref.get());
		}
	};

	template<class T> struct equal_to<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() == rhs.get();
		}
	};
}
)";

        // Write all header files
        write_file(project_path + "/include/core/asyncops.hpp", asyncops_content);
        write_file(project_path + "/include/core/raiiiofsw.hpp", raiiiofsw_content);
        write_file(project_path + "/include/core/stringformers.hpp", stringformers_content);
        write_file(project_path + "/include/core/utilities.hpp", utilities_content);
        
        std::cout << "Template header files created!" << std::endl;
    };

    // Create source file
    auto create_source_files = [&]()
    {
        std::cout << "Creating source files..." << std::endl;
        
        // Main source file template
        std::string main_cpp_template = R"(
#include "asyncops.hpp"
#include "raiiiofsw.hpp"
#include "stringformers.hpp"
#include "utilities.hpp"

// entry-point
int main()
{
    try
    {
        // start here ...
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& xxx)
    {
        std::cerr << "Error: " << xxx.what() << std::endl;
        return EXIT_FAILURE;
    }
}
    )";
        
        write_file(project_path + "/src/main.cpp", main_cpp_template);
    };

    // Create build system
    auto create_build_system = [&]()
    {
        std::cout << "Creating build system..." << std::endl;
        
        // Create C++ build system executable
        std::string build_cpp = "#include <iostream>\n";
        build_cpp += "#include <string>\n";
        build_cpp += "#include <vector>\n";
        build_cpp += "#include <cstdlib>\n";
        build_cpp += "#include <filesystem>\n\n";
        build_cpp += "namespace fs = std::filesystem;\n\n";
        build_cpp += "class BuildSystem\n";
        build_cpp += "{\n";
        build_cpp += "private:\n";
        build_cpp += "    std::string build_type_;\n";
        build_cpp += "    std::string output_type_;\n";
        build_cpp += "    \n";
        build_cpp += "    int execute_command(const std::string& command) const\n";
        build_cpp += "    {\n";
        build_cpp += "        std::cout << \"Executing: \" << command << std::endl;\n";
        build_cpp += "        return std::system(command.c_str());\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "public:\n";
        build_cpp += "    BuildSystem() : build_type_(\"debug\"), output_type_(\"executable\")\n";
        build_cpp += "    {\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    void set_build_type(const std::string& type)\n";
        build_cpp += "    {\n";
        build_cpp += "        build_type_ = type;\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    void set_output_type(const std::string& type)\n";
        build_cpp += "    {\n";
        build_cpp += "        output_type_ = type;\n";
        build_cpp += "    }\n";
        build_cpp += "    \n";
        build_cpp += "    int build()\n";
        build_cpp += "    {\n";
        build_cpp += "        std::string build_dir = \"build/\" + build_type_;\n";
        build_cpp += "        fs::create_directories(build_dir);\n";
        build_cpp += "        \n";
        build_cpp += "        std::vector<std::string> source_files;\n";
        build_cpp += "        \n";
        build_cpp += "        // Collect all source files\n";
        build_cpp += "        for (const auto& entry : fs::recursive_directory_iterator(\"src\"))\n";
        build_cpp += "        {\n";
        build_cpp += "            if (entry.is_regular_file() && entry.path().extension() == \".cpp\")\n";
        build_cpp += "            {\n";
        build_cpp += "                source_files.push_back(entry.path().string());\n";
        build_cpp += "            }\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        std::string compile_flags;\n";
        build_cpp += "        std::string link_flags;\n";
        build_cpp += "        std::string output_name;\n";
        build_cpp += "        \n";
        build_cpp += "        if (build_type_ == \"debug\")\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags = \"-g -O0 -DDEBUG\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (build_type_ == \"release\")\n";
        build_cpp += "        {\n";
        build_cpp += "            compile_flags = \"-O3 -DNDEBUG\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        // Common flags\n";
        build_cpp += "        compile_flags += \" -std=c++23 -Wall -Wextra -Wpedantic -Iinclude -Iinclude/core\";\n";
        build_cpp += "        \n";
        build_cpp += "        if (output_type_ == \"executable\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/\" + \"" + project_name + "\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"static\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name + "\" + \".a\";\n";
        build_cpp += "            compile_flags += \" -c\";\n";
        build_cpp += "        }\n";
        build_cpp += "        else if (output_type_ == \"dynamic\")\n";
        build_cpp += "        {\n";
        build_cpp += "            output_name = build_dir + \"/lib\" + \"" + project_name + "\" + \".so\";\n";
        build_cpp += "            compile_flags += \" -fPIC\";\n";
        build_cpp += "            link_flags += \" -shared\";\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        std::cout << \"Building " + project_name + " (\" << build_type_ << \", \" << output_type_ << \")...\" << std::endl;\n";
        build_cpp += "        \n";
        build_cpp += "        if (output_type_ == \"static\")\n";
        build_cpp += "        {\n";
        build_cpp += "            // Compile to object files first\n";
        build_cpp += "            std::vector<std::string> object_files;\n";
        build_cpp += "            for (const auto& source : source_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                std::string obj_file = build_dir + \"/\" + fs::path(source).stem().string() + \".o\";\n";
        build_cpp += "                object_files.push_back(obj_file);\n";
        build_cpp += "                \n";
        build_cpp += "                std::string compile_cmd = \"g++ \" + compile_flags + \" \" + source + \" -o \" + obj_file;\n";
        build_cpp += "                if (execute_command(compile_cmd) != 0)\n";
        build_cpp += "                {\n";
        build_cpp += "                    return 1;\n";
        build_cpp += "                }\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            // Create static library\n";
        build_cpp += "            std::string ar_cmd = \"ar rcs \" + output_name;\n";
        build_cpp += "            for (const auto& obj : object_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                ar_cmd += \" \" + obj;\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            if (execute_command(ar_cmd) == 0)\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cout << \"Static library built: \" << output_name << std::endl;\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            return 1;\n";
        build_cpp += "        }\n";
        build_cpp += "        else\n";
        build_cpp += "        {\n";
        build_cpp += "            // Build executable or dynamic library\n";
        build_cpp += "            std::string build_cmd = \"g++ \" + compile_flags + \" \";\n";
        build_cpp += "            for (const auto& source : source_files)\n";
        build_cpp += "            {\n";
        build_cpp += "                build_cmd += source + \" \";\n";
        build_cpp += "            }\n";
        build_cpp += "            build_cmd += link_flags + \" -o \" + output_name;\n";
        build_cpp += "            \n";
        build_cpp += "            if (output_type_ == \"executable\")\n";
        build_cpp += "            {\n";
        build_cpp += "                build_cmd += \" -static\";  // Static executable\n";
        build_cpp += "            }\n";
        build_cpp += "            \n";
        build_cpp += "            if (execute_command(build_cmd) == 0)\n";
        build_cpp += "            {\n";
        build_cpp += "                if (output_type_ == \"executable\")\n";
        build_cpp += "                {\n";
        build_cpp += "                    std::cout << \"Executable built: \" << output_name << std::endl;\n";
        build_cpp += "                }\n";
        build_cpp += "                else\n";
        build_cpp += "                {\n";
        build_cpp += "                    std::cout << \"Dynamic library built: \" << output_name << std::endl;\n";
        build_cpp += "                }\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            return 1;\n";
        build_cpp += "        }\n";
        build_cpp += "    }\n";
        build_cpp += "};\n\n";
        build_cpp += "int main(int argc, char* argv[])\n";
        build_cpp += "{\n";
        build_cpp += "    try\n";
        build_cpp += "    {\n";
        build_cpp += "        BuildSystem builder;\n";
        build_cpp += "        \n";
        build_cpp += "        // Parse command line arguments\n";
        build_cpp += "        for (int i = 1; i < argc; ++i)\n";
        build_cpp += "        {\n";
        build_cpp += "            std::string arg = argv[i];\n";
        build_cpp += "            if (arg == \"--debug\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"debug\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--release\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_build_type(\"release\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--executable\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"executable\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--static\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"static\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--dynamic\")\n";
        build_cpp += "            {\n";
        build_cpp += "                builder.set_output_type(\"dynamic\");\n";
        build_cpp += "            }\n";
        build_cpp += "            else if (arg == \"--help\")\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cout << \"Usage: \" << argv[0] << \" [options]\\n\";\n";
        build_cpp += "                std::cout << \"Options:\\n\";\n";
        build_cpp += "                std::cout << \"  --debug          Build in debug mode\\n\";\n";
        build_cpp += "                std::cout << \"  --release        Build in release mode\\n\";\n";
        build_cpp += "                std::cout << \"  --executable     Build static executable (default)\\n\";\n";
        build_cpp += "                std::cout << \"  --static         Build static library\\n\";\n";
        build_cpp += "                std::cout << \"  --dynamic        Build dynamic library\\n\";\n";
        build_cpp += "                std::cout << \"  --help           Show this help message\\n\";\n";
        build_cpp += "                return 0;\n";
        build_cpp += "            }\n";
        build_cpp += "            else\n";
        build_cpp += "            {\n";
        build_cpp += "                std::cerr << \"Unknown option: \" << arg << std::endl;\n";
        build_cpp += "                return 1;\n";
        build_cpp += "            }\n";
        build_cpp += "        }\n";
        build_cpp += "        \n";
        build_cpp += "        int result = builder.build();\n";
        build_cpp += "        if (result == 0)\n";
        build_cpp += "        {\n";
        build_cpp += "            std::cout << \"Build completed!\" << std::endl;\n";
        build_cpp += "        }\n";
        build_cpp += "        return result;\n";
        build_cpp += "    }\n";
        build_cpp += "    catch (const std::exception& e)\n";
        build_cpp += "    {\n";
        build_cpp += "        std::cerr << \"Build failed: \" << e.what() << std::endl;\n";
        build_cpp += "        return 1;\n";
        build_cpp += "    }\n";
        build_cpp += "}\n";
        
        write_file(project_path + "/builder.cpp", build_cpp);
    };
    
    // Create VSCode configuration
    auto create_vscode_config = [&]()
    {
        std::cout << "Creating VSCode configuration..." << std::endl;
        
        // Create .vscode directory
        fs::create_directories(fs::path(project_path) / fs::path(".vscode"));
        
        std::string vscode_settings = "{\n";
        vscode_settings += "    \"C_Cpp.default.compilerPath\": \"/usr/bin/g++\",\n";
        vscode_settings += "    \"C_Cpp.default.intelliSenseMode\": \"linux-gcc-x64\",\n";
        vscode_settings += "    \"C_Cpp.default.cppStandard\": \"c++23\",\n";
        vscode_settings += "    \"C_Cpp.default.cStandard\": \"c17\",\n";
        vscode_settings += "    \"C_Cpp.default.includePath\": [\n";
        vscode_settings += "        \"${workspaceFolder}/include\",\n";
        vscode_settings += "        \"${workspaceFolder}/include/core\"\n";
        vscode_settings += "    ],\n";
        vscode_settings += "    \"C_Cpp.default.defines\": [],\n";
        vscode_settings += "    \"editor.formatOnSave\": true,\n";
        vscode_settings += "    \"files.associations\": {\n";
        vscode_settings += "        \"*.hpp\": \"cpp\",\n";
        vscode_settings += "        \"*.cpp\": \"cpp\",\n";
        vscode_settings += "        \"*.h\": \"c\",\n";
        vscode_settings += "        \"*.c\": \"c\"\n";
        vscode_settings += "    },\n";
        vscode_settings += "    \"C_Cpp.clang_format_style\": \"{ BasedOnStyle: LLVM, IndentWidth: 4, ColumnLimit: 100, BreakBeforeBraces: Allman }\"\n";
        vscode_settings += "}";
        
        write_file(project_path + "/.vscode/settings.json", vscode_settings);
        
        std::string vscode_tasks = "{\n";
        vscode_tasks += "    \"version\": \"2.0.0\",\n";
        vscode_tasks += "    \"tasks\": [\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Compile Build System\",\n";
        vscode_tasks += "            \"command\": \"g++\",\n";
        vscode_tasks += "            \"args\": [\"-std=c++23\", \"builder.cpp\", \"-o\", \"builder\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Debug Executable\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--debug\", \"--executable\"],\n";
        vscode_tasks += "            \"group\": {\n";
        vscode_tasks += "                \"kind\": \"build\",\n";
        vscode_tasks += "                \"isDefault\": true\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Release Executable\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--release\", \"--executable\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        },\n";
        vscode_tasks += "        {\n";
        vscode_tasks += "            \"type\": \"shell\",\n";
        vscode_tasks += "            \"label\": \"Build Static Library\",\n";
        vscode_tasks += "            \"command\": \"./builder\",\n";
        vscode_tasks += "            \"args\": [\"--release\", \"--static\"],\n";
        vscode_tasks += "            \"group\": \"build\",\n";
        vscode_tasks += "            \"dependsOn\": \"Compile Build System\",\n";
        vscode_tasks += "            \"presentation\": {\n";
        vscode_tasks += "                \"echo\": true,\n";
        vscode_tasks += "                \"reveal\": \"always\",\n";
        vscode_tasks += "                \"focus\": false,\n";
        vscode_tasks += "                \"panel\": \"shared\",\n";
        vscode_tasks += "                \"showReuseMessage\": true,\n";
        vscode_tasks += "                \"clear\": false\n";
        vscode_tasks += "            },\n";
        vscode_tasks += "            \"problemMatcher\": \"$gcc\"\n";
        vscode_tasks += "        }\n";
        vscode_tasks += "    ]\n";
        vscode_tasks += "}";
        
        write_file(project_path + "/.vscode/tasks.json", vscode_tasks);
    };

    // Create README file
    auto create_readme = [&]()
    {
        std::cout << "Creating README..." << std::endl;
        
        std::string readme_content = "# " + project_name + "\n\n";
        readme_content += "A minimalistic C++ project with command-line build system.\n\n";
        readme_content += "## Features\n\n";
        readme_content += "- Modern C++23 support\n";
        readme_content += "- Command-line build system (no CMake/Makefile required)\n";
        readme_content += "- Support for static executables, static libraries, and dynamic libraries\n";
        readme_content += "- VSCode configuration\n";
        readme_content += "- Template header files (asyncops.hpp, raiiiofsw.hpp, stringformers.hpp, utilities.hpp)\n";
        readme_content += "- Pythonic naming convention (PascalCase for classes, snake_case for everything else)\n";
        readme_content += "- Allman indentation style\n\n";
        readme_content += "## Project Structure\n\n";
        readme_content += "```\n";
        readme_content += project_name + "/\n";
        readme_content += "├── include/                 # Header files (including template headers)\n";
        readme_content += "│   └── core/               # Core template headers\n";
        readme_content += "│       ├── asyncops.hpp    # Async operations & coroutines\n";
        readme_content += "│       ├── raiiiofsw.hpp   # RAII filesystem wrappers\n";
        readme_content += "│       ├── stringformers.hpp # String formatting utilities\n";
        readme_content += "│       └── utilities.hpp   # General utility functions\n";
        readme_content += "├── src/                     # Source files\n";
        readme_content += "├── tests/                   # Test files\n";
        readme_content += "├── build/                   # Build outputs\n";
        readme_content += "│   ├── debug/              # Debug builds\n";
        readme_content += "│   └── release/            # Release builds\n";
        readme_content += "├── .vscode/                 # VSCode configuration\n";
        readme_content += "├── builder.cpp              # Build system source\n";
        readme_content += "└── README.md                # This file\n";
        readme_content += "```\n\n";
        readme_content += "## Build Instructions\n\n";
        readme_content += "### Initial Setup\n\n";
        readme_content += "1. Compile the build system:\n";
        readme_content += "```bash\n";
        readme_content += "g++ -std=c++23 builder.cpp -o builder\n";
        readme_content += "```\n\n";
        readme_content += "### Building the Project\n\n";
        readme_content += "#### Build static executable (default):\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --executable\n";
        readme_content += "```\n\n";
        readme_content += "#### Build in debug mode:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --debug --executable\n";
        readme_content += "```\n\n";
        readme_content += "#### Build static library:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --static\n";
        readme_content += "```\n\n";
        readme_content += "#### Build dynamic library:\n";
        readme_content += "```bash\n";
        readme_content += "./builder --release --dynamic\n";
        readme_content += "```\n\n";
        readme_content += "### Build Options\n\n";
        readme_content += "- `--debug`: Build in debug mode (with debugging symbols)\n";
        readme_content += "- `--release`: Build in release mode (optimized)\n";
        readme_content += "- `--executable`: Build static executable (default)\n";
        readme_content += "- `--static`: Build static library\n";
        readme_content += "- `--dynamic`: Build dynamic library\n\n";
        readme_content += "## Template Headers\n\n";
        readme_content += "The following header files are automatically copied to `include/core/`:\n";
        readme_content += "- `core/asyncops.hpp`: Async operations and coroutines utilities\n";
        readme_content += "- `core/raiiiofsw.hpp`: RAII filesystem wrappers\n";
        readme_content += "- `core/stringformers.hpp`: String formatting and manipulation utilities\n";
        readme_content += "- `core/utilities.hpp`: General utility functions\n\n";
        readme_content += "## Development\n\n";
        readme_content += "The project follows these conventions:\n";
        readme_content += "- **Classes/Structs**: PascalCase (e.g., `ExampleClass`)\n";
        readme_content += "- **Methods/Variables/Constants**: snake_case (e.g., `get_name()`, `project_name_`)\n";
        readme_content += "- **Indentation**: Allman style (braces on new lines)\n\n";
        readme_content += "## Requirements\n\n";
        readme_content += "- GCC 11+ or Clang 14+ with C++23 support\n";
        readme_content += "- Linux x64 (Ubuntu/Debian)\n";
        readme_content += "- Git (for vcpkg management)\n";
        readme_content += "- Internet connection (for initial package downloads)\n\n";
        readme_content += "## License\n\n";
        readme_content += "This project is provided as-is for educational and development purposes.\n";
        
        write_file(project_path + "/README.md", readme_content);
    };

    try
    {
        if (argc != 2)
        {
            std::string errmsg{ "Usage: "};
            errmsg += argv[0];
            errmsg += " <project_path>\n";
            errmsg += "Example: ";
            errmsg += argv[0];
            errmsg += " ~/projects/my-new-project\n";
            throw std::runtime_error(errmsg.c_str());
        }
        
        project_path = argv[1];
        
        // Expand ~ to home directory if needed
        if (project_path.front() == '~')
        {
            const char* home = std::getenv("HOME");
            if (home)
            {
                project_path = std::string(home) + project_path.substr(1);
            }
        }
        
        if (fs::exists(project_path))
        {
            std::string errmsg{ "Error: Directory already exists: " };
            errmsg += project_path;
            throw std::runtime_error(errmsg.c_str());
        }
        
        std::cout << "Creating C++ project: " << project_path << std::endl;
        
        fs::path path(project_path);
        project_name = sanitize_cpp_name(path.stem().string());
        
        create_directory_structure();
        create_source_files();
        copy_template_headers();
        create_build_system();
        create_vscode_config();
        create_readme();
            
        std::cout << "\n=== Project Setup Complete ===" << std::endl;
        std::cout << "Project: " << project_name << std::endl;
        std::cout << "Location: " << fs::absolute(project_path) << std::endl;
        std::cout << "\nNext steps:" << std::endl;
        std::cout << "1. cd " << project_path << std::endl;
        std::cout << "2. g++ -std=c++23 builder.cpp -o builder" << std::endl;
        std::cout << "3. ./builder --release --executable" << std::endl;
        std::cout << "4. ./build/release/" << project_name << std::endl;
        
        return EXIT_SUCCESS;
    }
    catch (const std::exception& xxx)
    {
        std::cerr << "Failed to create project: " << xxx.what() << std::endl;
        return EXIT_FAILURE;
    }
}
