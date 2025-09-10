
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

// anonymous namespace
namespace
{
    // concept to constrain arithmetic types
    template<typename T> concept Arithmetic = std::floating_point<T> || std::integral<T>;

    // wait for some times in different time units
    export template<Arithmetic T> 
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

    // runtime calculator
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

    // conversion utilities
    namespace convert
    {
        template<std::floating_point T> constexpr T degrees_to_radians(T x) { return x * std::numbers::pi_v<T> / (T)180; }
        template<std::floating_point T> constexpr T radians_to_degrees(T x) { return x * (T)180 / std::numbers::pi_v<T>; }
        template<std::floating_point T> constexpr T Celsius_to_Fahrenheit(T x) { return (x * (T)9 / (T)5) + (T)32; }
        template<std::floating_point T> constexpr T Fahrenheit_to_Celsius(T x) { return (x - (T)32) * (T)5 / (T)9; }
        template<std::integral I> 
        std::vector<uint8_t> decimal_to_binary(I n)
        {
            std::vector<uint8_t> binval{};
            binval.reserve(64);
            if (n == (I)0)
                binval.emplace_back('0');
            else
            {
                while (n != (I)0)
                {
                    binval.emplace_back(static_cast<uint8_t>(n % (I)2));
                    n /= (I)2;
                }
                binval.emplace_back('b');
                binval.emplace_back('0');
                std::ranges::reverse(std::ranges::begin(binval), std::ranges::end(binval));
            }
            return binval;
        }
        template<std::integral I> 
        std::vector<char> decimal_to_hexadecimal(I n)
        {
            std::vector<char> hexval{};
            hexval.reserve(64);
            if (n == (I)0)
            {
                hexval.emplace_back('0');
                hexval.emplace_back('x');
                hexval.emplace_back('0');
            }
            else
            {
                while (n != (I)0)
                {
                    auto rem{ n % (I)16 };
                    switch (rem)
                    {
                    case (I)10:
                        hexval.emplace_back('A');
                        break;
                    case (I)11:
                        hexval.emplace_back('B');
                        break;
                    case (I)12:
                        hexval.emplace_back('C');
                        break;
                    case (I)13:
                        hexval.emplace_back('D');
                        break;
                    case (I)14:
                        hexval.emplace_back('E');
                        break;
                    case (I)15:
                        hexval.emplace_back('F');
                        break;
                    default:
                        break;
                    }
                    n /= (I)16;
                }
                hexval.emplace_back('x');
                hexval.emplace_back('0');
                std::ranges::reverse(std::ranges::begin(hexval), std::ranges::end(hexval));
            }
            return hexval;
        }
    }

    // countdown function
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

    // iterate through a range step sized
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

// standard namespace
namespace std
{
	// hash functor specialization for byte
	template<> struct 
    hash<byte>
	{
		constexpr const size_t operator()(const byte& b) const
		{
			return to_integer<size_t>(b);
		}
	};

	// equality functor specialization for byte
	template<> struct 
    equal_to<byte>
	{
		constexpr const bool operator()(const byte& lb, const byte& rb) const
		{
			return to_integer<size_t>(lb) == to_integer<size_t>(rb);
		}
	};

	// hash functor specialization for references/const references
	template<class T> 
    struct hash<reference_wrapper<const T>>
	{
		constexpr const size_t operator()(const reference_wrapper<const T>& ref) const
		{
			hash<T> hasher{};
			return hasher(ref.get());
		}
	};

	// equality functor specialization for references/const references
	template<class T> struct 
    equal_to<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() == rhs.get();
		}
	};

	// less than functor specialization for references/const references
	template<class T> 
    struct less<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() < rhs.get();
		}
	};

	// greater than functor specialization for references / const references
	template<class T> 
    struct greater<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() > rhs.get();
		}
	};

	// less than equal to functor specialization for references/const references
	template<class T> 
    struct less_equal<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() <= rhs.get();
		}
	};

	// greater than equal to functor specialization for references/const references
	template<class T> 
    struct greater_equal<reference_wrapper<const T>>
	{
		constexpr const bool operator()(const reference_wrapper<const T>& lhs, const reference_wrapper<const T>& rhs) const
		{
			return lhs.get() >= rhs.get();
		}
	};
}

