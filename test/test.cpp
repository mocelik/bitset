#include <bitset.hpp>
#include <algorithm>
#include <gtest/gtest.h>
#include <stdexcept>
#include <sstream>

using nonstd::bitset;

namespace {
constexpr std::size_t kNumBits{128};
}

// Compile-time tests
static_assert(sizeof(bitset<1, std::uint16_t>) > sizeof(bitset<1, std::uint8_t>),
              "Smaller Underlying type did not result in smaller object");

template <class T>
class Bitset : public testing::Test {};

using UnsignedTypes = ::testing::Types<std::uint8_t, std::uint16_t, std::uint32_t, std::uint64_t>;
TYPED_TEST_SUITE(Bitset, UnsignedTypes);

TYPED_TEST(Bitset, constructor_default) {
    static_assert(noexcept(bitset<kNumBits, TypeParam>()), "Default constructor is not noexcept");
}

TYPED_TEST(Bitset, constructor_unsignedlonglong) {
    constexpr bitset<kNumBits, TypeParam> s_1(1);
    ASSERT_TRUE(s_1[0]);

    constexpr bitset<kNumBits, TypeParam> s_all(~0ull);
    for (auto i = 0; i < 8 * sizeof(unsigned long long); i++) {
        ASSERT_TRUE(s_all[i]);
    }
    for (auto i = 8 * sizeof(unsigned long long); i < kNumBits; i++) {
        ASSERT_FALSE(s_all[i]);
    }

    constexpr bitset<1, TypeParam> s_overflow(~0ull);
    ASSERT_TRUE(s_overflow[0]);
}

TYPED_TEST(Bitset, constructor_string) {
    std::string data("110010");
    ASSERT_THROW((bitset<kNumBits, TypeParam>(data, data.size() + 1)), std::out_of_range);

    bitset<kNumBits, TypeParam> s(data); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    EXPECT_EQ(s.count(), 3); // three ones
    for (auto i = 6; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i << ", s: " << s;
    }

    bitset<kNumBits, TypeParam> s_offset(data, 2); // 0010
    ASSERT_FALSE(s_offset[0]) << s_offset;
    ASSERT_TRUE(s_offset[1]) << s_offset;
    ASSERT_FALSE(s_offset[2]) << s_offset;
    ASSERT_FALSE(s_offset[3]) << s_offset;
    for (auto i = 4; i < s_offset.size(); i++) {
        ASSERT_FALSE(s_offset[i]) << "i: " << i << ", s: " << s_offset;
    }

    bitset<kNumBits, TypeParam> s_offset_size(data, 2, 3); // 001
    ASSERT_TRUE(s_offset_size[0]);
    for (auto i = 1; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    ASSERT_THROW((bitset<kNumBits, TypeParam>(std::string("01X10"))), std::invalid_argument);

    // There should not be an exception thrown if the invalid value is out of range
    ASSERT_NO_THROW((bitset<kNumBits, TypeParam>(std::string("01X10"), 0, 2)));
}

TYPED_TEST(Bitset, constructor_charptr) {
    constexpr bitset<kNumBits, TypeParam> s("110010"); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    for (auto i = 6; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i << ", s: " << s;
    }

    constexpr bitset<kNumBits, TypeParam> s_offset("110010", 4); // 0010
    ASSERT_FALSE(s_offset[0]) << s_offset;
    ASSERT_TRUE(s_offset[1]) << s_offset;
    ASSERT_FALSE(s_offset[2]) << s_offset;
    ASSERT_FALSE(s_offset[3]) << s_offset;
    for (auto i = 4; i < s_offset.size(); i++) {
        ASSERT_FALSE(s_offset[i]) << "i: " << i << ", s: " << s_offset;
    }

    constexpr bitset<kNumBits, TypeParam> s_offset_size("11001", 3); // 001
    ASSERT_TRUE(s_offset_size[0]);
    for (auto i = 1; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    constexpr bitset<kNumBits, TypeParam> s_alt_char("XXOOXO", 6, 'O', 'X'); // 110010
    ASSERT_FALSE(s[0]) << s;
    ASSERT_TRUE(s[1]) << s;
    ASSERT_FALSE(s[2]) << s;
    ASSERT_FALSE(s[3]) << s;
    ASSERT_TRUE(s[4]) << s;
    ASSERT_TRUE(s[5]) << s;
    for (auto i = 6; i < s_offset_size.size(); i++) {
        ASSERT_FALSE(s_offset_size[i]) << "i: " << i << ", s: " << s_offset_size;
    }

    ASSERT_THROW((bitset<kNumBits, TypeParam>("01X10")), std::invalid_argument);

    // There should not be an exception thrown if the invalid value is out of range
    ASSERT_NO_THROW((bitset<kNumBits, TypeParam>("01X10", 2)));
}

TYPED_TEST(Bitset, bracket_operator) {
    bitset<kNumBits, TypeParam> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s[i]) << "i: " << i;
    }
}

