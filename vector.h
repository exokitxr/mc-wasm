#include <cmath>
#include <algorithm>

// 3D Vector Class
// Can also be used for 2D vectors
// by ignoring the z value
class Vec {
  public:
    
        union {
            float data[3];
            struct {
                float x;
                float y;
                float z;
            };
        };

        // Constructors

        // Vectors default to 0, 0, 0.
        Vec() {
            x = 0;
            y = 0;
            z = 0;
        }

        // Construct with values, 3D
        Vec(float ax, float ay, float az) {
            x = ax;
            y = ay;
            z = az;
        }

        /* // Construct with values, 2D
        Vec(float ax, float ay) {
            x = ax;
            y = ay;
            z = 0;
        } */

        // Copy constructor
        Vec(const Vec &o) {
            x = o.x;
            y = o.y;
            z = o.z;
        }

        // Addition
        
        Vec operator+(const Vec &o) const {
            return Vec(x + o.x, y + o.y, z + o.z);
        }

        Vec& operator+=(const Vec &o) {
            x += o.x;
            y += o.y;
            z += o.z;
            return *this;
        }

        // Subtraction

        Vec operator-() const {
            return Vec(-x, -y, -z);
        }

        Vec operator-(const Vec &o) const {
            return Vec(x - o.x, y - o.y, z - o.z);
        }

        Vec& operator-=(const Vec &o) {
            x -= o.x;
            y -= o.y;
            z -= o.z;
            return *this;
        }

        // Multiplication by scalars

        Vec operator*(const float s) const {
            return Vec(x * s, y * s, z * s);
        }

        Vec& operator*=(const float s) {
            x *= s;
            y *= s;
            z *= s;
            return *this;
        }

        // Division by scalars

        Vec operator/(const float s) const {
            return Vec(x / s, y / s, z / s);
        }

        Vec& operator/=(const float s) {
            x /= s;
            y /= s;
            z /= s;
            return *this;
        }
        
        // Dot product

        float operator*(const Vec &o) const {
            return (x * o.x) + (y * o.y) + (z * o.z);
        }

        // An in-place dot product does not exist because
        // the result is not a vector.

        // Cross product

        Vec operator^(const Vec &o) const {
            float nx = y * o.z - o.y * z;
            float ny = z * o.x - o.z * x;
            float nz = x * o.y - o.x * y;
            return Vec(nx, ny, nz);
        }

        Vec& operator^=(const Vec &o) {
            float nx = y * o.z - o.y * z;
            float ny = z * o.x - o.z * x;
            float nz = x * o.y - o.x * y;
            x = nx;
            y = ny;
            z = nz;
            return *this;
        }

        // Other functions
        
        // Length of vector
        float magnitude() const {
            return sqrt(magnitude_sqr());
        }

        // Length of vector squared
        float magnitude_sqr() const {
            return (x * x) + (y * y) + (z * z);
        }

        // Returns a normalized copy of the vector
        // Will break if it's length is 0
        Vec normalized() const {
            return Vec(*this) / magnitude();
        }

        // Modified the vector so it becomes normalized
        Vec& normalize() {
            (*this) /= magnitude();
            return *this;
        }

        Vec& min(const Vec &o) {
          x = std::min<float>(x, o.x);
          y = std::min<float>(y, o.y);
          z = std::min<float>(z, o.z);
          return *this;
        }

        Vec& max(const Vec &o) {
          x = std::max<float>(x, o.x);
          y = std::max<float>(y, o.y);
          z = std::max<float>(z, o.z);
          return *this;
        }

        static Vec normal(const Vec &a, const Vec &b, const Vec &c) {
          Vec result(c);
          result -= b;
          Vec v0(a);
          v0 -= b;
          result ^= v0;

          float resultLengthSq = result.magnitude_sqr();
          if (resultLengthSq > 0) {
            result *= 1.0 / sqrt(resultLengthSq);
            return result;
          }
          return Vec(0, 0, 0);
        }
};

class Quat {
  public:
    union {
      float data[4];
      struct {
        float x;
        float y;
        float z;
        float w;
      };
    };

    Quat() {
      x = 0;
      y = 0;
      z = 0;
      w = 0;
    }

    Quat(float ax, float ay, float az, float aw) {
      x = ax;
      y = ay;
      z = az;
      w = aw;
    }

    Quat(const Quat &q) {
      x = q.x;
      y = q.y;
      z = q.z;
      w = q.w;
    }
};
