#pragma once

#include <cmath>

namespace alice2 {

constexpr float PI = 3.14159265358979323846f;
constexpr float TWO_PI = PI * 2.0f;
constexpr float RAD_TO_DEG = 180.0f / PI;
constexpr float DEG_TO_RAD = PI / 180.0f;
constexpr int PRECISION = 6;

struct Vec3f {
    float x, y, z;
    
    Vec3f() : x(0.0f), y(0.0f), z(0.0f) {}
    Vec3f(float _x, float _y, float _z) : x(_x), y(_y), z(_z) {}
    
    bool operator==(const Vec3f& other) const {
        return x == other.x && y == other.y && z == other.z;
    }

    Vec3f operator+(const Vec3f &other) const
    {
        return Vec3f(x + other.x, y + other.y, z + other.z);
    }

    Vec3f operator-(const Vec3f &other) const
    {
        return Vec3f(x - other.x, y - other.y, z - other.z);
    }

    Vec3f operator*(float scalar) const
    {
        return Vec3f(x * scalar, y * scalar, z * scalar);
    }

    Vec3f operator/(float scalar) const
    {
        return Vec3f(x / scalar, y / scalar, z / scalar);
    }

    Vec3f& operator+=(const Vec3f& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }
    
    Vec3f& operator-=(const Vec3f& other) {
        x -= other.x;
        y -= other.y;
        z -= other.z;
        return *this;
    }

    Vec3f& operator*=(float scalar) {
        x *= scalar;
        y *= scalar;
        z *= scalar;
        return *this;
    }

    Vec3f& operator/=(float scalar) {
        x /= scalar;
        y /= scalar;
        z /= scalar;
        return *this;
    }

    float Length() const {
        return std::sqrt(x*x + y*y + z*z);
    }
    
    Vec3f Normalize() const {
        float len = Length();
        if (len > 0) {
            return Vec3f(x/len, y/len, z/len);
        }
        return *this;
    }
    
    Vec3f Cross(const Vec3f& other) const {
        return Vec3f(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    float Dot(const Vec3f& other) const {
        return x * other.x + y * other.y + z * other.z;
    }
    
    float SquareDistanceTo(const Vec3f& v1) const {
        Vec3f diff = *this - v1;
        return diff.Dot(diff);
    }

    float DistanceTo(const Vec3f& v1) const {
        return std::sqrt(SquareDistanceTo(v1));
    }

    float Angle(const Vec3f& v1) const {
        Vec3f a = *this;
        Vec3f b = v1;

        a.Normalize();
        b.Normalize();

        float dotProduct = a.Dot(b);
        float factor = std::powf(10.0f, PRECISION);
        dotProduct = std::roundf(dotProduct * factor) / factor;

        if (dotProduct >= 1.0f) return 0.0f;
        if (dotProduct <= -1.0f) return 180.0f;
        return std::acosf(dotProduct) * RAD_TO_DEG;
    }

    float Angle360(const Vec3f& v1, const Vec3f& normal) const {
        Vec3f a = *this;
        Vec3f b = v1;

        a.Normalize();
        b.Normalize();

        float dot = a.Dot(b);
        if (dot >= 1.0f) return 0.0f;
        if (dot <= -1.0f) return 180.0f;

        Vec3f cross = a.Cross(b);
        float det = normal.Dot(cross);

        float angle = std::atan2(det, dot);
        if (angle < 0) angle += TWO_PI;

        return angle * RAD_TO_DEG;
    }

    float DihedralAngle(const Vec3f& v1, const Vec3f& v2) const {
        Vec3f e = *this;
        Vec3f n1 = v1;
        Vec3f n2 = v2;

        n1.Normalize();
        n2.Normalize();

        float dot = n1.Dot(n2);
        Vec3f cross = n1.Cross(n2);
        float dtheta = std::atan2(e.Dot(cross), dot);

        return dtheta * RAD_TO_DEG;
    }

    float Cotan(const Vec3f& v) const {
        Vec3f u = *this;
        float dot = u.Dot(v);
        float denom = u.Cross(v).Length();

        if (denom == 0.0f) return 0.0f;
        return dot / denom;
    }
};

struct Vec4f {
    float r, g, b, a;
    
    Vec4f() : r(0), g(0), b(0), a(0) {}
    Vec4f(float _r, float _g, float _b, float _a) : r(_r), g(_g), b(_b), a(_a) {}
    
    bool operator==(const Vec4f& other) const {
        return r == other.r && g == other.g && b == other.b && a == other.a;
    }
    
    Vec4f operator+(const Vec4f& other) const {
        return Vec4f(r + other.r, g + other.g, b + other.b, a + other.a);
    }
    
    Vec4f& operator+=(const Vec4f& other) {
        r += other.r;
        g += other.g;
        b += other.b;
        a += other.a;
        return *this;
    }
    
    Vec4f& operator-=(const Vec4f& other) {
        r -= other.r;
        g -= other.g;
        b -= other.b;
        a -= other.a;
        return *this;
    }

    Vec4f& operator*=(float scalar) {
        r *= scalar;
        g *= scalar;
        b *= scalar;
        a *= scalar;
        return *this;
    }

    Vec4f& operator/=(float scalar) {
        r /= scalar;
        g /= scalar;
        b /= scalar;
        a /= scalar;
        return *this;
    }
    
    Vec4f operator-(const Vec4f& other) const {
        return Vec4f(r - other.r, g - other.g, b - other.b, a - other.a);
    }
    
    Vec4f operator*(float scalar) const {
        return Vec4f(r * scalar, g * scalar, b * scalar, a * scalar);
    }
    
    float Length() const {
        return std::sqrt(r*r + g*g + b*b + a*a);
    }
    
    Vec4f Normalize() const {
        float len = Length();
        if (len > 0) {
            return Vec4f(r/len, g/len, b/len, a/len);
        }
        return *this;
    }
    
    Vec4f Cross(const Vec4f& other) const {
        return Vec4f(
            g * other.b - b * other.g,
            b * other.r - r * other.b,
            r * other.g - g * other.r,
            a * other.a - a * other.a
        );
    }
    
    float Dot(const Vec4f& other) const {
        return r * other.r + g * other.g + b * other.b + a * other.a;
    }
    
    Vec4f operator/(float scalar) const {
        return Vec4f(r / scalar, g / scalar, b / scalar, a / scalar);
    }
};

struct Color : Vec4f {
    Color(float red = 1.0f, float green = 1.0f, float blue = 1.0f, float alpha = 1.0f)
        : Vec4f(red, green, blue, alpha) {}
    
    static Color White() { return Color(1.0f, 1.0f, 1.0f, 1.0f  ); }
    static Color Black() { return Color(0.0f, 0.0f, 0.0f, 1.0f); }
    static Color Red()   { return Color(1.0f, 0.0f, 0.0f, 1.0f); }
    static Color Green() { return Color(0.0f, 1.0f, 0.0f, 1.0f); }
    static Color Blue()  { return Color(0.0f, 0.0f, 1.0f, 1.0f); }
    static Color Yellow() { return Color(1.0f, 1.0f, 0.0f, 1.0f); }
    static Color Gray()  { return Color(0.5f, 0.5f, 0.5f, 1.0f); }
};

} // namespace alice2 