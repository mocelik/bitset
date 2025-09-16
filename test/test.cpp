#include "../small_bitset.hpp"
#include <algorithm>
#include <gtest/gtest.h>
#include <stdexcept>

using nonstd::small_bitset;

namespace {
static constexpr std::size_t kNumBits{128};
}

// Compile-time tests
static_assert(sizeof(small_bitset<1, std::uint16_t>) > sizeof(small_bitset<1, std::uint8_t>),
              "Smaller Underlying type did not result in smaller object");

TEST(SmallBitset, constructor_default) {
    constexpr small_bitset<kNumBits> s; // constexpr
    static_assert(noexcept(small_bitset<kNumBits>()), "Default constructor is not noexcept");
}

TEST(SmallBitset, constructor_unsignedlonglong) {
    constexpr small_bitset<kNumBits> s_1(1);
    ASSERT_TRUE(s_1[0]);

    constexpr small_bitset<kNumBits> s_all(~0ull);
    for (auto i = 0; i < 8 * sizeof(unsigned long long); i++) {
        ASSERT_TRUE(s_all[i]);
    }
    for (auto i = 8 * sizeof(unsigned long long); i < kNumBits; i++) {
        ASSERT_FALSE(s_all[i]);
    }

    constexpr small_bitset<1> s_overflow(~0ull);
    ASSERT_TRUE(s_overflow[0]);
}

