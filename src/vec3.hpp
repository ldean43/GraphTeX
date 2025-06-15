#include <cmath>
struct vec3 {
    float x, y, z;

    vec3() : x(0), y(0), z(0) {}
    vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    bool operator==(const vec3& other) const {
        return (x == other.x && y == other.y && z == other.z);
    }

    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    vec3& operator+=(const vec3& other) {
        x += other.x;
        y += other.y;
        z += other.z;
        return *this;
    }

    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    vec3 operator*(float scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    vec3 operator/(float scalar) const {
        return vec3(x / scalar, y / scalar, z / scalar);
    }

    vec3 operator-() const {
        return vec3(-x, -y, -z);
    }

    vec3 normalize() const {
        float length = sqrt(x * x + y * y + z * z);
        if (length > 0) {
            return vec3(x / length, y / length, z / length);
        }
        return vec3(0, 0, 0);
    }

    float length() const {
        return sqrt(x * x + y * y + z * z);
    }

    vec3 cross(const vec3& other) const {
        return vec3(
            y * other.z - z * other.y,
            z * other.x - x * other.z,
            x * other.y - y * other.x
        );
    }
    
    float dot(const vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    vec3 lerp(const vec3& other, float t) const {
        return vec3(
            other.x * t + x * (1 - t),
            other.y * t + y * (1 - t),
            other.z * t + z * (1 - t)
        );
    }

    bool isfinite() {
        return std::isfinite(x) && std::isfinite(y) && std::isfinite(z);
    }

    struct Vec3Hash {
        std::size_t operator()(const vec3& v) const {
            std::size_t h1 = std::hash<int>{}(static_cast<int>(v.x));
            std::size_t h2 = std::hash<int>{}(static_cast<int>(v.y));
            std::size_t h3 = std::hash<int>{}(static_cast<int>(v.z));
            std::size_t seed = h1;
            seed ^= h2 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            seed ^= h3 + 0x9e3779b9 + (seed << 6) + (seed >> 2);
            return seed;
        }
    };
};