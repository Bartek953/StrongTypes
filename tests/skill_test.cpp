#include <gtest/gtest.h>

#include "strong.h"

#include <compare>
#include <cstddef>
#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <unordered_set>

// ===================== readable =====================

TEST(SkillTest, ReadableTest1) {
    using S = strong<int, struct r1_tag, readable>;
    S s(42);
    EXPECT_EQ(s.value(), 42);
}

TEST(SkillTest, ReadableTest2) {
    using S = strong<int, struct r2_tag, readable>;
    S s(1);
    const S cs(2);
    static_assert(std::is_same_v<decltype(s.value()), const int&>);
    static_assert(std::is_same_v<decltype(cs.value()), const int&>);
    EXPECT_EQ(cs.value(), 2);
}

TEST(SkillTest, ReadableTest3) {
    using S = strong<std::string, struct r3_tag, readable>;
    S s(3, 'x');
    EXPECT_EQ(s.value(), "xxx");
}

TEST(SkillTest, ReadableTest4) {
    using S = strong<int, struct r4_tag, readable>;
    constexpr S s(7);
    static_assert(s.value() == 7);
    EXPECT_EQ(s.value(), 7);
}

TEST(SkillTest, ReadableTest5) {
    using S = strong<double, struct r5_tag, readable>;
    using N = strong<double, struct r5n_tag>;
    static_assert(sizeof(S) == sizeof(double));
    static_assert(sizeof(N) == sizeof(double));
    static_assert(std::is_trivially_copyable_v<S>);
    S a(1.5);
    S b = a;
    EXPECT_DOUBLE_EQ(b.value(), 1.5);
}

// ===================== addable =====================

TEST(SkillTest, AddableTest1) {
    using S = strong<int, struct a1_tag, readable, addable>;
    S c = S(2) + S(3);
    EXPECT_EQ(c.value(), 5);
    static_assert(std::is_same_v<decltype(S(2) + S(3)), S>);
}

TEST(SkillTest, AddableTest2) {
    using S = strong<int, struct a2_tag, readable, addable>;
    S a(10);
    S& ref = (a += S(5));
    EXPECT_EQ(&ref, &a);
    EXPECT_EQ(a.value(), 15);
}

TEST(SkillTest, AddableTest3) {
    using S = strong<int, struct a3_tag, readable, addable_with<std::minus<>{}>>;
    S c = S(10) + S(4);
    EXPECT_EQ(c.value(), 6);
    S d(1);
    d += S(3);
    EXPECT_EQ(d.value(), -2);
}

TEST(SkillTest, AddableTest4) {
    using S = strong<std::string, struct a4_tag, readable, addable>;
    S c = S("ab") + S("cd");
    EXPECT_EQ(c.value(), "abcd");
    using M = strong<int, struct a4m_tag, readable,
                     addable_with<[](int x, int y) { return x < y ? y : x; }>>;
    EXPECT_EQ((M(3) + M(8)).value(), 8);
}

TEST(SkillTest, AddableTest5) {
    using S = strong<int, struct a5_tag, readable, addable>;
    using N = strong<int, struct a5n_tag, readable>;
    constexpr S c = S(1) + S(2);
    static_assert(c.value() == 3);
    static_assert(std::is_invocable_v<std::plus<>, S, S>);
    static_assert(!std::is_invocable_v<std::plus<>, N, N>);
    static_assert(!std::is_invocable_v<std::plus<>, S, int>);
    EXPECT_EQ(c.value(), 3);
}

// ===================== scalable =====================

TEST(SkillTest, ScalableTest1) {
    using S = strong<double, struct s1_tag, readable, scalable<double>>;
    S a(3.0);
    EXPECT_DOUBLE_EQ((a * 2.0).value(), 6.0);
    EXPECT_DOUBLE_EQ((2.0 * a).value(), 6.0);
    EXPECT_DOUBLE_EQ((a / 2.0).value(), 1.5);
}

TEST(SkillTest, ScalableTest2) {
    using S = strong<double, struct s2_tag, readable, scalable<double>>;
    S a(4.0);
    S& r1 = (a *= 3.0);
    EXPECT_EQ(&r1, &a);
    EXPECT_DOUBLE_EQ(a.value(), 12.0);
    S& r2 = (a /= 4.0);
    EXPECT_EQ(&r2, &a);
    EXPECT_DOUBLE_EQ(a.value(), 3.0);
}

