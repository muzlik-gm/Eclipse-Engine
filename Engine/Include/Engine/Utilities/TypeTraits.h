#pragma once

/**
 * @file TypeTraits.h
 * @brief Extended type-trait utilities beyond those in <type_traits>.
 *
 * Provides detection traits for string-like and container types, a
 * convenience alias for removing cv-qualifiers and references, and the
 * Overloaded helper pattern for visiting std::variant with multiple lambdas.
 */

#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace engine::util
{

    // ========================================================================
    // IsStringLike — detects types that behave like strings.
    // ========================================================================

    /// Primary template: not string-like by default.
    template <typename T>
    struct IsStringLike : std::false_type
    {
    };

    /// Specialization for std::string (including cv-qualified variants).
    template <>
    struct IsStringLike<std::string> : std::true_type
    {
    };

    /// Specialization for std::string_view.
    template <>
    struct IsStringLike<std::string_view> : std::true_type
    {
    };

    /// Specialization for C-style string literals (const char*).
    template <>
    struct IsStringLike<const char*> : std::true_type
    {
    };

    /// Specialization for non-const C-style strings (char*).
    template <>
    struct IsStringLike<char*> : std::true_type
    {
    };

    /// Specialization for char arrays (captures string literals by reference).
    template <std::size_t N>
    struct IsStringLike<char[N]> : std::true_type
    {
    };

    /// Specialization for const char arrays.
    template <std::size_t N>
    struct IsStringLike<const char[N]> : std::true_type
    {
    };

    /// Variable template shorthand for IsStringLike<T>::value.
    template <typename T>
    inline constexpr bool IsStringLikeV = IsStringLike<T>::value;

    // ========================================================================
    // IsContainer — SFINAE-based detection of container types.
    // ========================================================================

    namespace detail
    {
        template <typename T, typename = void>
        struct IsContainerImpl : std::false_type
        {
        };

        template <typename T>
        struct IsContainerImpl<T, std::void_t<
            decltype(std::declval<T&>().begin()),
            decltype(std::declval<T&>().end()),
            typename T::value_type
        >> : std::true_type
        {
        };
    } // namespace detail

    /// True when @p T provides begin(), end(), and a nested value_type —
    /// i.e. it models a basic iterable container.  Explicitly excludes
    /// std::string and std::string_view to avoid surprising matches.
    template <typename T>
    struct IsContainer : detail::IsContainerImpl<T>
    {
    };

    /// Prevent std::string from being detected as a generic container.
    template <>
    struct IsContainer<std::string> : std::false_type
    {
    };

    /// Prevent std::string_view from being detected as a generic container.
    template <>
    struct IsContainer<std::string_view> : std::false_type
    {
    };

    /// Variable template shorthand for IsContainer<T>::value.
    template <typename T>
    inline constexpr bool IsContainerV = IsContainer<T>::value;

    // ========================================================================
    // RemoveCVRef — strips cv-qualifiers and references in one step.
    // ========================================================================

    /// Equivalent to std::remove_cv_t<std::remove_reference_t<T>>.
    /// Useful for deducing the underlying type of forwarding references.
    template <typename T>
    using RemoveCVRef = std::remove_cv_t<std::remove_reference_t<T>>;

    // ========================================================================
    // Overloaded — build a single visitor from multiple callables.
    // ========================================================================

    /// Inherit from a pack of callable types and bring all their
    /// operator() overloads into scope.  Primary use-case is with
    /// std::visit:
    ///
    /// @code
    ///   std::visit(Overloaded{
    ///       [](int x)    { /* ... */ },
    ///       [](double x) { /* ... */ },
    ///       [](auto)     { /* ... */ },
    ///   }, variant);
    /// @endcode
    template <typename... Ts>
    struct Overloaded : Ts...
    {
        using Ts::operator()...;
    };

    /// Deduction guide — allows Overloaded{...} without explicitly
    /// specifying template arguments.
    template <typename... Ts>
    Overloaded(Ts...) -> Overloaded<Ts...>;

} // namespace engine::util