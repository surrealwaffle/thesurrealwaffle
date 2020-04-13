#ifndef SIMULA__VECTOR_EXTENSIONS_HPP
#define SIMULA__VECTOR_EXTENSIONS_HPP

#include <sentinel/numeric_types.hpp>

struct real3x3{
    sentinel::real3 columns[3];
};

constexpr sentinel::real3 cross(sentinel::real3 const& u, sentinel::real3 const& v) {
    return {
        u[1]*v[2] - u[2]*v[1],
        u[2]*v[0] - u[0]*v[2],
        u[0]*v[1] - u[1]*v[0]
    };
}

constexpr sentinel::real3 add(sentinel::real3 const& u, sentinel::real3 const& v) {
    return {u[0] + v[0], u[1] + v[1], u[2] + v[2]};
}

constexpr sentinel::real3 sub(sentinel::real3 const& u, sentinel::real3 const& v) {
    return {u[0] - v[0], u[1] - v[1], u[2] - v[2]};
}

constexpr sentinel::real3 mul(sentinel::real const a, sentinel::real3 const& u) {
    return {a * u[0], a * u[1], a * u[2]};
}

constexpr sentinel::real dot(sentinel::real3 const& u, sentinel::real3 const& v) {
    return u[0] * v[0] + u[1] * v[1] + u[2] * v[2];
}

constexpr real3x3 transpose(real3x3 const& matrix) {
    return {
        sentinel::real3{matrix.columns[0][0], matrix.columns[1][0], matrix.columns[2][0]},
        sentinel::real3{matrix.columns[0][1], matrix.columns[1][1], matrix.columns[2][1]},
        sentinel::real3{matrix.columns[0][2], matrix.columns[1][2], matrix.columns[2][2]}
    };
}
constexpr sentinel::real3 mul(real3x3 const& matrix, sentinel::real3 u) {
    return add(mul(u[0], matrix.columns[0]), add(mul(u[1], matrix.columns[1]), mul(u[2], matrix.columns[2])));
}

#endif // SIMULA__VECTOR_EXTENSIONS_HPP
