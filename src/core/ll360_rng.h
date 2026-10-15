#pragma once
// Small deterministic PRNG for gameplay, procedural content, and repeatable tests.
// This is not a cryptographic random-number generator.
#include <stdint.h>

namespace ll360
{
    struct Rng32
    {
        uint32_t state;

        explicit Rng32(uint32_t seed = 0xA341316Cu) { Seed(seed); }

        void Seed(uint32_t seed)
        {
            // xorshift32 cannot leave the all-zero state.
            state = seed ? seed : 0xA341316Cu;
        }

        uint32_t NextU32()
        {
            uint32_t x = state;
            x ^= x << 13;
            x ^= x >> 17;
            x ^= x << 5;
            state = x;
            return x;
        }

        // Returns an integer in [0, upperExclusive). Returns 0 when upperExclusive is 0.
        uint32_t RangeU32(uint32_t upperExclusive)
        {
            if (upperExclusive == 0) return 0;
            const uint32_t threshold = static_cast<uint32_t>(0u - upperExclusive) % upperExclusive;
            uint32_t value;
            do { value = NextU32(); } while (value < threshold);
            return value % upperExclusive;
        }

        // Returns a float in [0, 1), using 24 random mantissa bits.
        float Float01()
        {
            return static_cast<float>(NextU32() >> 8) * (1.0f / 16777216.0f);
        }
    };
}
