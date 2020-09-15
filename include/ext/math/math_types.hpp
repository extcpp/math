// Copyright - 2020 - Jan Christoph Uhde <Jan@UhdeJC.com>
// Please see LICENSE.md for license or visit https://github.com/extcpp/basics
#ifndef EXT_MATH_MATH_TYPES_HEADER
#define EXT_MATH_MATH_TYPES_HEADER
#include <algorithm>
#include <array>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <vector>

#include <ext/macros/assert.hpp>


// TODO - find better name for classes and file
//      - implement multiplication and addition so it is more than an array
//      - implement vec using matrix

namespace ext { namespace math {

template<typename T, std::size_t N>
struct vec {
    using value_type = T;
    static constexpr std::size_t size = N;

    std::array<T, N> data;

    vec(vec const&) = default;

    template<typename... Type,
             typename = std::enable_if_t<!(std::is_same_v<vec<T, N>, std::decay_t<Type>> && ...)>>
    vec(Type&&... f) : data{std::forward<Type>(f)...} {}

    //vec(std::initializer_list<T> list) {


    //}

    vec(std::vector<T> const& v) {
        EXT_ASSERT(N == v.size());
        std::copy(v.begin(), v.end(), data.begin());
    }

    static vec zero() {
        auto rv = vec();
        std::memset(&rv.data, 0, sizeof rv.data);
        return rv;
    }

    T& get_x() {
        static_assert(N > 0);
        return this->data[0];
    }

    T& get_y() {
        static_assert(N > 1);
        return this->data[1];
    }

    T& get_z() {
        static_assert(N > 2);
        return this->data[2];
    }

    T const& get_x() const {
        static_assert(N > 0);
        return this->data[0];
    }

    T const& get_y() const {
        static_assert(N > 1);
        return this->data[1];
    }

    T const& get_z() const {
        static_assert(N > 2);
        return this->data[2];
    }

    T& operator[](std::size_t index) {
        return data[index];
    }

    T const& operator[](std::size_t index) const {
        return data[index];
    }

    std::vector<value_type> to_vector() const {
        std::vector<value_type> v(size);
        std::copy(data.begin(), data.end(), v.begin());
        return v;
    }
};

template<typename X, typename... XS>
vec(X, XS... ) -> vec<X, sizeof...(XS) + 1>;

template<typename T, std::size_t N>
vec<T, N> operator-(vec<T, N> const& left, vec<T, N> const& right) {
    auto rv = left;
    for (std::size_t i = 0; i < N; ++i) {
        rv[i] -= right[i];
    }
    return rv;
}

template<typename T, std::size_t N>
vec<T, N> operator+(vec<T, N> const& left, vec<T, N> const& right) {
    auto rv = left;
    for (std::size_t i = 0; i < N; ++i) {
        rv[i] += right[i];
    }
    return rv;
}

template<typename T, std::size_t N>
bool operator==(vec<T, N> const& left, vec<T, N> const& right) {
    for (std::size_t i = 0; i < N; ++i) {
        if(left[i] != right[i]){
            return false;
        }
    }
    return true;
}

template<typename T, std::size_t N>
bool operator!=(vec<T, N> const& left, vec<T, N> const& right) {
    return !(left == right);
}

namespace _detail {

// TODO - use enum for options
template<typename Container, bool row_major, bool checked>
constexpr auto const& matrix_get(Container const& cont,
                                 std::size_t const& column_count,
                                 std::size_t const& row_count,
                                 std::size_t i,
                                 std::size_t j) {
    if constexpr (checked) {
        if (i >= row_count || j >= column_count) {
            throw std::runtime_error("out of range");
        }
    }

    if constexpr (row_major) {
        return cont.data[j + column_count * i];
    } else {
        return cont.data[i + row_count * j];
    }
}

template<typename Container, bool row_major, bool checked, typename T>
constexpr auto const& matrix_set(Container& cont,
                                 std::size_t const& column_count,
                                 std::size_t const& row_count,
                                 std::size_t i,
                                 std::size_t j,
                                 T&& value) {
    if constexpr (checked) {
        if (i >= row_count || j >= column_count) {
            throw std::runtime_error("out of range");
        }
    }

    if constexpr (row_major) {
        return cont.data[j + column_count * i] = std::forward<T>(value);
    } else {
        return cont.data[i + row_count * j] = std::forward<T>(value);
    }
}

} // namespace _detail

// https://simple.wikipedia.org/wiki/Matrix_(mathematics)#/media/File:Matrix.svg


template<typename T, bool row_major = true, bool checked = false>
struct dynamic_matrix {
    using value_type = T;
    std::size_t const row_count;    // j
    std::size_t const column_count; // i
    std::vector<T> data = {};

