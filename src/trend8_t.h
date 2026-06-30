#ifndef _TREND8_T_A6E097742470_H_
#define _TREND8_T_A6E097742470_H_

#include <stdint.h>

#define TREND_UNSET_VALUE         0xFFFF
#define TREND_ENCODED_UNSET_VALUE ((trend8_t)0xFF)

typedef uint8_t trend8_t;

static inline trend8_t encode_trend8_t(uint32_t v)
{
    // Fast return for edge cases and small values
    if (v >= TREND_UNSET_VALUE) {
        return TREND_ENCODED_UNSET_VALUE;
    }
    if (v < 32) {
        return (trend8_t)v;
    }

    // Hardware-accelerated MSB search
    // __builtin_clz returns the number of leading zeros in a 32-bit integer
    // (31 - clz) gives the MSB index
    // Subtracting 4 (mantissa size) yields the required bit shift
    uint32_t shift = 31 - __builtin_clz(v) - 4;

    // Single shift with proper mathematical rounding to nearest
    uint32_t temp = (v + (1 << (shift - 1))) >> shift;

    // Overflow correction
    // Required if rounding pushed the number beyond 31
    // Example: 63 rounds to 64, making temp 32
    if (temp > 31) {
        temp >>= 1;
        shift++;
    }

    // Pack into the custom E4M4 byte format
    return (trend8_t)(((shift + 1) << 4) | (temp & 0x0F));
}

static inline uint32_t decode_trend8_t(trend8_t c)
{
    // Optimization: all values up to 31 decode exactly to themselves
    // This covers small numbers and mantissas with exponent 0 and 1
    if (c < 32) {
        return c;
    }

    // Handle the special invalid value flag
    if (c == TREND_ENCODED_UNSET_VALUE) {
        return TREND_UNSET_VALUE;
    }

    // Restore the full value
    // (c >> 4) extracts the exponent
    // (c & 0x0F) extracts the mantissa, to which we add the implicit hidden bit (16)
    uint32_t val = (uint32_t)(16 + (c & 0x0F)) << ((c >> 4) - 1);

    // Safeguard against uint16_t overflow after restoring large approximated values
    if (val > 0xFFFE) {
        return 0xFFFE;
    }

    return val;
}

#endif // _TREND8_T_A6E097742470_H_
