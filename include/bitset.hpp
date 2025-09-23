
#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iosfwd>
#include <iterator>
#include <stdexcept>
#include <type_traits>

namespace nonstd {

template <std::size_t N, typename Underlying = std::uint8_t> class bitset {
    static_assert(std::is_unsigned_v<Underlying>,
                  "bitset requires an unsigned underlying type");

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

    std::array<underlying_type_t, num_words()> m_data{underlying_type_t(0)};

    static constexpr underlying_type_t m_last_word_mask =
        (N % num_underlying_bits() == 0)
            ? ~underlying_type_t{0}
            : ~underlying_type_t{0} >>
                  (num_underlying_bits() - (N % num_underlying_bits()));

    static constexpr underlying_type_t mask(std::size_t pos) noexcept {
        return underlying_type_t{1} << (pos % num_underlying_bits());
    }

  public:
    constexpr bitset() noexcept = default;

    constexpr bitset(unsigned long long value) noexcept {
        std::size_t num_bits_copied{0};
        constexpr std::size_t num_bits_in_ull{8 * sizeof value};
        while (num_bits_copied < num_bits_in_ull && num_bits_copied < N) {
            underlying_type_t copy =
                (value >> num_bits_copied) & ~underlying_type_t(0);
            m_data[underlying_index(num_bits_copied)] = copy;
            num_bits_copied += num_underlying_bits();
        }
        if (num_bits_in_ull > N) {
            m_data[num_words() - 1] &= m_last_word_mask;
        }
    }

    template <class CharT, class Traits, class Alloc>
    explicit bitset(
        const std::basic_string<CharT, Traits, Alloc> &str,
        typename std::basic_string<CharT, Traits, Alloc>::size_type pos = 0,
        typename std::basic_string<CharT, Traits, Alloc>::size_type n =
            std::basic_string<CharT, Traits, Alloc>::npos,
        CharT zero = CharT('0'), CharT one = CharT('1')) {
        if (pos > str.size()) {
            throw std::out_of_range("pos > str.size()");
        }

        auto i = 0;
        auto reverse_start = std::reverse_iterator(
            n == str.npos ? str.end() : str.begin() + pos + n);
        auto reverse_end = std::reverse_iterator(str.begin() + pos);
        auto iter = reverse_start;
        while (iter != reverse_end) {
            if (Traits::eq(*iter, zero)) {
                set(i, 0);
            } else if (Traits::eq(*iter, one)) {
                set(i, 1);
            } else {
                throw std::invalid_argument(
                    std::string("Unexpected character ") + *iter +
                    " is neither zero (" + zero + ") or one (" + one + ")");
            }
            ++i;
            ++iter;
        }
    }

    template <class CharT>
    constexpr explicit bitset(const CharT *str, std::size_t n = std::size_t(-1),
                              CharT zero = CharT('0'), CharT one = CharT('1')) {
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
                throw std::invalid_argument(
                    std::string("Unexpected character ") + *iter +
                    " is neither zero (" + zero + ") or one (" + one + ")");
            }
            ++i;
            ++iter;
        }
    }

    class reference {
      public:
        constexpr reference(const reference &) = default;

        constexpr reference &operator=(bool value) noexcept {
            if (value) {
                m_parent.m_data[bitset::underlying_index(m_pos)] |= mask(m_pos);
            } else {
                m_parent.m_data[bitset::underlying_index(m_pos)] &=
                    ~mask(m_pos);
            }
            return *this;
        }

        constexpr reference &operator=(const reference &value) noexcept {
            this->operator=(bool(value));
            return *this;
        }

        constexpr operator bool() const noexcept {
            const bitset &parent = m_parent;
            return parent[m_pos];
        }
        constexpr bool operator~() const noexcept {
            const bitset &parent = m_parent;
            return !parent[m_pos];
        }

        constexpr reference &flip() noexcept {
            this->operator=(bool(!*this));
            return *this;
        }

      private:
        friend bitset;
        constexpr reference(bitset &parent, std::size_t pos) noexcept
            : m_parent(parent), m_pos(pos) {}

        bitset &m_parent;
        std::size_t m_pos;
    };

    constexpr bool operator[](std::size_t i) const {
        return (m_data[underlying_index(i)] & mask(i)) != underlying_type_t(0);
    }

    constexpr reference operator[](std::size_t i) {
        return reference(*this, i);
    }

    constexpr bitset &set() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = ~underlying_type_t{0};
        }
        if constexpr (N % num_underlying_bits() != 0) {
            m_data[num_words() - 1] = m_last_word_mask;
        }
        return *this;
    }

    constexpr bitset &set(std::size_t pos, bool value = true) {
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

    constexpr bitset &flip() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = ~m_data[i];
        }
        if constexpr (N % num_underlying_bits() != 0) {
            m_data[num_words() - 1] &= m_last_word_mask;
        }
        return *this;
    }

    constexpr bitset &flip(std::size_t pos) {
        if (pos >= size()) {
            throw std::out_of_range("bitset::flip: __position (which is " +
                                    std::to_string(pos) + " >= _Nb (which is " +
                                    std::to_string(N) + ")");
        }
        m_data[underlying_index(pos)] ^= mask(pos);
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
            if (this->operator[](i))
                ++cnt;
        }
        return cnt;
    }

    constexpr std::size_t size() const noexcept { return N; }

    constexpr bool all() const noexcept {
        constexpr underlying_type_t ones = ~underlying_type_t{0};
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (m_data[i] != ones) {
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
        return m_data[num_words() - 1];
    }

    constexpr bool none() const noexcept {
        for (auto i = 0; i < num_words(); ++i) {
            if (m_data[i] != 0) {
                return false;
            }
        }
        return true;
    }

    constexpr bitset &reset() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = 0;
        }
        return *this;
    }

    constexpr bitset &reset(std::size_t pos) { return set(pos, false); }

    constexpr bitset &operator&=(const bitset &other) noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] &= other.m_data[i];
        }
        return *this;
    }

    constexpr bitset &operator|=(const bitset &other) noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] |= other.m_data[i];
        }
        return *this;
    }

    constexpr bitset &operator^=(const bitset &other) noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] ^= other.m_data[i];
        }
        return *this;
    }

    constexpr bitset operator~() const noexcept {
        bitset other;
        for (auto i = 0; i < num_words(); i++) {
            other.m_data[i] = ~m_data[i];
        }
        if (N % num_underlying_bits() != 0) {
            other.m_data[num_words() - 1] &= m_last_word_mask;
        }
        return other;
    }

    constexpr bitset operator<<(std::size_t shift) const noexcept {
        return bitset(*this) <<= shift;
    }

    constexpr bitset &operator<<=(std::size_t shift) noexcept {
        if (shift == 0) {
            return *this;
        }
        if (shift >= N) {
            reset();
            return *this;
        }

        const std::size_t num_words_to_shift = shift / num_underlying_bits();

        // If the shift is exactly the size of a word
        if (const auto num_bits_to_shift = shift % num_underlying_bits();
            num_bits_to_shift == 0) {
            for (auto i = num_words() - 1; i > num_words_to_shift; i--) {
                m_data[i] = m_data[(i - num_words_to_shift) - 0];
            }
            m_data[num_words_to_shift] = m_data[0];
        } else {
            for (std::size_t i = num_words() - 1; i > num_words_to_shift; i--) {
                m_data[i] = m_data[(i - num_words_to_shift) - 0]
                            << num_bits_to_shift;
                m_data[i] |= m_data[(i - num_words_to_shift) - 1] >>
                             (num_underlying_bits() - num_bits_to_shift);
            }
            m_data[num_words_to_shift] = m_data[0] << num_bits_to_shift;
        }

        // zero-fill from the right
        for (auto i = 0; i < num_words_to_shift; i++) {
            m_data[i] = 0;
        }

        return *this;
    }

    constexpr bitset operator>>(std::size_t shift) const noexcept {
        return bitset(*this) >>= shift;
    }

    constexpr bitset &operator>>=(std::size_t shift) noexcept {
        if (shift == 0) {
            return *this;
        }
        if (shift >= N) {
            reset();
            return *this;
        }
        const auto num_words_to_shift = shift / num_underlying_bits();

        // If the shift is exactly the size of a word
        if (const auto num_bits_to_shift = shift % num_underlying_bits();
            num_bits_to_shift == 0) {
            for (auto i = 0; i < (num_words() - 1) - num_words_to_shift; i++) {
                m_data[i] = m_data[(i + num_words_to_shift) + 0];
            }
            m_data[(num_words() - 1) - num_words_to_shift] =
                m_data[num_words() - 1];
        } else {
            for (auto i = 0; i < (num_words() - 1) - num_words_to_shift; i++) {
                m_data[i] =
                    m_data[(i + num_words_to_shift) + 0] >> num_bits_to_shift;
                m_data[i] |= (m_data[(i + num_words_to_shift) + 1]
                              << (num_underlying_bits() - num_bits_to_shift));
            }
            m_data[(num_words() - 1) - num_words_to_shift] =
                m_data[num_words() - 1] >> num_bits_to_shift;
        }

        // zero-fill from the left
        for (auto i = num_words() - num_words_to_shift; i < num_words(); i++) {
            m_data[i] = 0;
        }

        return *this;
    }

    template <class CharT = char, class Traits = std::char_traits<CharT>,
              class Allocator = std::allocator<CharT>>
    std::basic_string<CharT, Traits, Allocator>
    to_string(CharT zero = CharT('0'), CharT one = CharT('1')) const {
        std::basic_string<CharT, Traits, Allocator> str;
        str.reserve(size());
        for (auto i = size() - 1; i > 0; i--) {
            str.append(1, this->operator[](i) ? one : zero);
        }
        str.append(1, this->operator[](0) ? one : zero);
        return str;
    }

    constexpr unsigned long to_ulong() const {
        static_assert(sizeof(unsigned long) % sizeof(underlying_type_t) == 0);
        constexpr auto kNumBitsInUnsignedLong = 8 * sizeof(unsigned long);
        constexpr auto kNumWordsInUnsignedLong =
            sizeof(unsigned long) / sizeof(underlying_type_t);

        if (N < kNumBitsInUnsignedLong) {
            unsigned long value = 0;

            for (auto i = num_words() - 1; i > 0; i--) {
                value |= (unsigned long)m_data[i]
                         << (i * num_underlying_bits());
            }
            value |= m_data[0];

            return value;
        }

        unsigned long value = 0;
        for (auto i = kNumWordsInUnsignedLong - 1; i > 0; i--) {
            value |= (unsigned long)m_data[i] << (i * num_underlying_bits());
        }
        value |= m_data[0];

        // Throw overflow_error if the bitset contains more data that cant be
        // represented
        for (auto i = kNumWordsInUnsignedLong; i < num_words(); i++) {
            if (m_data[i] != 0) {
                throw std::overflow_error("bitset to_ulong overflow error");
            }
        }
        return value;
    }

    constexpr unsigned long long to_ullong() const {
        constexpr auto kNumBitsInUnsignedLongLong =
            8 * sizeof(unsigned long long);
        constexpr auto kNumWordsInUnsignedLongLong =
            sizeof(unsigned long long) / sizeof(underlying_type_t);

        if constexpr (N < kNumBitsInUnsignedLongLong) {
            unsigned long value = 0;
            for (auto i = num_words() - 1; i > 0; i--) {
                value |= (unsigned long long)m_data[i]
                         << (i * num_underlying_bits());
            }
            value |= m_data[0];

            return value;
        }

        unsigned long long value = 0;
        for (auto i = kNumWordsInUnsignedLongLong - 1; i > 0; i--) {
            value |= (unsigned long long)m_data[i]
                     << (i * num_underlying_bits());
        }
        value |= m_data[0];

        // Throw overflow_error if the bitset contains more data that cant be
        // represented
        for (auto i = kNumWordsInUnsignedLongLong; i < num_words(); i++) {
            if (m_data[i] != 0) {
                throw std::overflow_error("bitset to_ulong overflow error");
            }
        }
        return value;
    }

    constexpr bool operator==(const bitset &rhs) const noexcept {
        for (auto i = 0; i < num_words(); i++) {
            if (m_data[i] != rhs.m_data[i]) {
                return false;
            }
        }
        if constexpr (N % num_underlying_bits() == 0) {
            return true;
        } else {
            return m_data[num_words() - 1] == rhs.m_data[num_words() - 1];
        }
    }

    constexpr bool operator!=(const bitset &rhs) const noexcept {
        return !(*this == rhs);
    }

    friend constexpr bitset operator&(const bitset &lhs,
                                      const bitset &rhs) noexcept {
        bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] & rhs.m_data[i];
        }
        return value;
    }

    friend constexpr bitset operator|(const bitset &lhs,
                                      const bitset &rhs) noexcept {
        bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] | rhs.m_data[i];
        }
        return value;
    }

    friend constexpr bitset operator^(const bitset &lhs,
                                      const bitset &rhs) noexcept {
        bitset value;
        for (auto i = 0; i < num_words(); i++) {
            value.m_data[i] = lhs.m_data[i] ^ rhs.m_data[i];
        }
        return value;
    }

    template <class CharT, class Traits>
    friend std::basic_ostream<CharT, Traits> &
    operator<<(std::basic_ostream<CharT, Traits> &os, const bitset &bits) {
        return os << bits.to_string(
                   std::use_facet<std::ctype<CharT>>(os.getloc()).widen('0'),
                   std::use_facet<std::ctype<CharT>>(os.getloc()).widen('1'));
    }

    template <class CharT, class Traits>
    friend std::basic_istream<CharT, Traits> &
    operator>>(std::basic_istream<CharT, Traits> &is, bitset &bits) {
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

    friend struct std::hash<bitset>;
};

} // namespace nonstd

namespace std {

template <size_t N, typename Underlying>
struct hash<nonstd::bitset<N, Underlying>> {
    constexpr size_t
    operator()(const nonstd::bitset<N, Underlying> &s) const noexcept {
        if constexpr (N < (8 * sizeof(unsigned long long))) {
            return hash<unsigned long long>()(s.to_ullong());
        }

        // boost::hash_combine algorithm
        size_t value = hash<Underlying>()(s.m_data[0]);
        for (auto i = 1; i < s.num_words(); i++) {
            value ^= hash<Underlying>()(s.m_data[i]) + 0x9e3779b9 +
                     (value << 6) + (value >> 2);
        }
        return value;
    }
};
} // namespace std
