#pragma once

#include <new>     // std::launder
#include <utility> // std:move

namespace sentinel { namespace detail {

struct basic_inplace_operations_traits {
    using dtor_proc = void(*)(void*);
    using move_proc = void(*)(void* dst, void* src);

    dtor_proc dtor;
    move_proc move;
};

template<class T>
inline constexpr basic_inplace_operations_traits basic_inplace_ops = {
    +[] (void* d) { std::launder(reinterpret_cast<T*>(d))->~T(); },
    +[] (void* dst, void* src)
        { new(dst) T(std::move(*std::launder(reinterpret_cast<T*>(src)))); }
};

} } // namespace sentinel::detail
