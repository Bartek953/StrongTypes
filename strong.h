#ifndef STRONG_H
#define STRONG_H

#include <concepts>
#include <type_traits>
#include <utility>
#include <functional>

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

namespace strong_detail {
    template <typename U>
    inline constexpr bool is_strong_raw = false;

    template <typename T, typename Tag, typename... Skills>
        requires strong_detail::valid_strong_value<T>
    inline constexpr bool is_strong_raw<strong<T, Tag, Skills...>> = true;
}

template <typename S>
inline constexpr bool is_strong_v = strong_detail::is_strong_raw<std::remove_cvref_t<S>>;

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

class skill_access {
public:
    template<typename S>
        requires is_strong_v<std::remove_cvref_t<S>>
    static constexpr decltype(auto) get(S&& strong_object) {
        return (std::forward<S>(strong_object).m_value);
    }
};

template <typename... Skills> 
struct skills;

namespace ops {
    // Skill families
    struct access {};
    struct addition {};
    struct scaling {};
    struct equality {};
    struct ordering {};
    struct printing {};
    struct hashing {};
}

// Skills
struct readable {
    using family = ops::access;

    template<typename S>
    struct mixin {
        constexpr decltype(auto) value() const {
            return skill_access::get(static_cast<const S&>(*this));
        }
    };
};

template<auto Op>
struct addable_with {
    using family = ops::addition;

    template <typename T>
    static constexpr bool accepts = requires(const T& x, const T& y) {
        requires std::constructible_from<T, decltype(Op(x, y))>;
    };

    template<typename S>
    struct mixin {
        friend constexpr S operator+(const S& a, const S& b) {
            return S(Op(skill_access::get(a), skill_access::get(b)));
        }
        friend constexpr S& operator+=(S& a, S& b) {
            a = a + b;
            return a;
        }
    };
};
using addable = addable_with<std::plus<>{}>;

template<typename Scalar>
struct scalable {
    using family = ops::scaling;

    template<typename S>
    struct mixin {
        friend constexpr S operator*(const S& a, const Scalar& s) {
            return S(skill_access::get(a) * s);
        }
        friend constexpr S operator*(const Scalar& s, const S& a) {
            return a * s;
        }
        friend constexpr S operator/(const S& a, const Scalar& s) {
            return S(skill_access::get(a) / s);
        }

        friend constexpr S& operator*=(S& a, const Scalar& s) {
            a = a * s;
            return a;
        }
        friend constexpr S& operator/=(S& a, const Scalar& s) {
            a = a / s;
            return a;
        }
    };

    template <typename T>
    static constexpr bool accepts = requires(const T& a, const Scalar& s) {
        requires std::constructible_from<T, decltype(a * s)>;
        requires std::constructible_from<T, decltype(a / s)>;
    };
};

template<auto Eq>
struct equality_with {
    using family = ops::equality;

    template<typename S>
    struct mixin {
        friend constexpr bool operator==(const S& a, const S& b) {
            return Eq(skill_access::get(a), skill_access::get(b));
        }
        friend constexpr bool operator!=(const S& a, const S& b) {
            return !(a == b);
        }
    };
};
using equality = equality_with<std::equal_to<>{}>;

#endif
