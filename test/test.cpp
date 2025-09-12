#include <iostream>

#include "../small_bitset.hpp"
#include <gtest/gtest.h>

using nonstd::small_bitset;

namespace {
static constexpr std::size_t kNumBits{128};
}

// Compile-time tests
static_assert(sizeof(small_bitset<1, std::uint16_t>) > sizeof(small_bitset<1, std::uint8_t>),
              "Smaller Underlying type did not result in smaller object");

TEST(SmallBitset, bracket_operator) {
    small_bitset<kNumBits> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
}

TEST(SmallBitset, initialize_to_false) {
    small_bitset<kNumBits> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }
}

TEST(SmallBitset, set) {
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s[i]) << "i: " << i;
        for (int j = 0; j < s.size(); j++) {
            if (j == i) { // do not test the bit that was set
                continue;
            }
            ASSERT_FALSE(s[j]) << "Unexpected bit set at index " << j;
        }
        s.set(i, false);
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
}

TEST(SmallBitset, flip_all) {
    small_bitset<kNumBits> s;

    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);

        // flip to reverse
        s.flip();
        ASSERT_FALSE(s[i]);
        for (int j = 0; j < s.size(); j++) {
            if (j == i) { // do not test the bit that was set
                continue;
            }
            ASSERT_TRUE(s[j]);
        }

        // flip back to normal
        s.flip();
        ASSERT_TRUE(s[i]);
        for (int j = 0; j < s.size(); j++) {
            if (j == i) { // do not test the bit that was set
                continue;
            }
            ASSERT_FALSE(s[j]);
        }

        s.set(i, false);
    }
}

TEST(SmallBitset, flip_index) {
    small_bitset<kNumBits> s;

    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s[i]);

        s.flip(i);
        ASSERT_FALSE(s[i]);

        s.flip();
        ASSERT_TRUE(s[i]);

        s.set(i, false);
    }

    ASSERT_THROW(s.flip(s.size()), std::out_of_range);
}

TEST(SmallBitset, test) {
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s.test(i)) << "i: " << i;
        s.set(i, false);
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }
}

TEST(SmallBitset, count) {
    small_bitset<kNumBits> s;
    ASSERT_EQ(s.count(), 0);
    for (std::size_t i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_EQ(s.count(), i + 1) << "i: " << i;
    }
}

TEST(SmallBitset, size) {
    small_bitset<1> s_1;
    static_assert(s_1.size() == 1);

    small_bitset<8> s_8;
    static_assert(s_8.size() == 8);

    small_bitset<9> s_9;
    static_assert(s_9.size() == 9);

    small_bitset<64> s_64;
    static_assert(s_64.size() == 64);

    small_bitset<129> s_129;
    static_assert(s_129.size() == 129);
}

TEST(SmallBitset, all) {
    small_bitset<kNumBits, uint8_t> s;
    for (auto i = 0; i < s.size() - 1; ++i) {
        s.set(i, true);
        EXPECT_FALSE(s.all());
    }
    s.set(s.size() -1, true);
    EXPECT_TRUE(s.all());
}

TEST(SmallBitset, any) {
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_TRUE(s.any()) << "i: " << i;
        s.set(i, false);
        EXPECT_FALSE(s.any()) << "i: " << i;
    }
}

TEST(SmallBitset, none) {
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_FALSE(s.none()) << "i: " << i;
        s.set(i, false);
        EXPECT_TRUE(s.none()) << "i: " << i;
    }
}

TEST(SmallBitset, bitwise_and) {
    small_bitset<kNumBits> s1;
    small_bitset<kNumBits> s2;

    s1.set(0);
    s2.set(0);

    s1.set(1);
    s1 &= s2;
    ASSERT_TRUE(s1[0]);
    ASSERT_FALSE(s1[1]);
}

TEST(SmallBitset, bitwise_or) {
    small_bitset<kNumBits> s1;
    small_bitset<kNumBits> s2;

    s1.set(0);
    s2.set(1);

    s1 |= s2;
    ASSERT_TRUE(s1[0]);
    ASSERT_TRUE(s1[1]);
    ASSERT_FALSE(s1[2]);
}

TEST(SmallBitset, bitwise_xor) {
    small_bitset<kNumBits> s1;
    small_bitset<kNumBits> s2;

    s1.set(0);
    s2.set(1);
    s1.set(2);
    s2.set(2);

    s1 ^= s2;
    ASSERT_TRUE(s1[0]);
    ASSERT_TRUE(s1[1]);
    ASSERT_FALSE(s1[2]);
    ASSERT_FALSE(s1[3]);
}

TEST(SmallBitset, bitwise_not) {
    small_bitset<kNumBits> s1;

    s1.set(0);
    s1.set(2);
    auto inverse = ~s1;

    ASSERT_TRUE(s1[0]);
    ASSERT_FALSE(inverse[0]);

    ASSERT_FALSE(s1[1]);
    ASSERT_TRUE(inverse[1]);

    ASSERT_TRUE(s1[2]);
    ASSERT_FALSE(inverse[2]);

    ASSERT_FALSE(s1[3]);
    ASSERT_TRUE(inverse[3]);
}
