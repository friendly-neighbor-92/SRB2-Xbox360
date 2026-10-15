#pragma once
// Alignment-safe byte-order helpers for binary asset/save readers.
// Xbox 360's PowerPC CPU is big-endian; common PC-produced formats are often
// little-endian. Never reinterpret an arbitrary byte buffer as a typed pointer.

#include <stdint.h>
#include <string.h>

namespace ll360
{
    inline uint16_t ByteSwap16(uint16_t v)
    {
        return static_cast<uint16_t>((v >> 8) | (v << 8));
    }

    inline uint32_t ByteSwap32(uint32_t v)
    {
        return ((v & 0x000000FFu) << 24) |
        ((v & 0x0000FF00u) << 8)  |
        ((v & 0x00FF0000u) >> 8)  |
        ((v & 0xFF000000u) >> 24);
    }

    inline uint16_t ReadU16LE(const void* data)
    {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        return static_cast<uint16_t>(static_cast<uint16_t>(p[0]) |
        (static_cast<uint16_t>(p[1]) << 8));
    }

    inline uint16_t ReadU16BE(const void* data)
    {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        return static_cast<uint16_t>((static_cast<uint16_t>(p[0]) << 8) |
        static_cast<uint16_t>(p[1]));
    }

    inline uint32_t ReadU32LE(const void* data)
    {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        return static_cast<uint32_t>(p[0]) |
        (static_cast<uint32_t>(p[1]) << 8) |
        (static_cast<uint32_t>(p[2]) << 16) |
        (static_cast<uint32_t>(p[3]) << 24);
    }

    inline uint32_t ReadU32BE(const void* data)
    {
        const unsigned char* p = static_cast<const unsigned char*>(data);
        return (static_cast<uint32_t>(p[0]) << 24) |
        (static_cast<uint32_t>(p[1]) << 16) |
        (static_cast<uint32_t>(p[2]) << 8) |
        static_cast<uint32_t>(p[3]);
    }

    inline void WriteU16LE(void* data, uint16_t value)
    {
        unsigned char* p = static_cast<unsigned char*>(data);
        p[0] = static_cast<unsigned char>(value);
        p[1] = static_cast<unsigned char>(value >> 8);
    }

    inline void WriteU16BE(void* data, uint16_t value)
    {
        unsigned char* p = static_cast<unsigned char*>(data);
        p[0] = static_cast<unsigned char>(value >> 8);
        p[1] = static_cast<unsigned char>(value);
    }

    inline void WriteU32LE(void* data, uint32_t value)
    {
        unsigned char* p = static_cast<unsigned char*>(data);
        p[0] = static_cast<unsigned char>(value);
        p[1] = static_cast<unsigned char>(value >> 8);
        p[2] = static_cast<unsigned char>(value >> 16);
        p[3] = static_cast<unsigned char>(value >> 24);
    }

    inline void WriteU32BE(void* data, uint32_t value)
    {
        unsigned char* p = static_cast<unsigned char*>(data);
        p[0] = static_cast<unsigned char>(value >> 24);
        p[1] = static_cast<unsigned char>(value >> 16);
        p[2] = static_cast<unsigned char>(value >> 8);
        p[3] = static_cast<unsigned char>(value);
    }

    inline float ReadF32LE(const void* data)
    {
        const uint32_t bits = ReadU32LE(data);
        float value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    inline float ReadF32BE(const void* data)
    {
        const uint32_t bits = ReadU32BE(data);
        float value;
        memcpy(&value, &bits, sizeof(value));
        return value;
    }

    inline void WriteF32LE(void* data, float value)
    {
        uint32_t bits;
        memcpy(&bits, &value, sizeof(bits));
        WriteU32LE(data, bits);
    }

    inline void WriteF32BE(void* data, float value)
    {
        uint32_t bits;
        memcpy(&bits, &value, sizeof(bits));
        WriteU32BE(data, bits);
    }
}
