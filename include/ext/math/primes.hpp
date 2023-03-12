// Copyright - 2015 - Jan Christoph Uhde <Jan@UhdeJC.com>
// TODO enable_if for integral types
#ifndef EXT_MATH_PRIMES_HEADER
#define EXT_MATH_PRIMES_HEADER

#include <limits>
#include <stdexcept>
#include <utility>
#include <vector>
#include <iterator>

#include <ext/macros/compiler.hpp>

namespace ext { namespace math {

template<typename T, bool debug = false>
std::vector<T> prime_factors_naive(T x) {
    std::vector<T> rv;
    // get rid of the half
    while (x % T(2) == 0) {
        x /= T(2);
        rv.emplace_back(2);
    }
    for (T i(3); i <= x; i += T(2)) {
        if (debug && i % T(1000001) == 0) {
            // include debug where you use it
            std::cerr << "at: " << i << std::endl;
        }
        while (x % i == 0) {
            x /= i;
            rv.push_back(i);
        }
    }
    return rv;
}


inline std::ptrdiff_t index_of_value(std::size_t target_value) {
    return (target_value - 3 ) / 2;
}

inline std::size_t value_at_index(std::size_t index) {
    return 2 * index + 3;
}

template<std::random_access_iterator Iter, std::integral N>
requires std::integral<typename Iter::value_type>
void mark_sieve(Iter begin, Iter end, N factor) {
    using value_type = typename Iter::value_type;
    *begin = value_type(0);
    while (end - begin > factor) {
        begin += factor;
        *begin = value_type(0);
    };
}

template<std::random_access_iterator T>
requires std::is_integral_v<typename T::value_type> && std::is_unsigned_v<typename T::value_type>
typename std::size_t sift(T begin, T end) {
    using int_type = std::ptrdiff_t;
    using value_type = typename T::value_type;

    std::fill(begin, end, value_type(true));

    int_type n = std::distance(begin, end);
    int_type i(0), index_square(3), factor(3);

    while (index_square < n) {
        if (begin[i]) {
            mark_sieve(begin + index_square, end, factor);
        }

        ++i;
        index_square += factor;
        factor += 2;
        index_square += factor;
    }

    return n > 0 ? value_at_index(std::size_t(n-1)) : 2;
}

template<std::forward_iterator Iter, typename BackInserter>
void read_out_sieve(Iter begin, Iter end, std::back_insert_iterator<BackInserter> back) {
    using int_type = std::int64_t;
    using value_type = typename BackInserter::value_type;

    int_type i(0);
    back = value_type(2);

    while (begin < end) {
        if(*begin) {
            back = static_cast<value_type>(value_at_index(i));
        }
        ++i;
        ++begin;
    }
}

template<std::random_access_iterator Iter>
requires std::is_integral_v<typename Iter::value_type> && std::is_unsigned_v<typename Iter::value_type>
inline bool is_prime(std::size_t x, Iter begin, Iter end) {
    if(begin ==end || x > value_at_index(std::distance(begin,end) -1 )) {
        throw std::runtime_error("number not covered by range of sieve");
    }

    if (x < 2) { return false; }
    if (x == 2) { return true; }
    if (x % 2 == 0) { return false; }

    return *(begin + index_of_value(x));
}

template<std::random_access_iterator Iter>
requires std::is_integral_v<typename Iter::value_type> && std::is_unsigned_v<typename Iter::value_type>
std::size_t find_nth_prime(std::size_t n, Iter begin, Iter end) {
    if (n == 1) return 2;
    std::size_t count = 1;
    for (auto current = begin; current != end; ++current) {
        if (*current) {
            ++count;
            if (count == n) {
                return value_at_index(std::size_t(std::distance(begin, current)));
            }
        }
    }
    return 0;
}


template<std::random_access_iterator Iter>
requires std::is_integral_v<typename Iter::value_type> && std::is_unsigned_v<typename Iter::value_type>
std::size_t find_next_prime(std::size_t prime, Iter begin, Iter end) {
    if( prime == 2) return 3;
    std::size_t count = 0;
    for (auto current = begin+index_of_value(prime); current != end; ++current) {
        if (*current) {
            if (++count == 2) {
                return value_at_index(std::size_t(std::distance(begin, current)));
            }

        }

    }
    return 0;
}


#if defined(EXT_COMPILER_GCC) && false // disabled
template<std::size_t N>
struct sieve_of_eratosthenes_compile_time {
    std::array<bool, N> sieve; //!< the sieve
    std::size_t max_value;     //!< primes are calculated up to this number
    constexpr sieve_of_eratosthenes_compile_time() : sieve(), max_value(N) {}
};

template<std::size_t max_value>
inline constexpr auto create_sieve_of_eratosthenes() -> sieve_of_eratosthenes_compile_time<(max_value >> 1)> {
    std::size_t constexpr sieve_size = max_value >> 1;

    sieve_of_eratosthenes_compile_time<sieve_size> result;
    for (auto& e : result.sieve) {
        e = true;
    }
    result.max_value = max_value;

    auto& sieve = result.sieve;

    std::size_t current_pos = 1;
    while (current_pos <= std::size_t(std::sqrt(sieve_size)) + 1) {
        while (sieve[current_pos] == false && current_pos < sieve_size) {
            ++current_pos;
        }

        std::size_t distance = current_pos * 2 + 1;
        for (std::size_t i = current_pos + distance; i < sieve_size; i += distance) {
            if (i >= sieve_size || i < current_pos) {
                break;
            }
            sieve[i] = false;
        }
        ++current_pos;
    }
    return result;
}
#endif // EXT_COMPILER_GCC

}}     // namespace ext::math
#endif // EXT_MATH_PRIMES_HEADER