// anonymous namespace for internal linkage
namespace
{
    // histogram of a range elements
    template<std::ranges::range Rng> 
    constexpr decltype(auto) histogram(const Rng& rng)
    {
        std::unordered_map<
            std::reference_wrapper<const std::ranges::range_value_t<Rng>>,
            size_t,
            std::hash<std::reference_wrapper<const std::ranges::range_value_t<Rng>>>,
            std::equal_to<std::reference_wrapper<const std::ranges::range_value_t<Rng>>>,
            std::allocator<std::pair<const std::reference_wrapper<const std::ranges::range_value_t<Rng>>, size_t>>
        > h{};
        for (const auto& elem : rng)
            h[std::cref(elem)]++;
        return h;
    }

    template<std::input_or_output_iterator It> 
    constexpr decltype(auto) histogram(It beg, It end)
    {
        std::unordered_map<
            std::reference_wrapper<const std::iter_value_t<It>>,
            size_t,
            std::hash<std::reference_wrapper<const std::iter_value_t<It>>>,
            std::equal_to<std::reference_wrapper<const std::iter_value_t<It>>>,
            std::allocator<std::pair<const std::reference_wrapper<const std::iter_value_t<It>>, size_t>>
        > h{};
        std::ranges::for_each(beg, end, [&](const auto& elem) { h[std::cref(elem)]++; });
        return h;
    }

    template<class Enc, class EncTraits = std::char_traits<Enc>> 
    constexpr decltype(auto) frequencies(
        std::basic_string_view<Enc, EncTraits> query,
        std::basic_string_view<Enc, EncTraits> delimiters
    )
    {
        std::unordered_map<
            std::basic_string_view<Enc, EncTraits>,
            size_t,
            std::hash<std::basic_string_view<Enc, EncTraits>>,
            std::equal_to<std::basic_string_view<Enc, EncTraits>>,
            std::allocator<std::pair<const std::basic_string_view<Enc, EncTraits>, size_t>>
        > freqs{};

        // skip delimiters at begining
        auto last_pos = query.find_first_not_of(delimiters, 0);

        // find first non-delimiter
        auto pos = query.find_first_of(delimiters, last_pos);

        while (pos != std::basic_string_view<Enc, EncTraits>::npos || last_pos != std::basic_string_view<Enc, EncTraits>::npos)
        {
            // find a token, add it to the vector
            freqs[query.substr(last_pos, pos - last_pos)]++;

            // skip delimiters
            last_pos = query.find_first_not_of(delimiters, pos);

            // find next non-delimiter
            pos = query.find_first_of(delimiters, last_pos);
        }

        return freqs;
    }

    // shuffle a range and sample it n times
    template<std::input_or_output_iterator It>
    constexpr std::vector<std::iter_value_t<It>> do_n_times_shuffle_and_sample(It beg, It end, size_t n)
    {
        std::random_device rd{};
        std::mt19937_64 random_number_generator{ rd() };
        std::vector<std::iter_value_t<It>> samples(n);
        auto it{ std::ranges::begin(samples) };
        while (it != std::ranges::end(samples))
        {
            std::ranges::shuffle(beg, end, random_number_generator);
            std::ranges::sample(beg, end, it, 1, random_number_generator);
            it++;
        }
        return samples;
    }

    // exception carrier result type
    template<typename Expected>
    struct Result
    {
        using type = std::variant<std::monostate, Expected, std::exception_ptr>;
    };
    template<>
    struct Result<void> 
    {
        using type = std::variant<std::monostate, std::exception_ptr>;
    };
}