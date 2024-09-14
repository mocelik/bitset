#include <iostream>

#include "../small_bitset.hpp"
#include <gtest/gtest.h>

using nonstd::small_bitset;

namespace {
static constexpr std::size_t kNumBits{128};
}

TEST(SmallBitset, SingleBit_Set) {
    small_bitset<1> s;
    ASSERT_FALSE(s[0]);
    s.set(0, true);
    ASSERT_TRUE(s[0]);
    s.set(0, false);
    ASSERT_FALSE(s[0]);
}

TEST(SmallBitset, ManyBit_Set) {
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s[i]) << "i: " << i;
        s.set(i, false);
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
}

TEST(SmallBitset, Initialize_False) {
    small_bitset<kNumBits> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }
}

TEST(SmallBitset, UnderlyingType_SizeAdjusted) {
    ASSERT_GT(sizeof(small_bitset<1, std::uint64_t>),
              sizeof(small_bitset<1, std::byte>));
}

TEST(SmallBitset, UnderlyingType_ManyBit_Set) {
    small_bitset<kNumBits, std::uint64_t> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s[i]) << "i: " << i;
        s.set(i, false);
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
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

TEST(SmallBitset, all_1) {
    small_bitset<1> s;
    s.set(0, false);
    EXPECT_FALSE(s.all());
    s.set(0, true);
    EXPECT_TRUE(s.all());
}

TEST(SmallBitset, all_9) {
    small_bitset<9, uint8_t> s;
    for (auto i = 0; i < s.size(); ++i)
        s.set(i, true);
    EXPECT_TRUE(s.all());
}
