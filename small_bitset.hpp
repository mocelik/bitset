
#include <bitset>
#include <cstddef>

namespace nonstd {

template <std::size_t N, typename Underlying = std::byte> class small_bitset {

    using underlying_type_t = Underlying;
    static constexpr std::size_t underlying_bits() {
        return 8 * sizeof(underlying_type_t);
    }

    underlying_type_t data[(N + underlying_bits() - 1) / underlying_bits()];

    constexpr std::size_t indexOf(std::size_t i) const {
        return (i + underlying_bits() - 1) / underlying_bits();
    }

  public:
    constexpr bool operator[](std::size_t i) const {
        return (data[indexOf(i)] &
                underlying_type_t(1 << (i % underlying_bits()))) !=
               underlying_type_t(0);
    }

    small_bitset &set(std::size_t pos, bool value = true) {
        if (value) {
            data[indexOf(pos)] |=
                underlying_type_t(1 << (pos % underlying_bits()));
        } else {
            data[indexOf(pos)] &=
                ~underlying_type_t(1 << (pos % underlying_bits()));
        }
        return *this;
    }

    bool test(std::size_t pos) const { return (*this)[pos]; }

    constexpr std::size_t size() const noexcept { return N; }
};
} // namespace nonstd
