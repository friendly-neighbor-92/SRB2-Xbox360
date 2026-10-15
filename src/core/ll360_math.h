#pragma once
// Small, header-only helpers that avoid requiring a newer C++ standard.
// Use these for game logic; keep D3D/XDK-specific types in platform/render code.

#include <math.h>

namespace ll360
{
    inline float Clamp(float v, float lo, float hi)
    {
        return v < lo ? lo : (v > hi ? hi : v);
    }

    inline float Lerp(float a, float b, float t)
    {
        return a + (b - a) * t;
    }

    inline float DegreesToRadians(float degrees)
    {
        return degrees * 0.01745329251994329577f;
    }

    inline float RadiansToDegrees(float radians)
    {
        return radians * 57.295779513082320876f;
    }

    inline float MoveTowards(float current, float target, float maxDelta)
    {
        if (maxDelta < 0.0f) maxDelta = -maxDelta;
        if (current < target)
        return (target - current <= maxDelta) ? target : current + maxDelta;
        return (current - target <= maxDelta) ? target : current - maxDelta;
    }

    inline float WrapAngleRadians(float angle)
    {
        const float TWO_PI = 6.2831853071795864769f;
        while (angle > 3.14159265358979323846f) angle -= TWO_PI;
        while (angle < -3.14159265358979323846f) angle += TWO_PI;
        return angle;
    }

    struct Vec2
    {
        float x, y;
        Vec2() : x(0.0f), y(0.0f) {}
        Vec2(float xValue, float yValue) : x(xValue), y(yValue) {}
        Vec2 operator+(const Vec2& b) const { return Vec2(x + b.x, y + b.y); }
        Vec2 operator-(const Vec2& b) const { return Vec2(x - b.x, y - b.y); }
        Vec2 operator*(float s) const { return Vec2(x * s, y * s); }
    };

    inline float Dot(const Vec2& a, const Vec2& b) { return a.x * b.x + a.y * b.y; }
    inline float LengthSquared(const Vec2& v) { return Dot(v, v); }
    inline float Length(const Vec2& v) { return sqrtf(LengthSquared(v)); }
    inline Vec2 Normalize(const Vec2& v)
    {
        const float len = Length(v);
        return len > 0.000001f ? Vec2(v.x / len, v.y / len) : Vec2(0.0f, 0.0f);
    }

    struct Vec3
    {
        float x, y, z;
        Vec3() : x(0.0f), y(0.0f), z(0.0f) {}
        Vec3(float xValue, float yValue, float zValue) : x(xValue), y(yValue), z(zValue) {}
        Vec3 operator+(const Vec3& b) const { return Vec3(x + b.x, y + b.y, z + b.z); }
        Vec3 operator-(const Vec3& b) const { return Vec3(x - b.x, y - b.y, z - b.z); }
        Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    };

    inline float Dot(const Vec3& a, const Vec3& b) { return a.x * b.x + a.y * b.y + a.z * b.z; }
    inline Vec3 Cross(const Vec3& a, const Vec3& b)
    {
        return Vec3(a.y * b.z - a.z * b.y,
        a.z * b.x - a.x * b.z,
        a.x * b.y - a.y * b.x);
    }
    inline float LengthSquared(const Vec3& v) { return Dot(v, v); }
    inline float Length(const Vec3& v) { return sqrtf(LengthSquared(v)); }
    inline Vec3 Normalize(const Vec3& v)
    {
        const float len = Length(v);
        return len > 0.000001f ? Vec3(v.x / len, v.y / len, v.z / len) : Vec3(0.0f, 0.0f, 0.0f);
    }
}
