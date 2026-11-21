#ifndef STRONG_H
#define STRONG_H

#include <concepts>
#include <type_traits>
#include <utility>
#include <functional>
#include <algorithm>
#include <cstddef>
#include <ostream>
#include <compare>

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
    using explicit_skills = strong_detail::flatten_t<Skills...>;

    static_assert(!strong_detail::has_duplicates_v<explicit_skills>, 
                "Explicit skill list cannot contain duplicate skills.");

    static_assert(!strong_detail::has_family_conflict_v<explicit_skills>,
                "Explicit skill list cannot contain multiple skills from the same family.");

    using full_skills = strong_detail::resolve_skills_t<explicit_skills>;

    static_assert(strong_detail::all_have_mixin<full_skills>::value,
        "Every skill on the full skill list must define a public mixin<Strong> template.");

    static_assert(strong_detail::all_accept_type<full_skills, T>::value,
        "At least one skill does not accept the underlying value type T.");

    constexpr strong() requires std::default_initializable<T> : m_value() {}

    template <typename... Args>
        requires
        (sizeof...(Args) > 0) && 
        std::constructible_from<T, Args...> &&
        (!is_strong_v<std::remove_cvref_t<Args>> && ...)
    constexpr explicit strong(Args&&... args) : m_value(std::forward<Args>(args)...) {}
    
    constexpr strong(const strong&) = default;
    constexpr strong(strong&&) = default;
    constexpr strong& operato r=(const strong&) = default;
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

namespace strong_detail {
    template <typename... Ts>
    struct type_list {};

    template <typename List1, typename List2>
    struct concat;

    template <typename... Ts1, typename... Ts2>
    struct concat<type_list<Ts1...>, type_list<Ts2...>> {
        using type = type_list<Ts1..., Ts2...>;
    };

    template <typename List1, typename List2>
    using concat_t = typename concat<List1, List2>::type;

    template <typename T>
    struct flatten_one {
        using type = type_list<T>;
    };

    template <typename... Inner>
    struct flatten_one<skills<Inner...>> {
        using type = typename flatten<Inner...>::type;
    };

    template <typename... Ts>
    struct flatten {
        using type = type_list<>;
    };

    template <typename Head, typename... Tail>
    struct flatten<Head, Tail...> {
        using type = concat_t<
            typename flatten_one<Head>::type,
            typename flatten<Tail...>::type
        >;
    };

    template <typename... Ts>
    using flatten_t = typename flatten<Ts...>::type;
}

namespace strong_detail {
    template <typename T, typename List>
    struct contains;

    template<typename T, typename... Ts>
    struct contains<T, type_list<Ts...>> {
        static constexpr bool value = (std::same_as<T, Ts> || ...);
    };

    template <typename T, typename List>
    inline constexpr bool contains_v = contain<T, List>::value;
}

namespace strong_detail {
    template <typename List>
    struct has_duplicates;

    template<>
    struct has_duplicates<type_list<>> {
        static constexpr bool value = false;
    };

    template <typename Head, typename... Tail>
    struct has_duplicates<type_list<Head, Tail...>> {
        static constexpr bool value = 
            contains_v<Head, type_list<Tail...>> ||
            has_duplicates<type_list<Tail...>>::value;
    };

    template <typename List>
    inline constexpr bool has_duplicates_v = has_duplicates<List>::value;
}

namespace strong_detail {
    template <typename Skill>
    concept has_family_alias = requires {
        typename Skill::family;
    };

    template <typename Skill>
    struct get_family {
        static constexpr bool exists = false;
    };

    template <has_family_alias Skill>
    struct get_family<Skill> {
        static constexpr bool exists = true;
        using type = typename Skill::family;
    };
}

namespace strong_detail {
    template <typename S1, typename S2>
    consteval bool shares_family() {
        if constexpr (has_family_alias<S1> && has_family_alias<S2>) {
            return std::same_as<typename S1::family, typename S2::family>;
        }
        else {
            return false;
        }
    }

    template <typename List>
    struct has_family_conflict;

    template<>
    struct has_family_conflict<type_list<>> {
        static constexpr bool value = false;
    };

    template <typename Head, typename... Tail> 
    struct has_family_conflict<type_list<Head, Tail...>> {
        static constexpr bool value =
            (shares_family<Head, Tail>() || ...) || has_family_conflict<type_list<Tail...>>::value;
    };

    template <typename List> 
    inline constexpr bool has_family_conflict_v = has_family_conflict<List>::value;
}

namespace strong_detail {
    template<typename Skill, typename ResultList>
    struct is_satisfied;

    template <typename Skill, typename... Rs>
    struct is_satisfied<Skill, type_list<Rs...>> {
        static constexpr bool value = 
            ((std::same_as<Skill, Rs> || shares_family<Skll, Rs>()) || ...);
    };

    template <typename Skill, typename ResultList>
    inline constexpr bool is_satisfied_v = is_satisfied<Skill, ResultList>::value;

    template <typename Skill>
    concept has_needs_alias = requires {
        typename Skill::needs;
    };

    template <typename Skill>
    struct get_needs {
        using type = type_list<>;
    };

    template <has_needs_alias Skill>
    struct get_needs<Skill> {
        using type = flatten_t<typename Skill::needs>;
    };

    template <typename Skill>
    using get_needs_t = typename get_needs<Skill>::type;

