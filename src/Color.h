#pragma once
#include "glm/glm.hpp"
#include <algorithm>
#include <cmath>
#include <cstdint>

namespace fc {

struct Color {
    float r, g, b, a;

    // --- Constructors ---
    constexpr Color() : r(0.0f), g(0.0f), b(0.0f), a(1.0f) {}
    constexpr Color(float r, float g, float b) : r(r), g(g), b(b), a(1.0f) {}
    constexpr Color(float r, float g, float b, float a) : r(r), g(g), b(b), a(a) {}
    Color(const glm::vec4& vec) : r(vec.r), g(vec.g), b(vec.b), a(vec.a) {}
    Color(const glm::vec3& vec) : r(vec.r), g(vec.g), b(vec.b), a(1.0f) {}

    // Construct from 0-255 byte values
    static Color fromBytes(uint8_t r, uint8_t g, uint8_t b, uint8_t a = 255)
    {
        return Color(r / 255.0f, g / 255.0f, b / 255.0f, a / 255.0f);
    }

    // Construct from a hex value, e.g. 0xRRGGBBAA or 0xRRGGBB
    static Color fromHex(uint32_t hex, bool hasAlpha = false)
    {
        if (hasAlpha) {
            return fromBytes((hex >> 24) & 0xFF, (hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF);
        }
        return fromBytes((hex >> 16) & 0xFF, (hex >> 8) & 0xFF, hex & 0xFF, 255);
    }

    // Construct from HSV (h in [0,360), s/v in [0,1])
    static Color fromHSV(float h, float s, float v, float a = 1.0f)
    {
        float c = v * s;
        float x = c * (1.0f - std::fabs(std::fmod(h / 60.0f, 2.0f) - 1.0f));
        float m = v - c;
        float r1, g1, b1;
        if (h < 60) {
            r1 = c;
            g1 = x;
            b1 = 0;
        }
        else if (h < 120) {
            r1 = x;
            g1 = c;
            b1 = 0;
        }
        else if (h < 180) {
            r1 = 0;
            g1 = c;
            b1 = x;
        }
        else if (h < 240) {
            r1 = 0;
            g1 = x;
            b1 = c;
        }
        else if (h < 300) {
            r1 = x;
            g1 = 0;
            b1 = c;
        }
        else {
            r1 = c;
            g1 = 0;
            b1 = x;
        }
        return Color(r1 + m, g1 + m, b1 + m, a);
    }

    // --- Conversions ---
    operator glm::vec4() const { return glm::vec4(r, g, b, a); }
    operator glm::vec3() const { return glm::vec3(r, g, b); }

    uint32_t toHex(bool withAlpha = true) const
    {
        auto toByte = [](float v) -> uint32_t {
            return static_cast<uint32_t>(std::clamp(v, 0.0f, 1.0f) * 255.0f + 0.5f);
        };
        if (withAlpha) {
            return (toByte(r) << 24) | (toByte(g) << 16) | (toByte(b) << 8) | toByte(a);
        }
        return (toByte(r) << 16) | (toByte(g) << 8) | toByte(b);
    }

    // --- Utility ---
    
    Color clamped() const
    {
        return Color(std::clamp(r, 0.0f, 1.0f), std::clamp(g, 0.0f, 1.0f),
                     std::clamp(b, 0.0f, 1.0f), std::clamp(a, 0.0f, 1.0f));
    }

    // Perceived luminance (Rec. 709 coefficients)
    float luminance() const { return 0.2126f * r + 0.7152f * g + 0.0722f * b; }

    Color withAlpha(float newAlpha) const { return Color(r, g, b, newAlpha); }

    // --- Operators ---
    Color operator+(const Color& o) const { return Color(r + o.r, g + o.g, b + o.b, a + o.a); }
    Color operator-(const Color& o) const { return Color(r - o.r, g - o.g, b - o.b, a - o.a); }
    Color operator*(const Color& o) const { return Color(r * o.r, g * o.g, b * o.b, a * o.a); }
    Color operator*(float s) const { return Color(r * s, g * s, b * s, a * s); }
    Color operator/(float s) const { return Color(r / s, g / s, b / s, a / s); }

    Color& operator+=(const Color& o)
    {
        r += o.r;
        g += o.g;
        b += o.b;
        a += o.a;
        return *this;
    }
    Color& operator-=(const Color& o)
    {
        r -= o.r;
        g -= o.g;
        b -= o.b;
        a -= o.a;
        return *this;
    }
    Color& operator*=(const Color& o)
    {
        r *= o.r;
        g *= o.g;
        b *= o.b;
        a *= o.a;
        return *this;
    }
    Color& operator*=(float s)
    {
        r *= s;
        g *= s;
        b *= s;
        a *= s;
        return *this;
    }

    bool operator==(const Color& o) const { return r == o.r && g == o.g && b == o.b && a == o.a; }
    bool operator!=(const Color& o) const { return !(*this == o); }

    // --- Common colors ---
    static const Color White;
    static const Color Black;
    static const Color Red;
    static const Color Green;
    static const Color Blue;
    static const Color Yellow;
    static const Color Cyan;
    static const Color Magenta;
    static const Color Gray;
    static const Color Transparent;
};

inline Color operator*(float s, const Color& c)
{
    return c * s;
}

inline const Color Color::White = Color(1.0f, 1.0f, 1.0f, 1.0f);
inline const Color Color::Black = Color(0.0f, 0.0f, 0.0f, 1.0f);
inline const Color Color::Red = Color(1.0f, 0.0f, 0.0f, 1.0f);
inline const Color Color::Green = Color(0.0f, 1.0f, 0.0f, 1.0f);
inline const Color Color::Blue = Color(0.0f, 0.0f, 1.0f, 1.0f);
inline const Color Color::Yellow = Color(1.0f, 1.0f, 0.0f, 1.0f);
inline const Color Color::Cyan = Color(0.0f, 1.0f, 1.0f, 1.0f);
inline const Color Color::Magenta = Color(1.0f, 0.0f, 1.0f, 1.0f);
inline const Color Color::Gray = Color(0.5f, 0.5f, 0.5f, 1.0f);
inline const Color Color::Transparent = Color(0.0f, 0.0f, 0.0f, 0.0f);

} // namespace fc