#include <gtest/gtest.h>
#include "strong.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>

TEST(SmokeTest, SmokeTest1) 
{
    std::cout << "Running SmokeTest::SmokeTest1\n";
    ASSERT_EQ(1, 1);
}

namespace {

using Meters = strong<double, struct meters_tag>;
using Name   = strong<std::string, struct name_tag>;

struct FakeStrong {        // ten sam układ co strong, ale nie jest silnym typem
    double value_;
};

// Czy skill_access::get(x) w ogóle da się wywołać dla danego typu?
template <typename X>
concept gettable = requires(X&& x) { skill_access::get(std::forward<X>(x)); };

} // namespace

TEST(SmokeTest, SkillAccessTest1) {
    std::cout << "Running SmokeTest::SkillAccessTest1\n";

    Meters m{3.5};
    const Meters cm{7.0};

    // Typy zwracane: T&, const T&, T&&.
    static_assert(std::is_same_v<decltype(skill_access::get(m)), double&>);
    static_assert(std::is_same_v<decltype(skill_access::get(cm)), const double&>);
    static_assert(std::is_same_v<decltype(skill_access::get(std::move(m))), double&&>);
    static_assert(std::is_same_v<decltype(skill_access::get(std::move(cm))), const double&&>);

    // Odczyt.
    EXPECT_DOUBLE_EQ(skill_access::get(m), 3.5);
    EXPECT_DOUBLE_EQ(skill_access::get(cm), 7.0);

    // Zapis przez referencję zmienia obiekt (to nie jest kopia).
    skill_access::get(m) = 42.0;
    EXPECT_DOUBLE_EQ(skill_access::get(m), 42.0);
    EXPECT_EQ(&skill_access::get(m), &skill_access::get(m));

    // R-wartość pozwala przenieść wartość.
    Name n{"Ala ma kota, a kot ma bardzo długie imię"};
    std::string moved = skill_access::get(std::move(n));
    EXPECT_EQ(moved, "Ala ma kota, a kot ma bardzo długie imię");

    // Działa w wyrażeniu stałym.
    static_assert(skill_access::get(Meters{1.5}) == 1.5);

    // Dla typów niebędących silnymi się nie kompiluje.
    static_assert(gettable<Meters&>);
    static_assert(gettable<const Meters&>);
    static_assert(gettable<Meters>);
    static_assert(!gettable<double&>);
    static_assert(!gettable<FakeStrong&>);
    static_assert(!gettable<Meters*>);
}

TEST(SmokeTest, IsStrongTest1) {
    std::cout << "Running SmokeTest::IsStrongTest1\n";
    // Silne typy, także z referencją i kwalifikatorami.
    EXPECT_TRUE(is_strong_v<Meters>);
    EXPECT_TRUE(is_strong_v<Name>);
    EXPECT_TRUE(is_strong_v<const Meters>);
    EXPECT_TRUE(is_strong_v<volatile Meters>);
    EXPECT_TRUE(is_strong_v<const volatile Meters>);
    EXPECT_TRUE(is_strong_v<Meters&>);
    EXPECT_TRUE(is_strong_v<const Meters&>);
    EXPECT_TRUE(is_strong_v<Meters&&>);

    // Nie-silne typy.
    EXPECT_FALSE(is_strong_v<double>);
    EXPECT_FALSE(is_strong_v<std::string>);
    EXPECT_FALSE(is_strong_v<FakeStrong>);
    EXPECT_FALSE(is_strong_v<Meters*>);        // wskaźnik to nie silny typ
    EXPECT_FALSE(is_strong_v<const Meters*>);
    EXPECT_FALSE(is_strong_v<Meters[2]>);      // tablica też nie

    // Musi to być stała czasu kompilacji typu bool.
    static_assert(is_strong_v<Meters>);
    static_assert(!is_strong_v<int>);
    static_assert(std::is_same_v<decltype(is_strong_v<Meters>), const bool>);
}

TEST(StrongSmoke, Readable) {
    using Meters = strong<double, struct meters_tag, readable>;

    Meters m(7.5);
    const Meters cm(2.0);
    EXPECT_DOUBLE_EQ(m.value(), 7.5);
    EXPECT_DOUBLE_EQ(cm.value(), 2.0);
    EXPECT_DOUBLE_EQ(Meters().value(), 0.0);
    static_assert(std::is_same_v<decltype(m.value()), const double&>);

    constexpr Meters c(3.0);
    static_assert(c.value() == 3.0);
}

TEST(StrongSmoke, AddableWith) {
    using Seconds = strong<int, struct seconds_tag, addable, readable>;
    using Product = strong<int, struct product_tag, addable_with<std::multiplies<>{}>, readable>;

    Seconds a(2), b(3);
    static_assert(std::is_same_v<decltype(a + b), Seconds>);
    EXPECT_EQ((a + b).value(), 5);
    a += b;
    EXPECT_EQ(a.value(), 5);

    Product p(4), q(5);
    EXPECT_EQ((p + q).value(), 20);

    static_assert((Seconds(20) + Seconds(22)).value() == 42);
}

TEST(StrongSmoke, Scalable) {
    using Meters = strong<double, struct meters_tag, scalable<double>, readable>;

    Meters m(6.0);
    static_assert(std::is_same_v<decltype(m * 2.0), Meters>);
    EXPECT_DOUBLE_EQ((m * 2.0).value(), 12.0);
    EXPECT_DOUBLE_EQ((2.0 * m).value(), 12.0);
    EXPECT_DOUBLE_EQ((m / 4.0).value(), 1.5);

    m *= 3.0;
    EXPECT_DOUBLE_EQ(m.value(), 18.0);
    m /= 2.0;
    EXPECT_DOUBLE_EQ(m.value(), 9.0);

    static_assert(scalable<double>::accepts<int>);
    static_assert(!scalable<double>::accepts<std::string>);

    static_assert((Meters(1.5) * 4.0).value() == 6.0);
}