TEST(SkillTest, ScalableTest3) {
    using S = strong<int, struct s3_tag, readable, scalable<int>>;
    S a(7);
    static_assert(std::is_same_v<decltype(a * 2), S>);
    static_assert(std::is_same_v<decltype(2 * a), S>);
    static_assert(std::is_same_v<decltype(a / 2), S>);
    EXPECT_EQ((a * 2).value(), 14);
    EXPECT_EQ((a / 2).value(), 3);
}

TEST(SkillTest, ScalableTest4) {
    using S = strong<int, struct s4_tag, readable, scalable<int>>;
    constexpr S a = S(5) * 4;
    constexpr S b = 2 * S(3);
    constexpr S c = S(9) / 3;
    static_assert(a.value() == 20);
    static_assert(b.value() == 6);
    static_assert(c.value() == 3);
    EXPECT_EQ(a.value(), 20);
}

TEST(SkillTest, ScalableTest5) {
    using S = strong<double, struct s5_tag, readable, scalable<double>>;
    using N = strong<double, struct s5n_tag, readable>;
    static_assert(!std::is_invocable_v<std::multiplies<>, S, S>);
    static_assert(!std::is_invocable_v<std::divides<>, double, S>);
    static_assert(!std::is_invocable_v<std::multiplies<>, N, double>);
    static_assert(!std::is_invocable_v<std::plus<>, S, S>);
    EXPECT_DOUBLE_EQ((S(1.0) * 0.5).value(), 0.5);
}

// ===================== equality =====================

TEST(SkillTest, EqualityTest1) {
    using S = strong<int, struct e1_tag, equality>;
    EXPECT_TRUE(S(1) == S(1));
    EXPECT_FALSE(S(1) == S(2));
}

TEST(SkillTest, EqualityTest2) {
    using S = strong<int, struct e2_tag, equality>;
    EXPECT_TRUE(S(1) != S(2));
    EXPECT_FALSE(S(3) != S(3));
}

TEST(SkillTest, EqualityTest3) {
    using S = strong<int, struct e3_tag,
                     equality_with<[](int a, int b) { return a % 10 == b % 10; }>>;
    EXPECT_TRUE(S(13) == S(23));
    EXPECT_FALSE(S(13) == S(14));
    EXPECT_TRUE(S(13) != S(14));
}

TEST(SkillTest, EqualityTest4) {
    using S = strong<std::string, struct e4_tag, equality>;
    EXPECT_TRUE(S("abc") == S("abc"));
    EXPECT_TRUE(S("abc") != S("abd"));
    using I = strong<int, struct e4i_tag, equality>;
    static_assert(I(5) == I(5));
    static_assert(I(5) != I(6));
}

TEST(SkillTest, EqualityTest5) {
    using S = strong<int, struct e5_tag, equality>;
    using N = strong<int, struct e5n_tag>;
    static_assert(std::equality_comparable<S>);
    static_assert(!std::equality_comparable<N>);
    static_assert(!std::totally_ordered<S>);
    static_assert(!std::equality_comparable_with<S, int>);
    EXPECT_TRUE(S(0) == S(0));
}

// ===================== ordered =====================

TEST(SkillTest, OrderedTest1) {
    using S = strong<int, struct o1_tag, ordered>;
    EXPECT_TRUE(S(1) < S(2));
    EXPECT_TRUE(S(2) > S(1));
    EXPECT_TRUE(S(2) <= S(2));
    EXPECT_TRUE(S(3) >= S(2));
    EXPECT_FALSE(S(3) < S(2));
}

TEST(SkillTest, OrderedTest2) {
    // ordered declares its family and needs an equality skill consistent with Cmp
    static_assert(std::is_same_v<ordered::family, ops::ordering>);
    static_assert(std::is_same_v<ordered::needs::family, ops::equality>);
    // the generated equality_with<E> works as a standalone skill: E(x, y) == (Cmp(x, y) == 0)
    using S = strong<int, struct o2_tag, ordered::needs>;
    EXPECT_TRUE(S(4) == S(4));
    EXPECT_TRUE(S(4) != S(5));
    using D = strong<double, struct o2d_tag, ordered::needs>;
    EXPECT_TRUE(D(1.5) == D(1.5));
    EXPECT_FALSE(D(1.5) == D(2.5));
}