    dynamic_matrix(std::size_t m = 1, std::size_t n = 1) : row_count(m), column_count(n), data(n * m) {}

    void init_all_to(value_type const& value) {
        std::fill(data.begin(), data.end(), value);
    }

    T const& get(std::size_t i, std::size_t j) const {
        return _detail::matrix_get<dynamic_matrix, row_major, checked>(*this, column_count, row_count, i, j);
    }

    T& get_mutable(std::size_t i, std::size_t j) {
        T const& rv = _detail::matrix_get<dynamic_matrix, row_major, checked>(*this, column_count, row_count, i, j);
        return const_cast<T&>(rv);
    }

    T const& set(std::size_t i, std::size_t j, T const& value) {
        return _detail::matrix_set<dynamic_matrix, row_major, checked>(*this, column_count, row_count, i, j, value);
    }

    T const& set(std::size_t i, std::size_t j, T&& value) {
        return _detail::matrix_set<dynamic_matrix, row_major, checked>(
            *this, column_count, row_count, i, j, std::move(value));
    }
};


// M rows - N cloumns
template<typename T, std::size_t M, std::size_t N, bool row_major = true, bool checked = false>
struct static_matrix {
    using value_type = T;
    constexpr static std::size_t column_count = N; // j
    constexpr static std::size_t row_count = M;    // i

    std::array<T, row_count * column_count> data;

    void init_all_to(value_type const& value) {
        std::fill(data.begin(), data.end(), value);
    }

    T const& get(std::size_t i, std::size_t j) const {
        return _detail::matrix_get<static_matrix, row_major, checked>(*this, column_count, row_count, i, j);
    }

    T& get_mutable(std::size_t i, std::size_t j) {
        T const& rv = _detail::matrix_get<dynamic_matrix, row_major, checked>(*this, column_count, row_count, i, j);
        return const_cast<T&>(rv);
    }


    T const& set(std::size_t i, std::size_t j, T const& value) {
        return _detail::matrix_set<static_matrix, row_major, checked>(*this, column_count, row_count, i, j, value);
    }

    T const& set(std::size_t i, std::size_t j, T&& value) {
        return _detail::matrix_set<static_matrix, row_major, checked>(
            *this, column_count, row_count, i, j, std::move(value));
    }
};

namespace _detail {
template<typename T>
inline std::ostream& to_stream(std::ostream& os, T const& matrix, int width = 5) {
    for (std::size_t i = 0; i < matrix.row_count; i++) {
        for (std::size_t j = 0; j < matrix.column_count; j++) {
            os << std::setw(width) << matrix.get(i, j) << " ";
        }
        os << "\n";
    }


    return os;
}
} // namespace _detail

template<typename T, bool row_major, bool checked>
inline std::ostream& operator<<(std::ostream& os, dynamic_matrix<T, row_major, checked> const& matrix) {
    return _detail::to_stream(os, matrix);
}

template<typename T, std::size_t M, std::size_t N, bool row_major, bool checked>
inline std::ostream& operator<<(std::ostream& os, static_matrix<T, M, N, row_major, checked> const& matrix) {
    return _detail::to_stream(os, matrix);
}

}}     // namespace ext::math
#endif // EXT_MATH_MATH_TYPES_HEADER
