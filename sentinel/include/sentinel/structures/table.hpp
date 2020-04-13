#ifndef SENTINEL__STRUCTURES__TABLE_HPP
#define SENTINEL__STRUCTURES__TABLE_HPP

#include <cstddef> // offsetof
#include <cstdint> // std::uintptr_t

#include <iterator>
#include <type_traits> // std::conditional

#include <boost/iterator/filter_iterator.hpp>

#include <sentinel/types.hpp>
#include <sentinel/base.hpp>
#include <sentinel/traits.hpp>
#include <sentinel/fwd/table_fwd.hpp>

namespace sentinel {

template<class DatumType>
struct table_type {
private:
    static inline constexpr bool is_generic = std::is_same_v<void, DatumType>;
    static_assert(is_generic || traits::is_table_element<DatumType>::value,
                  "DatumType must be void or have member salt of type identity_salt");
    static inline constexpr auto is_datum_initialized
        = [] ([[maybe_unused]] const auto& datum)
            {
                if constexpr (is_generic) return true;
                else                      return datum.salt != 0;
            };

public:
    using value_type      = std::conditional_t<is_generic, unsigned char, DatumType>;
    using reference       = value_type&;
    using const_reference = const value_type&;
    using pointer         = value_type*;
    using const_pointer   = const value_type*;

    h_char        name[32];     ///< A null-terminated string denoting the table's purpose.
    int16         capacity;     ///< The maximum number of elements that can be assigned to the table.
    int16         element_size; ///< The stride between elements in the array. Confirmed as signed.
    boolean       initialized;  ///< Set to `true` if and only if the table is in a valid state for use.
    signature     sig_data;     ///< `"d@t@"_hsig`
    index_short   index_lfree;  ///< An inclusive lower bound on the first free element index.
    index_short   index_end;    ///< The `end` index in 1-past-end fashion for indices into the data array.
    int16         count;        ///< The number of elements active in the table array.
    identity_salt next_salt;    ///< The next ID to use when allocating a new element.
    pointer       array;        ///< Pointer to the array allocated for this table.

    auto begin() noexcept { return impl_iterator(*this, 0); }
    auto end()   noexcept { return impl_iterator(*this, index_end); }

    auto cbegin() const noexcept { return impl_iterator(*this, 0); }
    auto cend()   const noexcept { return impl_iterator(*this, index_end); }

    auto begin() const noexcept { return impl_iterator(*this, 0); }
    auto end()   const noexcept { return impl_iterator(*this, index_end); }

    reference       operator[](const int index)       { return array[index]; }
    const_reference operator[](const int index) const { return array[index]; }

private:
    std::conditional_t<is_generic, strided_iterator<pointer>, pointer>
    array_pointer() noexcept
    {
        pointer p = array;
        if constexpr (is_generic) return strided_iterator(p, element_size);
        else                      return p;
    }

    std::conditional_t<is_generic, strided_iterator<const_pointer>, const_pointer>
    array_pointer() const noexcept
    {
        const_pointer p = array;
        if constexpr (is_generic) return strided_iterator(p, element_size);
        else                      return p;
    }

    template<class Table, class Distance>
    static auto impl_iterator(Table& table,
                              const Distance& distance) noexcept
    {
        return boost::filter_iterator(is_datum_initialized,
                                      table.array_pointer() + distance,
                                      table.array_pointer() + table.index_end);
    }
}; static_assert(sizeof(table_type<void>) == 0x38);

#define SENTINEL_MAKE

#ifndef SENTINEL_SKIP_OFFSET_ASSERTIONS
static_assert(offsetof(table_type<void>, name) == 0x00);
static_assert(offsetof(table_type<void>, capacity) == 0x20);
static_assert(offsetof(table_type<void>, element_size) == 0x22);
static_assert(offsetof(table_type<void>, initialized) == 0x24);
static_assert(offsetof(table_type<void>, sig_data) == 0x28);
static_assert(offsetof(table_type<void>, index_lfree) == 0x2C);
static_assert(offsetof(table_type<void>, index_end) == 0x2E);
static_assert(offsetof(table_type<void>, count) == 0x30);
static_assert(offsetof(table_type<void>, next_salt) == 0x32);
static_assert(offsetof(table_type<void>, array) == 0x34);
#endif // SENTINEL_SKIP_OFFSET_ASSERTIONS

}

#endif // SENTINEL__STRUCTURES__TABLE_HPP