TEST(SkillTest, OrderedTest3) {
    using I = strong<int, struct o3i_tag, ordered>;
    using D = strong<double, struct o3d_tag, ordered>;
    static_assert(std::is_same_v<decltype(I(1) <=> I(2)), std::strong_ordering>);
    static_assert(std::is_same_v<decltype(D(1.0) <=> D(2.0)), std::partial_ordering>);
    EXPECT_EQ(I(1) <=> I(2), std::strong_ordering::less);
    EXPECT_EQ(D(2.0) <=> D(2.0), std::partial_ordering::equivalent);
}

TEST(SkillTest, OrderedTest4) {
    using S = strong<int, struct o4_tag,
                     ordered_with<[](int a, int b) { return b <=> a; }>, equality>;
    EXPECT_TRUE(S(5) < S(3));
    EXPECT_TRUE(S(1) > S(9));
    EXPECT_TRUE(S(7) == S(7));
    EXPECT_FALSE(S(7) == S(8));
}

TEST(SkillTest, OrderedTest5) {
    using S = strong<int, struct o5_tag, ordered, equality>;
    using E = strong<int, struct o5e_tag, equality>;
    static_assert(std::totally_ordered<S>);
    static_assert(!std::totally_ordered<E>);
    static_assert(!std::is_invocable_v<std::less<>, E, E>);
    static_assert(S(1) < S(2));
    static_assert(S(2) >= S(2));
    EXPECT_TRUE(S(-1) < S(0));
}

// ===================== printable =====================

TEST(SkillTest, PrintableTest1) {
    using S = strong<double, struct p1_tag, printable<"m">>;
    std::ostringstream os;
    os << S(7.5);
    EXPECT_EQ(os.str(), "7.5m");
}

TEST(SkillTest, PrintableTest2) {
    using S = strong<int, struct p2_tag, printable<>>;
    std::ostringstream os;
    os << S(42);
    EXPECT_EQ(os.str(), "42");
}

TEST(SkillTest, PrintableTest3) {
    using S = strong<std::string, struct p3_tag, printable<"!">>;
    std::ostringstream os;
    os << S("hello");
    EXPECT_EQ(os.str(), "hello!");
}

TEST(SkillTest, PrintableTest4) {
    using S = strong<int, struct p4_tag, printable<" kg">>;
    std::ostringstream os;
    os << S(1) << ", " << S(2);
    EXPECT_EQ(os.str(), "1 kg, 2 kg");
}

TEST(SkillTest, PrintableTest5) {
    using S = strong<int, struct p5_tag, printable<"s">>;
    using N = strong<int, struct p5n_tag>;
    static_assert(sizeof(N) == sizeof(S));
    std::ostringstream os;
    std::ostream& ret = (os << S(3));
    EXPECT_EQ(&ret, &os);
    EXPECT_EQ(os.str(), "3s");
}

// ===================== hashable =====================

TEST(SkillTest, HashableTest1) {
    using S = strong<int, struct h1_tag, hashable>;
    EXPECT_EQ(std::hash<S>{}(S(5)), std::hash<int>{}(5));
}

TEST(SkillTest, HashableTest2) {
    using S = strong<std::string, struct h2_tag, hashable>;
    EXPECT_EQ(std::hash<S>{}(S("abc")), std::hash<std::string>{}("abc"));
}

TEST(SkillTest, HashableTest3) {
    using S = strong<std::string, struct h3_tag,
                     hashable_with<[](const std::string& s) { return s.size(); }>>;
    EXPECT_EQ(std::hash<S>{}(S("abcd")), 4u);
    EXPECT_EQ(std::hash<S>{}(S("")), 0u);
}

TEST(SkillTest, HashableTest4) {
    using S = strong<int, struct h4_tag, hashable, equality>;
    std::unordered_set<S> set;
    set.insert(S(1));
    set.insert(S(2));
    set.insert(S(1));
    EXPECT_EQ(set.size(), 2u);
    EXPECT_TRUE(set.contains(S(2)));
    EXPECT_FALSE(set.contains(S(3)));
}

TEST(SkillTest, HashableTest5) {
    using S = strong<int, struct h5_tag, hashable, equality>;
    using N = strong<int, struct h5n_tag, equality>;
    // hashable declares its family and depends on equality
    static_assert(std::is_same_v<hashable::family, ops::hashing>);
    static_assert(std::is_same_v<hashable::needs, equality>);
    static_assert(std::is_default_constructible_v<std::hash<S>>);
    static_assert(!std::is_default_constructible_v<std::hash<N>>);
    EXPECT_EQ(std::hash<S>{}(S(9)), std::hash<int>{}(9));
}

