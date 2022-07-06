#pragma once
// Credits : Stack overflow - HolyBlackCat answer : https://stackoverflow.com/questions/1055452/c-get-name-of-type-in-template

#include <algorithm>
#include <array>
#include <cstddef>
#include <string_view>

namespace impl
{
    // TODO When, and if, flecs expose it, use its function to deduce type name.
#ifndef _MSC_VER
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wlanguage-extension-token"
#endif
    template <typename T>
    [[nodiscard]] constexpr std::string_view RawTypeName()
    {
        #ifndef _MSC_VER
        return __PRETTY_FUNCTION__;
        #else
        return __FUNCSIG__;
        #endif
    }
#ifndef _MSC_VER
#pragma GCC diagnostic pop
#endif

    struct TypeNameFormat
    {
        std::size_t junk_leading = 0;
        std::size_t junk_total = 0;
    };

    constexpr TypeNameFormat type_name_format = []{
        TypeNameFormat ret;
        std::string_view sample = RawTypeName<int>();
        ret.junk_leading = sample.find("int");
        ret.junk_total = sample.size() - 3;
        return ret;
    }();
    static_assert(type_name_format.junk_leading != std::size_t(-1), "Unable to determine the type name format on this compiler.");

    template <typename T>
    static constexpr auto type_name_storage = []{
        std::array<char, RawTypeName<T>().size() - type_name_format.junk_total + 1> ret{};
        std::copy_n(RawTypeName<T>().data() + type_name_format.junk_leading, ret.size() - 1, ret.data());
        return ret;
    }();
}

template <typename T>
[[nodiscard]] constexpr std::string_view type_name()
{
    return {impl::type_name_storage<T>.data(), impl::type_name_storage<T>.size() - 1};
}

template <typename T>
[[nodiscard]] constexpr const char * type_name_cstr()
{
    return impl::type_name_storage<T>.data();
}