TYPED_TEST(Bitset, reference_operator_eq_bool) {
    bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, reference_operator_eq_reference) {
    bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, reference_operator_not) {
    bitset<kNumBits, TypeParam> s;
    ASSERT_TRUE(~s[0]);
}

TYPED_TEST(Bitset, reference_operator_flip) {
    bitset<kNumBits, TypeParam> s;
    ASSERT_TRUE(s[0].flip());
    ASSERT_FALSE(s[0].flip());
}


TYPED_TEST(Bitset, initialize_to_false) {
    bitset<kNumBits, TypeParam> s;
    for (std::size_t i = 0; i < s.size(); i++) {
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }
}

TYPED_TEST(Bitset, set) {
    bitset<kNumBits, TypeParam> s;
    s.set();
    ASSERT_EQ(s.count(), kNumBits);
    ASSERT_TRUE(s.all());
}

TYPED_TEST(Bitset, set_pos) {
    bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, reset) {
    bitset<kNumBits, TypeParam> s;
    s.set();
    ASSERT_EQ(s.count(), kNumBits);
    s.reset();
    ASSERT_EQ(s.count(), 0);
    s[0] = true;
    ASSERT_EQ(s[0], true);
    s.reset(0);
    ASSERT_EQ(s[0], false);

    ASSERT_THROW(s.reset(kNumBits), std::out_of_range);
    ASSERT_THROW(s.reset(kNumBits+1), std::out_of_range);
}

TYPED_TEST(Bitset, flip_all) {
    bitset<kNumBits, TypeParam> s;

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

TYPED_TEST(Bitset, flip_index) {
    bitset<kNumBits, TypeParam> s;

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

TYPED_TEST(Bitset, test) {
    bitset<kNumBits, TypeParam> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s.test(i)) << "i: " << i;
        s.set(i, false);
        ASSERT_FALSE(s.test(i)) << "i: " << i;
    }

    ASSERT_THROW(s.test(kNumBits), std::out_of_range);
}

TYPED_TEST(Bitset, count) {
    bitset<kNumBits, TypeParam> s;
    ASSERT_EQ(s.count(), 0);
    for (std::size_t i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_EQ(s.count(), i + 1) << "i: " << i;
    }
}

TYPED_TEST(Bitset, size) {
    bitset<1, TypeParam> s_1;
    static_assert(s_1.size() == 1);

    bitset<8, TypeParam> s_8;
    static_assert(s_8.size() == 8);

    bitset<9, TypeParam> s_9;
    static_assert(s_9.size() == 9);

    bitset<64, TypeParam> s_64;
    static_assert(s_64.size() == 64);

    bitset<129, TypeParam> s_129;
    static_assert(s_129.size() == 129);
}

TYPED_TEST(Bitset, all) {
    bitset<kNumBits, TypeParam> s;
    for (auto i = 0; i < s.size() - 1; ++i) {
        s.set(i, true);
        ASSERT_FALSE(s.all());
    }
    s.set(s.size() -1, true);
    ASSERT_TRUE(s.all());
}

TYPED_TEST(Bitset, any) {
    bitset<kNumBits, TypeParam> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_TRUE(s.any()) << "i: " << i;
        s.set(i, false);
        ASSERT_FALSE(s.any()) << "i: " << i;
    }
}

