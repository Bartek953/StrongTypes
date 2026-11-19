#include <gtest/gtest.h>
#include "strong.h"
#include <iostream>
#include <string>
#include <type_traits>
#include <utility>
#include <unordered_set>

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

TEST(StrongSmoke, EqualityWith) {
    using Meters = strong<double, struct meters_tag, equality>;
    using Name = strong<std::string, struct name_tag, equality>;
    using Mod10 = strong<int, struct mod10_tag, equality_with<[](int a, int b) { return a % 10 == b % 10; }>>;

    Meters a(1.5), b(1.5), c(2.0);
    static_assert(std::is_same_v<decltype(a == b), bool>);
    EXPECT_TRUE(a == b);
    EXPECT_FALSE(a == c);
    EXPECT_TRUE(a != c);
    EXPECT_FALSE(a != b);

    EXPECT_EQ(Name("Ala"), Name("Ala"));
    EXPECT_NE(Name("Ala"), Name("Ola"));

    EXPECT_TRUE(Mod10(13) == Mod10(23));
    EXPECT_TRUE(Mod10(13) != Mod10(14));

    static_assert(Meters(3.0) == Meters(3.0));
    static_assert(Name("ab") != Name("cd"));
}

TEST(StrongSmoke, OrderedWith) {
    using Meters = strong<double, struct meters_tag, ordered, ordered::needs>;
    using Name = strong<std::string, struct name_tag, ordered, ordered::needs>;
    using Rev = strong<int, struct rev_tag,
        ordered_with<[](int a, int b) { return b <=> a; }>,
        ordered_with<[](int a, int b) { return b <=> a; }>::needs>;

    Meters a(1.0), b(2.0);
    static_assert(std::is_same_v<decltype(a <=> b), std::partial_ordering>);
    EXPECT_TRUE(a < b);
    EXPECT_TRUE(b > a);
    EXPECT_TRUE(a <= a);
    EXPECT_TRUE(a == Meters(1.0));
    EXPECT_TRUE(a != b);

    static_assert(std::is_same_v<decltype(Rev(1) <=> Rev(2)), std::strong_ordering>);
    EXPECT_TRUE(Rev(5) < Rev(3));

    static_assert(Name("a") < Name("b"));
    static_assert(Meters(3.0) >= Meters(2.0));
}

TEST(StrongSmoke, Printable) {
    using Meters = strong<double, struct meters_tag, printable<"m">>;
    using Plain = strong<int, struct plain_tag, printable<>>;
    using Name = strong<std::string, struct name_tag, printable<" (name)">>;

    std::ostringstream os;
    os << Meters(7.5);
    EXPECT_EQ(os.str(), "7.5m");

    os.str("");
    os << Plain(42) << ' ' << Name("Ala");
    EXPECT_EQ(os.str(), "42 Ala (name)");
}

TEST(StrongSmoke, HashableWith) {
    using Id = strong<int, struct id_tag, hashable, equality>;
    using Name = strong<std::string, struct name_tag, hashable, equality>;
    using Const = strong<int, struct const_tag, hashable_with<[](int) { return std::size_t{7}; }>, equality>;
    using Bare = strong<int, struct bare_tag>;

    EXPECT_EQ(std::hash<Id>{}(Id(5)), std::hash<int>{}(5));
    EXPECT_EQ(std::hash<Name>{}(Name("Ala")), std::hash<std::string>{}("Ala"));
    EXPECT_EQ(std::hash<Const>{}(Const(123)), 7u);

    std::unordered_set<Id> set{Id(1), Id(2), Id(1)};
    EXPECT_EQ(set.size(), 2u);
    EXPECT_TRUE(set.contains(Id(2)));

    static_assert(std::hash<Const>{}(Const(1)) == 7);
    static_assert(!std::is_default_constructible_v<std::hash<Bare>>);
}