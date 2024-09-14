
#include <bitset>
#include <cstddef>
#include <cstring> // memset

namespace nonstd {

template <std::size_t N, typename Underlying = std::byte> class small_bitset {

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
    constexpr small_bitset() { std::memset(m_data, 0, sizeof(m_data)); }

    constexpr bool operator[](std::size_t i) const {
        return (m_data[underlying_index(i)] &
                underlying_type_t(1 << (i % num_underlying_bits()))) !=
               underlying_type_t(0);
    }

    small_bitset &set(std::size_t pos, bool value = true) {
        if (value) {
            m_data[underlying_index(pos)] |=
                underlying_type_t(1 << (pos % num_underlying_bits()));
        } else {
            m_data[underlying_index(pos)] &=
                ~underlying_type_t(1 << (pos % num_underlying_bits()));
        }
        return *this;
    }

    bool test(std::size_t pos) const { return this->operator[](pos); }

    constexpr std::size_t count() const {
        std::size_t cnt = 0;
        for (std::size_t i = 0; i < size(); ++i) {
            if (test(i))
                ++cnt;
        }
        return cnt;
    }

    constexpr std::size_t size() const noexcept { return N; }
};
} // namespace nonstd