TYPED_TEST(Bitset, none) {
    bitset<kNumBits, TypeParam> s;
    for (int i = 0; i < s.size(); i++) {
        s.set(i, true);
        ASSERT_FALSE(s.none()) << "i: " << i;
        s.set(i, false);
        ASSERT_TRUE(s.none()) << "i: " << i;
    }
}

TYPED_TEST(Bitset, bitwise_and) {
    bitset<kNumBits, TypeParam> s1;
    bitset<kNumBits, TypeParam> s2;

    s1.set(0);
    s2.set(0);

    s1.set(1);
    s1 &= s2;
    ASSERT_TRUE(s1[0]);
    ASSERT_FALSE(s1[1]);
}

TYPED_TEST(Bitset, bitwise_or) {
    bitset<kNumBits, TypeParam> s1;
    bitset<kNumBits, TypeParam> s2;

    s1.set(0);
    s2.set(1);

    s1 |= s2;
    ASSERT_TRUE(s1[0]);
    ASSERT_TRUE(s1[1]);
    ASSERT_FALSE(s1[2]);
}

TYPED_TEST(Bitset, bitwise_xor) {
    bitset<kNumBits, TypeParam> s1;
    bitset<kNumBits, TypeParam> s2;

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

TYPED_TEST(Bitset, bitwise_not) {
    bitset<kNumBits, TypeParam> s1;

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

TYPED_TEST(Bitset, bitshift_left_first_bit) {
    for (auto i = 0; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(0);
        s = s << i;
        ASSERT_TRUE(s[i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TYPED_TEST(Bitset, bitshift_left_nth_bit) {
    for (auto i = 0; i < kNumBits - 1; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(i);
        s = s << 1;
        ASSERT_TRUE(s[i+1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    bitset<kNumBits, TypeParam> s;
    s.set(kNumBits - 1);
    s = s << 1;
    ASSERT_TRUE(s.none());
}

TYPED_TEST(Bitset, bitshift_left_multiple_bits) {
    for (auto i = 0; i < 8; i++) {
        bitset<kNumBits, TypeParam> s;
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


TYPED_TEST(Bitset, bitshift_left_first_bit_assignment) {
    for (auto i = 0; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(0);
        s <<= i;
        ASSERT_TRUE(s[i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TYPED_TEST(Bitset, bitshift_left_nth_bit_assignment) {
    for (auto i = 0; i < kNumBits - 1; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(i);
        s <<= 1;
        ASSERT_TRUE(s[i+1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    bitset<kNumBits, TypeParam> s;
    s.set(kNumBits - 1);
    s <<= 1;
    ASSERT_TRUE(s.none());
}

TYPED_TEST(Bitset, bitshift_left_multiple_bits_assignment) {
    for (auto i = 0; i < 8; i++) {
        bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, bitshift_right_last_bit) {
    for (auto i = 0; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(kNumBits - 1);
        s = s >> i;
        ASSERT_TRUE(s[(kNumBits - 1) - i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TYPED_TEST(Bitset, bitshift_right_nth_bit) {
    for (auto i = 1; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(i);
        s = s >> 1;
        ASSERT_TRUE(s[i-1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    bitset<kNumBits, TypeParam> s;
    s.set(0);
    s = s >> 1;
    ASSERT_TRUE(s.none());
}

TYPED_TEST(Bitset, bitshift_right_multiple_bits) {
    for (auto i = 0; i < 8; i++) {
        bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, bitshift_right_first_bit_assignment) {
    for (auto i = 0; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(kNumBits - 1);
        s >>= i;
        ASSERT_TRUE(s[(kNumBits - 1) - i]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }
}

TYPED_TEST(Bitset, bitshift_right_nth_bit_assignment) {
    for (auto i = 1; i < kNumBits; i++) {
        bitset<kNumBits, TypeParam> s;
        s.set(i);
        s >>= 1;
        ASSERT_TRUE(s[i-1]) << "i = " << i << ", bitset: " << s;
        ASSERT_EQ(s.count(), 1) << "i = " << i << ", bitset: " << s;
    }

    bitset<kNumBits, TypeParam> s;
    s.set(0);
    s >>= 1;
    ASSERT_TRUE(s.none());
}

TYPED_TEST(Bitset, bitshift_right_multiple_bits_assignment) {
    for (auto i = 0; i < 8; i++) {
        bitset<kNumBits, TypeParam> s;
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

TYPED_TEST(Bitset, operator_equals_1) {
    bitset<1, TypeParam> s1(1);
    bitset<1, TypeParam> s2(1);
    bitset<1, TypeParam> s3(0);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
}

TYPED_TEST(Bitset, operator_equals_16) {
    bitset<16, TypeParam> s1(1 << 8 | 1);
    bitset<16, TypeParam> s2(1 << 8 | 1);
    bitset<16, TypeParam> s3(1);
    bitset<16, TypeParam> s4(1 << 8);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
    ASSERT_NE(s1, s4);
    ASSERT_NE(s3, s4);
}

TYPED_TEST(Bitset, operator_equals_9) {
    bitset<9, TypeParam> s1(1 << 8 | 1);
    bitset<9, TypeParam> s2(1 << 8 | 1);
    bitset<9, TypeParam> s3(1);
    bitset<9, TypeParam> s4(1 << 8);
    ASSERT_EQ(s1, s2);
    ASSERT_NE(s1, s3);
    ASSERT_NE(s1, s4);
    ASSERT_NE(s3, s4);
}

TYPED_TEST(Bitset, operator_equals_10) {
    bitset<10, TypeParam> s1;
    bitset<10, TypeParam> s2;
    ASSERT_EQ(s1.flip(), s2.set());
}

TYPED_TEST(Bitset, to_string) {
    std::string data("101010101");
    data.insert(0, kNumBits - data.size(), '0');

    bitset<kNumBits, TypeParam> s(data);
    ASSERT_EQ(data, s.to_string());

    std::string modified_data(data);
    std::replace(modified_data.begin(), modified_data.end(), '1', 'X');
    std::replace(modified_data.begin(), modified_data.end(), '0', 'O');
    ASSERT_EQ(modified_data, s.to_string('O','X'));
}

TYPED_TEST(Bitset, to_ulong) {
    constexpr auto kNumBitsInUnsignedLong = 8 * sizeof(unsigned long);
    constexpr auto value = 1ul << (kNumBitsInUnsignedLong - 1);
    bitset<kNumBits, TypeParam> s(value);
    ASSERT_EQ(s.to_ulong(), value) << s;

    s.flip();
    static_assert(kNumBits > kNumBitsInUnsignedLong);
    ASSERT_THROW(s.to_ulong(), std::overflow_error);
}

TYPED_TEST(Bitset, to_ullong) {
    constexpr auto kNumBitsInUnsignedLongLong = 8 * sizeof(unsigned long long);
    constexpr auto value = 1ul << (kNumBitsInUnsignedLongLong - 1);
    bitset<kNumBits, TypeParam> s(value);
    ASSERT_EQ(s.to_ullong(), value) << s;

    s.flip();
    static_assert(kNumBits > kNumBitsInUnsignedLongLong);
    ASSERT_THROW(s.to_ullong(), std::overflow_error);
}

TYPED_TEST(Bitset, free_operator_bitand) {
    constexpr bitset<kNumBits, TypeParam> zero;
    constexpr bitset<kNumBits, TypeParam> ones{~zero};

    ASSERT_EQ(zero & zero, zero);
    ASSERT_EQ(zero & ones, zero);
    ASSERT_EQ(ones & zero, zero);
    ASSERT_EQ(ones & ones, ones);

    constexpr bitset<kNumBits, TypeParam> s1{1 | 1 << 8};
    constexpr bitset<kNumBits, TypeParam> s2{1 | 1 << 8 | 1 << 16};
    ASSERT_EQ(s1 & s2, s1);
    ASSERT_EQ(s1 & ones, s1);
    ASSERT_EQ(s1 & zero, zero);
}


TYPED_TEST(Bitset, free_operator_bitor) {
    constexpr bitset<kNumBits, TypeParam> zero;
    constexpr bitset<kNumBits, TypeParam> ones{~zero};

    ASSERT_EQ(zero | zero, zero);
    ASSERT_EQ(zero | ones, ones);
    ASSERT_EQ(ones | zero, ones);
    ASSERT_EQ(ones | ones, ones);

    constexpr bitset<kNumBits, TypeParam> s1{1 | 1 << 8};
    constexpr bitset<kNumBits, TypeParam> s2{1 | 1 << 8 | 1 << 16};
    ASSERT_EQ(s1 | s2, s2);
    ASSERT_EQ(s1 | ones, ones);
    ASSERT_EQ(s1 | zero, s1);
}

TYPED_TEST(Bitset, free_operator_bitxor) {
    constexpr bitset<kNumBits, TypeParam> zero;
    constexpr bitset<kNumBits, TypeParam> ones{~zero};

    ASSERT_EQ(zero ^ zero, zero);
    ASSERT_EQ(zero ^ ones, ones);
    ASSERT_EQ(ones ^ zero, ones);
    ASSERT_EQ(ones ^ ones, zero);

    constexpr bitset<kNumBits, TypeParam> s1{1 | 1 << 8};
    constexpr bitset<kNumBits, TypeParam> s2{ 1 << 8 | 1 << 16};
    ASSERT_EQ(s1 ^ s2, 1 | 1 << 16 );
    ASSERT_EQ(s1 ^ ones, ~s1);
    ASSERT_EQ(s1 ^ zero, s1);
}

TYPED_TEST(Bitset, operator_stream_insert_extract){
    std::string data("1111000010101010");
    data.insert(0, kNumBits - data.size(), '0');

    bitset<kNumBits, TypeParam> bits(data);

    std::stringstream ss;
    ss << bits;
    ASSERT_EQ(ss.str(), data);

    bitset<kNumBits, TypeParam> bits2;
    ss >> bits2;
    ASSERT_EQ(bits, bits2);
}

TYPED_TEST(Bitset, operator_stream_extract_invalid){
    std::stringstream ss;
    ss << "10X101"; // The X is invalid

    bitset<3, TypeParam> bitset;
    ss >> bitset; // Read 10X
    ASSERT_TRUE(ss.fail()); // X is invalid

    ss.clear(); // clear the fail bit
    ss >> bitset; // Read 101
    ASSERT_TRUE(ss.good());
    ASSERT_TRUE(bitset[0]);
    ASSERT_FALSE(bitset[1]);
    ASSERT_TRUE(bitset[2]);
}

TYPED_TEST(Bitset, operator_stream_extraction_reset){
    std::stringstream ss;
    ss << "101";

    bitset<4, TypeParam> bitset(1 << 3);
    ss >> bitset; // Read 101, zero-extend on left with 0's

    ASSERT_TRUE(bitset[0]) << bitset;
    ASSERT_FALSE(bitset[1]) << bitset;
    ASSERT_TRUE(bitset[2]) << bitset;
    ASSERT_FALSE(bitset[3]) << bitset;
}

TYPED_TEST(Bitset, hash) {
    auto big_hash = std::hash<bitset<kNumBits, TypeParam>>();

    ASSERT_EQ(big_hash(bitset<kNumBits, TypeParam>(1)), big_hash(bitset<kNumBits, TypeParam>(1)));
    ASSERT_NE(big_hash(bitset<kNumBits, TypeParam>(1)), big_hash(bitset<kNumBits, TypeParam>(0)));
    ASSERT_NE(big_hash(bitset<kNumBits, TypeParam>(1)), big_hash(bitset<kNumBits, TypeParam>(2)));

    auto small_hash = std::hash<bitset<32, TypeParam>>();
    ASSERT_EQ(small_hash(bitset<32, TypeParam>(1)), small_hash(bitset<32, TypeParam>(1)));
    ASSERT_NE(small_hash(bitset<32, TypeParam>(1)), small_hash(bitset<32, TypeParam>(0)));
    ASSERT_NE(small_hash(bitset<32, TypeParam>(1)), small_hash(bitset<32, TypeParam>(2)));
}
