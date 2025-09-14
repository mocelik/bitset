
#include <cstddef>
#include <cstdint>
#include <cstring> // memset
#include <iterator>
#include <stdexcept> // std::out_of_range
#include <type_traits>
#include <ostream>

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
            if (*iter == zero) {
                set(i, 0);
            } else if (*iter == one) {
                set(i, 1);
            } else {
                throw std::invalid_argument(std::string("Unexpected character ") + *iter + " is neither zero (" + zero + ") or one (" + one + ")");
            }
            ++i;
            ++iter;
        }
    }

    constexpr bool operator[](std::size_t i) const {
        return (m_data[underlying_index(i)] &
                underlying_type_t(1 << (i % num_underlying_bits()))) !=
               underlying_type_t(0);
    }

    constexpr small_bitset &set(std::size_t pos, bool value = true) {
        if (value) {
            m_data[underlying_index(pos)] |=
                underlying_type_t(1 << (pos % num_underlying_bits()));
        } else {
            m_data[underlying_index(pos)] &=
                ~underlying_type_t(1 << (pos % num_underlying_bits()));
        }
        return *this;
    }

    constexpr small_bitset &flip() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = ~m_data[i];
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

    bool test(std::size_t pos) const { return this->operator[](pos); }

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
        constexpr underlying_type_t all_ones =
            ~static_cast<underlying_type_t>(0);
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (std::memcmp(&m_data[i], &all_ones, sizeof all_ones) != 0) {

                return false;
            }
        }
        return m_data[num_words() - 1] ==
               (all_ones >> (num_underlying_bits() * num_words() - N));
    }

    constexpr bool any() const noexcept {
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (m_data[i] != underlying_type_t{0}) {
                return true;
            }
        }
        return (m_data[num_words() - 1] &
                (~underlying_type_t{0} >> (num_underlying_bits() * num_words() -
                                           N))) > underlying_type_t{0};
    }

    constexpr bool none() const noexcept {
        constexpr underlying_type_t all_zeros{0};
        for (auto i = 0; i < num_words() - 1; ++i) {
            if (std::memcmp(&m_data[i], &all_zeros, sizeof all_zeros) != 0) {
                return false;
            }
        }
        return m_data[num_words() - 1] ==
               (all_zeros >> (num_underlying_bits() * num_words() - N));
    }

    constexpr small_bitset &reset() noexcept {
        for (auto i = 0; i < num_words(); i++) {
            m_data[i] = 0;
        }
        return *this;
    }

    constexpr small_bitset &reset(std::size_t pos) { return set(pos, false); }


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
        const auto num_words_to_shift = shift / num_underlying_bits();
        const auto num_bits_to_shift = shift % num_underlying_bits();

        // Handle the words between the most significant and the least significant
        for (auto i = num_words() - 1; i > num_words_to_shift; i--) {
            m_data[i]  = m_data[(i - num_words_to_shift) - 0] << num_bits_to_shift;
            m_data[i] |= m_data[(i - num_words_to_shift) - 1] >> (num_underlying_bits() - num_bits_to_shift);
        }
        m_data[num_words_to_shift] = m_data[0] << num_bits_to_shift;

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
        const auto num_words_to_shift = shift / num_underlying_bits();
        const auto num_bits_to_shift = shift % num_underlying_bits();

        for (auto i = 0; i < (num_words() - 1) - num_words_to_shift; i++) {

            m_data[i]  = m_data[(i + num_words_to_shift) + 0] >> num_bits_to_shift;
            m_data[i] |= m_data[(i + num_words_to_shift) + 1] << (num_underlying_bits() - num_bits_to_shift);
        }
        m_data[(num_words() - 1) - num_words_to_shift] = m_data[(num_words() - 1)] >> num_bits_to_shift;

        // zero-fill from the left
        for (auto i = num_words() - num_words_to_shift; i < num_words(); i++) {
            m_data[i] = 0;
        }

        return *this;
    }
};

template <std::size_t N, typename Underlying>
std::ostream& operator<<(std::ostream& os, small_bitset<N, Underlying> bits) {
    for (auto i = bits.size() - 1; i > 0; i--) {
        os << bits[i];
    }
    return os << bits[0];
}

} // namespace nonstd
