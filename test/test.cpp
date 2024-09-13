
#include "../small_bitset.hpp"

#include <gtest/gtest.h>

TEST(SmallBitset, SingleBit_Set) {
    nonstd::small_bitset<1> s;
    s.set(0, true);
    EXPECT_TRUE(s[0]);
    s.set(0, false);
    EXPECT_FALSE(s[0]);
}

TEST(SmallBitset, ManyBit_Set) {
    static constexpr std::size_t kNumBits { 128 };
    nonstd::small_bitset<kNumBits> s;
    for (int i=0; i < s.size(); i++) {
        s.set(i, true);
        EXPECT_TRUE(s[i]);
        s.set(i, false);
        EXPECT_FALSE(s[i]);
    }
}
