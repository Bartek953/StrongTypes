#ifndef STRONG_H
#define STRONG_H

#include <concepts>

namespace strong_detail {

}

template <typename T, typename Tag, typename... Skills>
class strong {
public:
    using value_type = T;
    using tag_type = Tag;

    ~strong();

    constexpr strong() requires std::default_initializable<T> : value();

    template <typename... Args> 
    constexpr explicit strong(Args&&... args);
private:
    value_type value;
};

template <typename... Skills> struct skills;

#endif
