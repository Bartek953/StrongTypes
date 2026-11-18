#ifndef STRONG_H
#define STRONG_H

#include <concepts>
#include <type_traits>
#include <utility>

namespace strong_detail {
    template <typename T>
    concept valid_strong_value = 
        (std::is_arithmetic_v<T> || std::is_class_v<T> || std::is_pointer_v<T>) &&
        (!std::is_reference_v<T>) &&
        (!std::is_function_v<T>) &&
        (!std::is_void_v<T>) &&
        (!std::is_const_v<T>) &&
        (!std::is_volatile_v<T>) && 
        (!std::is_array_v<T>) &&
        (!std::is_abstract_v<T>);
}

template <typename T, typename Tag, typename... Skills>
requires strong_detail::valid_strong_value<T>
class strong;

template <typename U>
inline constexpr bool is_strong_v = false;

template <typename T, typename Tag, typename... Skills>
requires strong_detail::valid_strong_value<T>
inline constexpr bool is_strong_v<strong<T, Tag, Skills...>> = true;

template <typename T, typename Tag, typename... Skills>
requires strong_detail::valid_strong_value<T>
class strong : public Skills::template mixin<strong<T, Tag, Skills...>>... {
    // friend typename Skills::template mixin<strong<T, Tag, Skills...>>...;
    // doesn't compile rn
    friend class skill_access;

public:
    using value_type = T;
    using tag_type = Tag;

    constexpr strong() requires std::default_initializable<T> : m_value() {}

    template <typename... Args>
    requires
    (sizeof...(Args) > 0) && 
    std::constructible_from<T, Args...> &&
    (!is_strong_v<std::remove_cvref_t<Args>> && ...)
    constexpr explicit strong(Args&&... args) : m_value(std::forward<Args>(args)...) {}
    
    constexpr strong(const strong&) = default;
    constexpr strong(strong&&) = default;
    constexpr strong& operator=(const strong&) = default;
    constexpr strong& operator=(strong&&) = default;
    ~strong() = default;
private:
    value_type m_value;
};

namespace strong_detail {
    template <typename S> struct is_strong : std::false_type {};

    template <typename T, typename Tag, typename... Sk>
    struct is_strong<strong<T, Tag, Sk...>> : std::true_type {};
}

class skill_access {
public:
    template<typename S>
        requires strong_detail::is_strong<std::remove_cvref_t<S>>::value
    static constexpr decltype(auto) get(S strong_object) {
        return (std::forward<S>(strong_object).m_value);
    }
};

class Skill {
public:
    template<typename T>
    struct mixin;
};

template <typename... Skills> 
struct skills;

#endif