// ===================== convertible_to =====================

TEST(SkillTest, ConvertibleTest1) {
    using S = strong<int, struct c1_tag, convertible_to<int>>;
    S s(17);
    EXPECT_EQ(static_cast<int>(s), 17);
}

TEST(SkillTest, ConvertibleTest2) {
    using S = strong<int, struct c2_tag, convertible_to<double>>;
    static_assert(std::is_constructible_v<double, S>);
    static_assert(!std::is_convertible_v<S, double>);  // explicit only
    EXPECT_DOUBLE_EQ(static_cast<double>(S(3)), 3.0);
}

TEST(SkillTest, ConvertibleTest3) {
    using S = strong<int, struct c3_tag, convertible_to<int, [](int x) { return x * 2; }>>;
    EXPECT_EQ(static_cast<int>(S(21)), 42);
}

TEST(SkillTest, ConvertibleTest4) {
    using M = strong<double, struct c4m_tag, readable>;
    using Km = strong<double, struct c4km_tag, readable,
                      convertible_to<M, [](double v) { return v * 1000.0; }>>;
    M m = static_cast<M>(Km(2.5));
    EXPECT_DOUBLE_EQ(m.value(), 2500.0);
    static_assert(!std::is_convertible_v<Km, M>);
    static_assert(!std::is_constructible_v<Km, M>);
}

TEST(SkillTest, ConvertibleTest5) {
    using S = strong<int, struct c5_tag, convertible_to<long>, convertible_to<double>>;
    using N = strong<int, struct c5n_tag>;
    EXPECT_EQ(static_cast<long>(S(5)), 5L);
    EXPECT_DOUBLE_EQ(static_cast<double>(S(5)), 5.0);
    static_assert(!std::is_constructible_v<int, N>);
    static_assert(static_cast<long>(S(8)) == 8L);
}

// ===================== divisible_by =====================

TEST(SkillTest, DivisibleTest1) {
    using T = strong<double, struct d1t_tag, readable>;
    using V = strong<double, struct d1v_tag, readable>;
    using D = strong<double, struct d1d_tag, readable, divisible_by<T, V>>;
    V v = D(100.0) / T(20.0);
    EXPECT_DOUBLE_EQ(v.value(), 5.0);
    static_assert(std::is_same_v<decltype(D(1.0) / T(1.0)), V>);
}

TEST(SkillTest, DivisibleTest2) {
    using D = strong<double, struct d2_tag, divisible_by<double, double>>;
    double r = D(9.0) / 3.0;
    EXPECT_DOUBLE_EQ(r, 3.0);
    static_assert(std::is_same_v<decltype(D(1.0) / 1.0), double>);
}

TEST(SkillTest, DivisibleTest3) {
    using O = strong<int, struct d3o_tag>;
    using D = strong<int, struct d3d_tag,
                     divisible_by<O, int, [](int a, int b) { return a % b; }>>;
    EXPECT_EQ(D(17) / O(5), 2);
}

TEST(SkillTest, DivisibleTest4) {
    using T = strong<double, struct d4t_tag, readable>;
    using V = strong<double, struct d4v_tag, readable>;
    using D = strong<double, struct d4d_tag, readable, divisible_by<T, V>>;
    static_assert(std::is_invocable_v<std::divides<>, D, T>);
    static_assert(!std::is_invocable_v<std::divides<>, T, D>);  // o / a not available
    static_assert(!std::is_invocable_v<std::divides<>, D, V>);
    static_assert(!std::is_invocable_v<std::divides<>, D, double>);
    EXPECT_DOUBLE_EQ((D(6.0) / T(4.0)).value(), 1.5);
}

TEST(SkillTest, DivisibleTest5) {
    using T = strong<double, struct d5t_tag, readable>;
    using V = strong<double, struct d5v_tag, readable>;
    using D = strong<double, struct d5d_tag, readable, scalable<double>, divisible_by<T, V>>;
    D half = D(10.0) / 2.0;           // scalable
    V speed = D(10.0) / T(2.0);       // divisible_by
    EXPECT_DOUBLE_EQ(half.value(), 5.0);
    EXPECT_DOUBLE_EQ(speed.value(), 5.0);
    constexpr V cv = D(8.0) / T(4.0);
    static_assert(cv.value() == 2.0);
}