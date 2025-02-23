#pragma once

namespace rhield::random {
    namespace impl {
        inline std::mt19937_64 prng = {};

        inline void seed(std::optional<uint64_t> seed = std::nullopt) {
            if (!seed.has_value()) {
                std::random_device device;
                seed = (static_cast<uint64_t>(device()) << (sizeof(uint32_t) * CHAR_BIT)) | device();
            }

            prng.seed(*seed);
        }
    }  // namespace impl

    template<class Ty, class... Types>
    inline constexpr bool is_any_of_v = std::disjunction_v<std::is_same<Ty, Types>...>;

    template<typename Ty = uint32_t, typename TyVal = std::remove_reference_t<Ty>, typename Limits = std::numeric_limits<TyVal>>
    TyVal number(const TyVal min = Limits::min(), const TyVal max = Limits::max()) {
        using GenTy = std::conditional_t<is_any_of_v<Ty, int8_t, uint8_t, char>, int, TyVal>;
        std::uniform_int_distribution<GenTy> dist(min, max);
        return static_cast<TyVal>(dist(impl::prng));
    }

    inline void bytes(std::uint8_t* ptr, const std::size_t size) {
        std::generate_n(ptr, size, []() -> std::uint8_t { return number<std::uint8_t>(); });
    }

    inline std::vector<std::uint8_t> bytes(const std::size_t size) {
        std::vector<std::uint8_t> result = {};
        result.resize(size);

        bytes(result.data(), result.size());
        return result;
    }
}  // namespace rhield::random