    template <typename DepsList, typename Queue, typename Result>
    struct process_deps;

    template <typename Queue, typename Result> 
    struct process_deps<type_list<>, Queue, Result> {
        using queue = Queue;
        using result = Result;
    };

    template <typename Dep, typename... RestDeps, typename Queue, typename Result>
    struct process_deps<type_list<Dep, RestDeps...>, Queue, Result> {
        static constexpr bool skip = is_satisfied_v<Dep, Result>;

        using next_queue = std::conditional_t<skip, Queue, concat_t<Queue, type_list<Dep>>>;

        using next_result = std::conditional_t<skip, Result, concat_t<Result, type_list<Dep>>>;

        using step = process_deps<type_list<RestDeps...?, next_queue, next_result>;
        using queue = typename step::queue;
        using result = typename step::result;
    };

    template<typename Queue, typename Result>
    struct bfs_resolve;

    template <typename Result>
    struct bfs_resolve<type_list<>, Result> {
        using type = Result;
    };

    template <typename Head, typename... Tail, typename Result>
    struct bfs_resolve<type_list<Head, Tail...>, Result> {
        using step = process_deps<get_needs_t<Head>, type_list<Tail...>, Result>;

        using type = typename bfs_resolve<typename step::queue, typename step::result>::type;
    };

    template <typename ExplicitList>
    using resolve_skills_t = typename bfs_resolve<ExplicitList, ExplicitList>::type;
}

namespace strong_detail {
    struct dummy_strong{};

    template<typename Skill>
    concept has_mixin_template = requires {
        typename Skill::template mixin<dummy_strong>;
    };

    template <typename Skill, typename T>
    concept has_accepts_var = requires {
        {Skill::template accepts<T> } -> std::convertible_to<bool>;
    };

    template <typename Skill, typename T>
    consteval bool is_skill_accepted() {
        if constexpr (has_accepts_var<Skill, T>) {
            return static_cast<bool>(Skill::template accepts<T>);
        }
        else {
            return true;
        }
    }

    template <typename List>
    struct all_have_mixin;

    template <typename... Ss>
    struct all_have_mixin<type_list<Ss...>> {
        static constexpr bool value = (has_mixin_template<Ss> && ...);
    };

    template <typename List, typename T>
    struct all_accept_type;

    template <typename... Ss, typename T>
    struct all_accept_type<type_list<Ss...>, T> {
        static constexpr bool value = (is_skill_accepted<Ss, T>() && ...);
    };
}

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
        friend constexpr S& operator+=(S& a, const S& b) {
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

template<auto Cmp>
struct ordered_with {
    using family = ops::ordering;

    template<typename S>
    struct mixin {
        friend constexpr auto operator<=>(const S& a, const S& b) {
            return Cmp(skill_access::get(a), skill_access::get(b));
        }
    };

    using needs = equality_with<[](const auto& a, const auto& b){ return Cmp(a, b) == 0; }>;
};
using ordered = ordered_with<std::compare_three_way{}>;

namespace strong_detail {
    template <std::size_t N>
    struct fixed_string {
        char data[N]{};
        constexpr fixed_string(const char (&s)[N]) { std::copy_n(s, N, data); }
    };
}

// const char* cannot be directly passed via template.
template<strong_detail::fixed_string suffix = "">
struct printable {
    using family = ops::printing;

    template<typename S>
    struct mixin {
        friend std::ostream& operator<<(std::ostream& os, const S& a) {
            os << skill_access::get(a) << suffix.data;
            return os;
        }
    };
};

template<auto H>
struct hashable_with {
    using family = ops::hashing;

    template<typename S>
    struct mixin {
        friend constexpr std::size_t strong_hash_value(const S& a) {
            return H(skill_access::get(a));
        }
    };

    using needs = equality;

    // It's not directly in specification, but's it's needed for std::hash support.
    template <typename T>
    static constexpr bool accepts = requires(const T& v) {
        { H(v) } -> std::convertible_to<std::size_t>;
    };
};

using hashable = hashable_with<[](const auto& v) {
    return std::hash<std::remove_cvref_t<decltype(v)>>{}(v);
}>;

template <typename T, typename Tag, typename... Skills>
    requires requires(const strong<T, Tag, Skills...>& s) { strong_hash_value(s); }
struct std::hash<strong<T, Tag, Skills...>> {
    constexpr std::size_t operator()(const strong<T, Tag, Skills...>& s) const {
        return strong_hash_value(s);
    }
};

template<typename Target, auto Conv = std::identity{}>
struct convertible_to {
    template<typename S>
    struct mixin {
        constexpr explicit operator Target() const {
            return static_cast<Target>(Conv(skill_access::get(static_cast<const S&>(*this))));
        }
    };

    template<typename T>
    static constexpr bool accepts = requires(const T& v) {
        requires std::constructible_from<Target, decltype(Conv(v))>;
    };
};

template<typename Other, typename Result, auto Op=std::divides<>{}>
struct divisible_by {
    template<typename S>
    struct mixin {
        friend constexpr Result operator/(const S& a, const Other& o) {
            if constexpr (is_strong_v<Other>) {
                return static_cast<Result>(Op(skill_access::get(a), skill_access::get(o)));
            }
            else {
                return static_cast<Result>(Op(skill_access::get(a), o));
            }
        }
    };
};

#endif
