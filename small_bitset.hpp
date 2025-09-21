
#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring> // memset
#include <iterator>
#include <stdexcept> // std::out_of_range
#include <type_traits>
#include <ostream>
#include <istream>

namespace nonstd {

template <std::size_t N, typename Underlying = std::uint8_t> class small_bitset {
    static_assert(std::is_unsigned_v<Underlying>,
                  "small_bitset requires an unsigned underlying type");

    using underlying_type_t = Underlying;
    static constexpr std::size_t num_underlying_bits() {
        return 8 * sizeof(underlying_type_t);
    }

    static constexpr std::size_t num_words() {
        return (N + num_underlying_bits() - 1) / num_underlying_bits();
    }

    static constexpr std::size_t underlying_index(std::size_t i) {
        return i / num_underlying_bits();
    }

    underlying_type_t m_data[num_words()] = {underlying_type_t(0)};
    static constexpr underlying_type_t m_last_word_mask = (N % num_underlying_bits() != 0) ? (1 << (N % num_underlying_bits())) - 1 : ~0;

    static constexpr underlying_type_t mask(std::size_t pos) noexcept {
        return underlying_type_t{1} << (pos % num_underlying_bits());
    }

    public:
    constexpr small_bitset() noexcept { reset(); }

    constexpr small_bitset( unsigned long long value ) noexcept {
        reset();
        for (auto i = 0; i < 8 * sizeof value && i < N; i++) {
            if ((1ull << i) & value) {
                set(i, true);
            }
        }
    }

    template< class CharT, class Traits, class Alloc >
    explicit small_bitset( const std::basic_string<CharT, Traits, Alloc>& str,
                            typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
                            typename std::basic_string<CharT, Traits, Alloc>::size_type n = std::basic_string<CharT, Traits, Alloc>::npos,
                            CharT zero = CharT('0'),
                            CharT one = CharT('1') ) {
        if (pos > str.size()) {
            throw std::out_of_range("pos > str.size()");
        }

        auto i = 0;
        auto reverse_start = std::reverse_iterator(n == str.npos ? str.end() : str.begin() + pos + n);
        auto reverse_end = std::reverse_iterator(str.begin() + pos);
        auto iter = reverse_start;
        while (iter != reverse_end) {
            if (Traits::eq(*iter, zero)) {
                set(i, 0);
            } else if (Traits::eq(*iter, one)) {
                set(i, 1);
            } else {
                throw std::invalid_argument(std::string("Unexpected character ") + *iter + " is neither zero (" + zero + ") or one (" + one + ")");
            }
            ++i;
            ++iter;
        }
    }

    template< class CharT >
    constexpr explicit small_bitset( const CharT* str, std::size_t n = std::size_t(-1),
                 CharT zero = CharT('0'), CharT one = CharT('1') ) {
        const auto len = std::char_traits<CharT>::length(str);
        const auto end = str + len;
        if (n == std::basic_string<CharT>::npos || n == std::size_t(-1)) {
            n = len;
        }

        auto iter = std::reverse_iterator(end);
        auto reverse_end = std::reverse_iterator(end - n);

        auto i = 0;
        while (iter != reverse_end) {
            if (*iter == one) {
                set(i, 1);
            } else if (*iter != zero) {
                throw std::invalid_argument(std::string("Unexpected character ") + *iter + " is neither zero (" + zero + ") or one (" + one + ")");
            }
            ++i;
            ++iter;
        }
    }

    class reference {
    public:
        constexpr reference(const reference&) = default;

        constexpr reference& operator=(bool value) noexcept {
            m_parent.set(m_pos, value);
            return *this;
        }
        constexpr reference& operator=(const reference& value) noexcept {
            m_parent.set(m_pos, value);
            return *this;
        }

        constexpr operator bool() const noexcept {
            const small_bitset& parent = m_parent;
            return parent[m_pos];
        }
        constexpr bool operator~() const noexcept {
            const small_bitset& parent = m_parent;
            return !parent[m_pos];
        }

        constexpr reference& flip() noexcept {
            m_parent.set(m_pos, !*this);
            return *this;
        }

    private:
        friend small_bitset;
        constexpr reference(small_bitset& parent, std::size_t pos) noexcept
            : m_parent(parent), m_pos(pos) {}

        small_bitset& m_parent;
        std::size_t m_pos;
    };

    constexpr bool operator[](std::size_t i) const {
        return (m_data[underlying_index(i)] & mask(i)) != underlying_type_t(0);
    }

    constexpr reference operator[](std::size_t i) {
        return reference(*this, i);
    }

    constexpr small_bitset& set() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = 0;
            m_data[i] = ~m_data[i];
        }
        if constexpr (N % 8 != 0) {
            m_data[num_words() - 1] = m_last_word_mask;
        }
        return *this;
    }

    constexpr small_bitset &set(std::size_t pos, bool value = true) {
        if (pos >= N) {
            throw std::out_of_range("bitset::set: pos out of range.");
        }
        if (value) {
            m_data[underlying_index(pos)] |= mask(pos);
        } else {
            m_data[underlying_index(pos)] &= ~mask(pos);
        }
        return *this;
    }

    constexpr small_bitset &flip() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = ~m_data[i];
        }
        if constexpr (N % 8 != 0) {
            m_data[num_words() - 1] &= m_last_word_mask;
        }
        return *this;
    }

    constexpr small_bitset &flip(std::size_t pos) {
        if (pos >= size()) {
            throw std::out_of_range("bitset::flip: __position (which is " + std::to_string(pos) + " >= _Nb (which is " + std::to_string(N) + ")" );
        }

        set(pos, !this->test(pos));
        return *this;
    }

    bool test(std::size_t pos) const {
        if (pos >= N) {
            throw std::out_of_range("bitset::test: pos out of range");
        }
        return this->operator[](pos);
    }

    constexpr std::size_t count() const noexcept {
        std::size_t cnt{0};
        for (std::size_t i = 0; i < size(); ++i) {
            if (test(i))
                ++cnt;
        }
        return cnt;
    }

    constexpr std::size_t size() const noexcept { return N; }

    constexpr bool all() const noexcept {
        constexpr underlying_type_t all_ones = ~underlying_type_t{0};
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (std::memcmp(&m_data[i], &all_ones, sizeof all_ones) != 0) {
                return false;
            }
        }
        return m_data[num_words() - 1] == m_last_word_mask;
    }

    constexpr bool any() const noexcept {
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (m_data[i] != underlying_type_t{0}) {
                return true;
            }
        }
        return m_data[num_words() - 1] & m_last_word_mask;
    }

    constexpr bool none() const noexcept {
        constexpr underlying_type_t all_zeros{0};
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (std::memcmp(&m_data[i], &all_zeros, sizeof all_zeros) != 0) {
                return false;
            }
        }
        return m_data[num_words() - 1] == 0;
    }

    constexpr small_bitset &reset() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = 0;
        }
        return *this;
    }

    constexpr small_bitset &reset(std::size_t pos) {
        if (pos >= N) {
            throw std::out_of_range("bitset::reset: pos out of range");
        }
        return set(pos, false);
    }


    constexpr small_bitset& operator&=( const small_bitset& other ) noexcept {
        for (auto i=0; i < num_words(); i++) {
            m_data[i] &= other.m_data[i];
        }
        return *this;
    }

    constexpr small_bitset& operator|=( const small_bitset& other ) noexcept {
        for (auto i=0; i < num_words(); i++) {
            m_data[i] |= other.m_data[i];
        }
        return *this;
    }

    constexpr small_bitset& operator^=( const small_bitset& other ) noexcept{
        for (auto i=0; i < num_words(); i++) {
            m_data[i] ^= other.m_data[i];
        }
        return *this;
    }

    constexpr small_bitset operator~() const noexcept {
        small_bitset other;
        for (auto i = 0; i < num_words(); i++) {
            other.m_data[i] = ~m_data[i];
        }
        return other;
    }

    constexpr small_bitset operator<<( std::size_t shift ) const noexcept {
        return small_bitset(*this) <<= shift;
    }

    constexpr small_bitset& operator<<=( std::size_t shift ) noexcept {
        if (shift == 0) {
            return *this;
        }

        const auto num_words_to_shift = shift / num_underlying_bits();
        const auto num_bits_to_shift = shift % num_underlying_bits();

        // Handle the words between the most significant and the least significant
        if (num_bits_to_shift == 0 || num_bits_to_shift == num_underlying_bits()) {
            for (auto i = num_words() - 1; i > num_words_to_shift; i--) {
                m_data[i]  = m_data[(i - num_words_to_shift) - 0];
            }
            m_data[num_words_to_shift] = m_data[0];
        } else {
            for (auto i = num_words() - 1; i > num_words_to_shift; i--) {
                m_data[i]  = m_data[(i - num_words_to_shift) - 0] << num_bits_to_shift;
                m_data[i] |= m_data[(i - num_words_to_shift) - 1] >> (num_underlying_bits() - num_bits_to_shift);
            }
            m_data[num_words_to_shift] = m_data[0] << num_bits_to_shift;
        }

        // zero-fill from the right
        for (auto i = 0; i < num_words_to_shift; i++) {
            m_data[i] = 0;
        }

        return *this;
    }

    constexpr small_bitset operator>>( std::size_t shift ) const noexcept {
        return small_bitset(*this) >>= shift;
    }

    constexpr small_bitset& operator>>=( std::size_t shift ) noexcept {
        if (shift == 0) {
            return *this;
        }
        const auto num_words_to_shift = shift / num_underlying_bits();
        const auto num_bits_to_shift = shift % num_underlying_bits();

        if (num_bits_to_shift == 0 || num_bits_to_shift == num_underlying_bits()) {
            for (auto i = 0; i < (num_words() - 1) - num_words_to_shift; i++) {
                m_data[i]  = m_data[(i + num_words_to_shift) + 0];
            }
            m_data[(num_words() - 1) - num_words_to_shift] = m_data[(num_words() - 1)];
        }
        else {
            for (auto i = 0; i < (num_words() - 1) - num_words_to_shift; i++) {
                m_data[i]  = m_data[(i + num_words_to_shift) + 0] >> num_bits_to_shift;
                m_data[i] |= (m_data[(i + num_words_to_shift) + 1] << (num_underlying_bits() - num_bits_to_shift));
            }
            m_data[(num_words() - 1) - num_words_to_shift] = m_data[(num_words() - 1)] >> num_bits_to_shift;
        }

        // zero-fill from the left
        for (auto i = num_words() - num_words_to_shift; i < num_words(); i++) {
            m_data[i] = 0;
        }

        return *this;
    }

    template<class CharT = char, class Traits = std::char_traits<CharT>, class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, Traits, Allocator> to_string( CharT zero = CharT('0'), CharT one = CharT('1') ) const {
        std::basic_string<CharT, Traits, Allocator> str;
        str.reserve(size());
        for (auto i = size() - 1; i > 0; i--) {
            str.append(1, this->operator[](i) ? one : zero);
        }
        str.append(1, this->operator[](0) ? one : zero);
        return str;
    }

    constexpr unsigned long to_ulong() const {
        constexpr auto kNumBitsInUnsignedLong = 8 * sizeof(unsigned long);
        if (N < kNumBitsInUnsignedLong) {
            unsigned long value = 0;
            for (auto i = 0; i < size(); i++) {
                value |= this->operator[](i) << i;
            }
            return value;
        }

        unsigned long value = 0;
        for (auto i = 0; i < kNumBitsInUnsignedLong; i++) {
            value |= ((unsigned long)this->operator[](i)) << i;
        }
        for (auto i = kNumBitsInUnsignedLong; i < size(); i++) {
            if (this->operator[](i)) {
                throw std::overflow_error("bitset to_ulong overflow error");
            }
        }
        return value;
    }

    constexpr unsigned long long to_ullong() const {
        constexpr auto kNumBitsInUnsignedLongLong = 8 * sizeof(unsigned long long);
        if (N < kNumBitsInUnsignedLongLong) {
            unsigned long long value = 0;
            for (auto i = 0; i < size(); i++) {
                value |= this->operator[](i) << i;
            }
            return value;
        }

        unsigned long long value = 0;
        for (auto i = 0; i < kNumBitsInUnsignedLongLong; i++) {
            value |= ((unsigned long long)this->operator[](i)) << i;
        }
        for (auto i = kNumBitsInUnsignedLongLong; i < size(); i++) {
            if (this->operator[](i)) {
                throw std::overflow_error("bitset to_ulong overflow error");
            }
        }
        return value;
    }

    constexpr bool operator==( const small_bitset& rhs) const noexcept {
        for (auto i = 0; i < num_words(); i++) {
            if (m_data[i] != rhs.m_data[i]) {
                return false;
            }
        }
        if constexpr (N % num_underlying_bits() == 0) {
            return true;
        } else {
            constexpr Underlying ones_mask = (1u << (N % num_underlying_bits())) - 1;
            return (m_data[num_words() - 1] & ones_mask) == (rhs.m_data[num_words() -1 ] & ones_mask);
        }
    }

    constexpr bool operator!=(const small_bitset& rhs) const noexcept {return !(*this == rhs);}

    friend constexpr small_bitset operator&( const small_bitset& lhs,
                          const small_bitset& rhs ) noexcept {
        small_bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] & rhs.m_data[i];
        }
        return value;
    }

    friend constexpr small_bitset operator|( const small_bitset& lhs,
                          const small_bitset& rhs ) noexcept {
        small_bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] | rhs.m_data[i];
        }
        return value;
    }

    friend constexpr small_bitset operator^( const small_bitset& lhs,
                          const small_bitset& rhs ) noexcept {
        small_bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] ^ rhs.m_data[i];
        }
        return value;
    }


    template< class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<( std::basic_ostream<CharT, Traits>& os, const small_bitset& bits) {
        return os << bits.to_string(std::use_facet<std::ctype<CharT>>(os.getloc()).widen('0'),
                                    std::use_facet<std::ctype<CharT>>(os.getloc()).widen('1'));
    }

    template< class CharT, class Traits>
    friend std::basic_istream<CharT, Traits>& operator>>( std::basic_istream<CharT, Traits>& is, small_bitset& bits ) {
        int i{N - 1};
        while (i >= 0) {
            CharT input;
            is >> input;

            if (!is.good()) {
                break;
            }
            if (input == is.widen('0')) {
                bits[i] = 0;
            } else if (input == is.widen('1')) {
                bits[i] = 1;
            } else {
                is.setstate(std::ios_base::failbit);
                break;
            }
            --i;
        }
        if (i >= 0) { // if incomplete read
            bits >>= (i + 1);
        }
        return is;
    }

    friend struct std::hash<small_bitset>;
};

} // namespace nonstd

namespace std {

template<size_t N, typename Underlying>
struct hash<nonstd::small_bitset<N, Underlying>>
{
    constexpr size_t operator()(const nonstd::small_bitset<N, Underlying>& s) const noexcept
    {
        if constexpr (N < (8 * sizeof(unsigned long long))) {
            return hash<unsigned long long>()(s.to_ullong());
        }

        // boost::hash_combine algorithm
        size_t value = hash<Underlying>()(s.m_data[0]);
        for (auto i = 1; i < s.num_words(); i++) {
            value ^= hash<Underlying>()(s.m_data[i]) + 0x9e3779b9 + (value << 6) + (value >> 2);
        }
        return value;
    }
};
}
