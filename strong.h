#ifndef STRONG_H
#define STRONG_H

namespace strong_detail {

}

template <typename T, typename Tag, typename... Skills> 
class strong;

class Skill {
public:
    template<typename T>
    struct mixin;
}

template <typename... Skills> 
struct skills;

#endif
