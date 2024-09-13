
#include "../small_bitset.hpp"

#include <gtest/gtest.h>

using nonstd::small_bitset;

TEST(SmallBitset, SingleBit_Set) {
    small_bitset<1> s;
    s.set(0, true);
    EXPECT_TRUE(s[0]);
    s.set(0, false);
    EXPECT_FALSE(s[0]);
}

TEST(SmallBitset, ManyBit_Set) {
    static constexpr std::size_t kNumBits{128};
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_TRUE(s[i]);
        s.set(i, false);
        EXPECT_FALSE(s[i]);
    }
}

TEST(SmallBitset, UnderlyingType_SizeAdjusted) {
    EXPECT_GT(sizeof(small_bitset<1, std::uint64_t>),
              sizeof(small_bitset<1, std::byte>));
}

TEST(SmallBitset, UnderlyingType_ManyBit_Set) {
    static constexpr std::size_t kNumBits{128};
    small_bitset<kNumBits, std::uint64_t> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_TRUE(s[i]);
        s.set(i, false);
        EXPECT_FALSE(s[i]);
    }
}

TEST(SmallBitset, test) {
    static constexpr std::size_t kNumBits{128};
    small_bitset<kNumBits> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_TRUE(s.test(i));
        s.set(i, false);
        EXPECT_FALSE(s.test(i));
    }
}