TEST(SmallBitset, constructor_string) {
    std::string data("110010");
    small_bitset<kNumBits> s(data); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    for (auto i = 6; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i << ", s: " << s;
    }

    small_bitset<kNumBits> s_offset(data, 2); // 0010
    ASSERT_FALSE(s_offset[0]) << s_offset;
    ASSERT_TRUE(s_offset[1]) << s_offset;
    ASSERT_FALSE(s_offset[2]) << s_offset;
    ASSERT_FALSE(s_offset[3]) << s_offset;
    for (auto i = 4; i < s_offset.size(); i++) {
        ASSERT_FALSE(s_offset[i]) << "i: " << i << ", s: " << s_offset;
    }

    small_bitset<kNumBits> s_offset_size(data, 2, 3); // 001
    ASSERT_TRUE(s_offset_size[0]);
    for (auto i = 1; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    ASSERT_THROW(small_bitset<kNumBits>(std::string("01X10")), std::invalid_argument);

    // There should not be an exception thrown if the invalid value is out of range
    ASSERT_NO_THROW(small_bitset<kNumBits>(std::string("01X10"), 0, 2));
}

TEST(SmallBitset, constructor_charptr) {
    constexpr small_bitset<kNumBits> s("110010"); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    for (auto i = 6; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i << ", s: " << s;
    }

    constexpr small_bitset<kNumBits> s_offset("110010", 4); // 0010
    ASSERT_FALSE(s_offset[0]) << s_offset;
    ASSERT_TRUE(s_offset[1]) << s_offset;
    ASSERT_FALSE(s_offset[2]) << s_offset;
    ASSERT_FALSE(s_offset[3]) << s_offset;
    for (auto i = 4; i < s_offset.size(); i++) {
        ASSERT_FALSE(s_offset[i]) << "i: " << i << ", s: " << s_offset;
    }

    constexpr small_bitset<kNumBits> s_offset_size("11001", 3); // 001
    ASSERT_TRUE(s_offset_size[0]);
    for (auto i = 1; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    constexpr small_bitset<kNumBits> s_alt_char("XXOOXO", 6, 'O', 'X'); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    for (auto i = 6; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    ASSERT_THROW(small_bitset<kNumBits>("01X10"), std::invalid_argument);

    // There should not be an exception thrown if the invalid value is out of range
    ASSERT_NO_THROW(small_bitset<kNumBits>("01X10", 2));
}

TEST(SmallBitset, bracket_operator) {
    small_bitset<kNumBits> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
}

TEST(SmallBitset, reference_operator_eq_bool) {
    small_bitset<kNumBits> s;
    s[0] = true;
    ASSERT_TRUE(s[0]);
    s[0] = false;
    ASSERT_FALSE(s[0]);

    s[2] = true;
    ASSERT_TRUE(s[2]);
    s[2] = false;
    ASSERT_FALSE(s[2]);

    s[8] = true;
    ASSERT_TRUE(s[8]);
    s[8] = false;
    ASSERT_FALSE(s[8]);

    s[kNumBits - 1] = true;
    ASSERT_TRUE(s[kNumBits - 1]);
    s[kNumBits - 1] = false;
    ASSERT_FALSE(s[kNumBits - 1]);

    ASSERT_EQ(s.count(), 0);
}

TEST(SmallBitset, reference_operator_eq_reference) {
    small_bitset<kNumBits> s;
    s[0] = true;

    ASSERT_TRUE(s[0]);

    s[2] = s[0];
    ASSERT_TRUE(s[2]);
    s[2] = s[1];
    ASSERT_FALSE(s[2]);

    s[8] = s[0];
    ASSERT_TRUE(s[8]);
    s[8] = s[1];
    ASSERT_FALSE(s[8]);

    s[kNumBits - 1] = s[0];
    ASSERT_TRUE(s[kNumBits - 1]);
    s[kNumBits - 1] = s[1];
    ASSERT_FALSE(s[kNumBits - 1]);

    ASSERT_EQ(s.count(), 1);
}

TEST(SmallBitset, reference_operator_not) {
    small_bitset<kNumBits> s;
    ASSERT_TRUE(~s[0]);
}

TEST(SmallBitset, reference_operator_flip) {
    small_bitset<kNumBits> s;
    ASSERT_TRUE(s[0].flip());
    ASSERT_FALSE(s[0].flip());
}


TEST(SmallBitset, initialize_to_false) {
    small_bitset<kNumBits> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }
}

TEST(SmallBitset, set) {
    small_bitset<kNumBits> s;
    s.set();
    ASSERT_EQ(s.count(), kNumBits);
    ASSERT_TRUE(s.all());
}

TEST(SmallBitset, set_pos) {
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
    ASSERT_THROW(s.set(kNumBits, true), std::out_of_range);
}

TEST(SmallBitset, reset) {
    small_bitset<kNumBits> s;
    s.set();
    ASSERT_EQ(s.count(), kNumBits);
    s.reset();
    ASSERT_EQ(s.count(), 0);
    s[0] = true;
    ASSERT_EQ(s[0], true);
    s.reset(0);
    ASSERT_EQ(s[0], false);

    ASSERT_THROW(s.reset(kNumBits), std::out_of_range);
}

TEST(SmallBitset, flip_all) {
    small_bitset<kNumBits> s;

    ASSERT_TRUE(s.none());
    s.flip();
    ASSERT_TRUE(s.all());
    s.flip();
    ASSERT_FALSE(s.all());

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

    ASSERT_THROW(s.test(kNumBits), std::out_of_range);
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

TEST(SmallBitset, bitshift_left_first_bit) {
    for (auto i = 0; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(0);
        s = s << i;
        ASSERT_TRUE(s[i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TEST(SmallBitset, bitshift_left_nth_bit) {
    for (auto i = 0; i < kNumBits - 1; i++) {
        small_bitset<kNumBits> s;
        s.set(i);
        s = s << 1;
        ASSERT_TRUE(s[i+1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    small_bitset<kNumBits> s;
    s.set(kNumBits - 1);
    s = s << 1;
    ASSERT_TRUE(s.none());
}

TEST(SmallBitset, bitshift_left_multiple_bits) {
    for (auto i = 0; i < 8; i++) {
        small_bitset<kNumBits> s;
        constexpr auto kNumUnderlying = kNumBits/(8*sizeof(uint8_t));

        // Set the i'th bit in each word
        for (auto j = 0; j < kNumUnderlying; j++) {
            s.set(i+(j*8));
        }
        ASSERT_EQ(s.count(), kNumUnderlying);

        s = s << 1;

        // Check the i+1'th bit in each word
        for (auto j = 0; j < (i == 7 ? kNumUnderlying - 1 : kNumUnderlying); j++) {
            ASSERT_TRUE(s[i+(j*8) + 1]);
            ASSERT_FALSE(s[i+(j*8)]);
        }

        // Verify that extraneous bits were not set
        if (i != 7) {
            ASSERT_EQ(s.count(), kNumUnderlying);
        } else {
            ASSERT_EQ(s.count(), kNumUnderlying - 1); // the leftmost 1 was discarded
        }
    }
}


TEST(SmallBitset, bitshift_left_first_bit_assignment) {
    for (auto i = 0; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(0);
        s <<= i;
        ASSERT_TRUE(s[i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TEST(SmallBitset, bitshift_left_nth_bit_assignment) {
    for (auto i = 0; i < kNumBits - 1; i++) {
        small_bitset<kNumBits> s;
        s.set(i);
        s <<= 1;
        ASSERT_TRUE(s[i+1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    small_bitset<kNumBits> s;
    s.set(kNumBits - 1);
    s <<= 1;
    ASSERT_TRUE(s.none());
}

TEST(SmallBitset, bitshift_left_multiple_bits_assignment) {
    for (auto i = 0; i < 8; i++) {
        small_bitset<kNumBits> s;
        constexpr auto kNumUnderlying = kNumBits/(8*sizeof(uint8_t));

        // Set the i'th bit in each word
        for (auto j = 0; j < kNumUnderlying; j++) {
            s.set(i+(j*8));
        }
        ASSERT_EQ(s.count(), kNumUnderlying);

        s <<= 1;

        // Check the i+1'th bit in each word
        for (auto j = 0; j < (i == 7 ? kNumUnderlying - 1 : kNumUnderlying); j++) {
            ASSERT_TRUE(s[i+(j*8) + 1]);
            ASSERT_FALSE(s[i+(j*8)]);
        }

        // Verify that extraneous bits were not set
        if (i != 7) {
            ASSERT_EQ(s.count(), kNumUnderlying);
        } else {
            ASSERT_EQ(s.count(), kNumUnderlying - 1); // the leftmost 1 was discarded
        }
    }
}

TEST(SmallBitset, bitshift_right_last_bit) {
    for (auto i = 0; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(kNumBits - 1);
        s = s >> i;
        ASSERT_TRUE(s[(kNumBits - 1) - i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TEST(SmallBitset, bitshift_right_nth_bit) {
    for (auto i = 1; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(i);
        s = s >> 1;
        ASSERT_TRUE(s[i-1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    small_bitset<kNumBits> s;
    s.set(0);
    s = s >> 1;
    ASSERT_TRUE(s.none());
}

TEST(SmallBitset, bitshift_right_multiple_bits) {
    for (auto i = 0; i < 8; i++) {
        small_bitset<kNumBits> s;
        constexpr auto kNumUnderlying = kNumBits/(8*sizeof(uint8_t));

        // Set the i'th bit in each word
        for (auto j = 0; j < kNumUnderlying; j++) {
            s.set(i+(j*8));
        }
        ASSERT_EQ(s.count(), kNumUnderlying);

        s = s >> 1;

        // Check the i-1'th bit in each word
        for (auto j = i == 0 ? 1 : 0; j < kNumUnderlying; j++) {
            ASSERT_TRUE(s[(i+(j*8)) - 1]);
            ASSERT_FALSE(s[i+(j*8)]);
        }

        // Verify that extraneous bits were not set
        if (i != 0) {
            ASSERT_EQ(s.count(), kNumUnderlying);
        } else {
            ASSERT_EQ(s.count(), kNumUnderlying - 1); // the rightmost 1 was discarded
        }
    }
}

TEST(SmallBitset, bitshift_right_first_bit_assignment) {
    for (auto i = 0; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(kNumBits - 1);
        s >>= i;
        ASSERT_TRUE(s[(kNumBits - 1) - i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TEST(SmallBitset, bitshift_right_nth_bit_assignment) {
    for (auto i = 1; i < kNumBits; i++) {
        small_bitset<kNumBits> s;
        s.set(i);
        s >>= 1;
        ASSERT_TRUE(s[i-1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    small_bitset<kNumBits> s;
    s.set(0);
    s >>= 1;
    ASSERT_TRUE(s.none());
}

TEST(SmallBitset, bitshift_right_multiple_bits_assignment) {
    for (auto i = 0; i < 8; i++) {
        small_bitset<kNumBits> s;
        constexpr auto kNumUnderlying = kNumBits/(8*sizeof(uint8_t));

        // Set the i'th bit in each word
        for (auto j = 0; j < kNumUnderlying; j++) {
            s.set(i+(j*8));
        }
        ASSERT_EQ(s.count(), kNumUnderlying);

        s >>= 1;

        // Check the i-1'th bit in each word
        for (auto j = i == 0 ? 1 : 0; j < kNumUnderlying; j++) {
            ASSERT_TRUE(s[(i+(j*8)) - 1]);
            ASSERT_FALSE(s[i+(j*8)]);
        }

        // Verify that extraneous bits were not set
        if (i != 0) {
            ASSERT_EQ(s.count(), kNumUnderlying);
        } else {
            ASSERT_EQ(s.count(), kNumUnderlying - 1); // the rightmost 1 was discarded
        }
    }
}

TEST(SmallBitset, operator_equals_1) {
    small_bitset<1> s1(1);
    small_bitset<1> s2(1);
    small_bitset<1> s3(0);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
}

TEST(SmallBitset, operator_equals_16) {
    small_bitset<16> s1(1 << 8 | 1);
    small_bitset<16> s2(1 << 8 | 1);
    small_bitset<16> s3(1);
    small_bitset<16> s4(1 << 8);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
    ASSERT_NE(s1, s4);
    ASSERT_NE(s3, s4);
}

TEST(SmallBitset, operator_equals_9) {
    small_bitset<9> s1(1 << 8 | 1);
    small_bitset<9> s2(1 << 8 | 1);
    small_bitset<9> s3(1);
    small_bitset<9> s4(1 << 8);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
    ASSERT_NE(s1, s4);
    ASSERT_NE(s3, s4);
}

TEST(SmallBitset, operator_equals_10) {
    small_bitset<10> s1;
    small_bitset<10> s2;
    ASSERT_EQ(s1.flip(), s2.set());
}

TEST(SmallBitset, to_string) {
    std::string data("101010101");
    data.insert(0, kNumBits - data.size(), '0');

    small_bitset<kNumBits> s(data);
    ASSERT_EQ(data, s.to_string());

    std::string modified_data(data);
    std::replace(modified_data.begin(), modified_data.end(), '1', 'X');
    std::replace(modified_data.begin(), modified_data.end(), '0', 'O');
    ASSERT_EQ(modified_data, s.to_string('O','X'));
}

TEST(SmallBitset, to_ulong) {
    constexpr auto kNumBitsInUnsignedLong = 8 * sizeof(unsigned long);
    constexpr auto value = 1ul << (kNumBitsInUnsignedLong - 1);
    small_bitset<kNumBits> s(value);
    ASSERT_EQ(s.to_ulong(), value) << s;

    s.flip();
    static_assert(kNumBits > kNumBitsInUnsignedLong);
    ASSERT_THROW(s.to_ulong(), std::overflow_error);
}

TEST(SmallBitset, to_ullong) {
    constexpr auto kNumBitsInUnsignedLongLong = 8 * sizeof(unsigned long long);
    constexpr auto value = 1ul << (kNumBitsInUnsignedLongLong - 1);
    small_bitset<kNumBits> s(value);
    ASSERT_EQ(s.to_ullong(), value) << s;

    s.flip();
    static_assert(kNumBits > kNumBitsInUnsignedLongLong);
    ASSERT_THROW(s.to_ullong(), std::overflow_error);
}
