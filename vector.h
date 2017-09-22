#ifndef VECTOR_H
#define VECTOR_H

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

        Vec& operator=(const Vec &v) {
            x += v.x;
            y += v.y;
            z += v.z;
            return *this;
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

class Tri {
  public:
    Vec a;
    Vec b;
    Vec c;

    Tri() {
    }

    Tri(const Vec &a, const Vec &b, const Vec &c) : a(a), b(b), c(c) {
    }

    Tri(const Tri &t) : a(t.a), b(t.b), c(t.c) {
    }

    Vec normal() const {
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

class Matrix {
  public:
    float elements[16];

    Matrix() : elements() {}
    
    Matrix(const Matrix &m) {
      for (unsigned int i = 0; i < sizeof(this->elements) / sizeof(this->elements[0]); i++) {
        this->elements[i] = m.elements[i];
      }
    }

    Matrix(float *elements) {
      for (unsigned int i = 0; i < sizeof(this->elements) / sizeof(this->elements[0]); i++) {
        this->elements[i] = elements[i];
      }
    }

    Matrix &operator*=(const Matrix &m) {
      return this->multiply(*this, m);
    }

    Matrix operator*(const Matrix &m) const {
      return Matrix().multiply(*this, m);
    }

    Matrix &multiply(const Matrix &a, const Matrix &b) {
      const float * const ae = a.elements;
      const float * const be = b.elements;
      float *te = this->elements;

      const float a11 = ae[ 0 ], a12 = ae[ 4 ], a13 = ae[ 8 ], a14 = ae[ 12 ];
      const float a21 = ae[ 1 ], a22 = ae[ 5 ], a23 = ae[ 9 ], a24 = ae[ 13 ];
      const float a31 = ae[ 2 ], a32 = ae[ 6 ], a33 = ae[ 10 ], a34 = ae[ 14 ];
      const float a41 = ae[ 3 ], a42 = ae[ 7 ], a43 = ae[ 11 ], a44 = ae[ 15 ];

      const float b11 = be[ 0 ], b12 = be[ 4 ], b13 = be[ 8 ], b14 = be[ 12 ];
      const float b21 = be[ 1 ], b22 = be[ 5 ], b23 = be[ 9 ], b24 = be[ 13 ];
      const float b31 = be[ 2 ], b32 = be[ 6 ], b33 = be[ 10 ], b34 = be[ 14 ];
      const float b41 = be[ 3 ], b42 = be[ 7 ], b43 = be[ 11 ], b44 = be[ 15 ];

      te[ 0 ] = a11 * b11 + a12 * b21 + a13 * b31 + a14 * b41;
      te[ 4 ] = a11 * b12 + a12 * b22 + a13 * b32 + a14 * b42;
      te[ 8 ] = a11 * b13 + a12 * b23 + a13 * b33 + a14 * b43;
      te[ 12 ] = a11 * b14 + a12 * b24 + a13 * b34 + a14 * b44;

      te[ 1 ] = a21 * b11 + a22 * b21 + a23 * b31 + a24 * b41;
      te[ 5 ] = a21 * b12 + a22 * b22 + a23 * b32 + a24 * b42;
      te[ 9 ] = a21 * b13 + a22 * b23 + a23 * b33 + a24 * b43;
      te[ 13 ] = a21 * b14 + a22 * b24 + a23 * b34 + a24 * b44;

      te[ 2 ] = a31 * b11 + a32 * b21 + a33 * b31 + a34 * b41;
      te[ 6 ] = a31 * b12 + a32 * b22 + a33 * b32 + a34 * b42;
      te[ 10 ] = a31 * b13 + a32 * b23 + a33 * b33 + a34 * b43;
      te[ 14 ] = a31 * b14 + a32 * b24 + a33 * b34 + a34 * b44;

      te[ 3 ] = a41 * b11 + a42 * b21 + a43 * b31 + a44 * b41;
      te[ 7 ] = a41 * b12 + a42 * b22 + a43 * b32 + a44 * b42;
      te[ 11 ] = a41 * b13 + a42 * b23 + a43 * b33 + a44 * b43;
      te[ 15 ] = a41 * b14 + a42 * b24 + a43 * b34 + a44 * b44;

      return *this;
    }

    static Matrix fromArray(float *elements) {
      return Matrix(elements);
    }
};

class Sphere {
  public:
    Vec center;
    float radius;

    Sphere(const Vec &center, float radius) : center(center), radius(radius) {}
    Sphere(float x, float y, float z, float radius) : center(x, y, z), radius(radius) {}
};

class Plane {
  public:
    Vec normal;
    float constant;

    Plane() {}

    Plane(const Vec &normal, float constant) : normal(normal), constant(constant) {}

    Plane &setComponents(float x, float y, float z, float w) {
		  normal = Vec(x, y, z);
      constant = w;
      return *this;
    }

    Plane &normalize() {
      float inverseNormalLength = 1.0 / normal.magnitude();
      normal *= inverseNormalLength;
      constant *= inverseNormalLength;

      return *this;
    }

    float distanceToPoint(const Vec &point) const {
      return normal * point + constant;
    }
};

class Frustum {
  public:
    Plane planes[6];

    Frustum() {}

    bool intersectsSphere(const Sphere &sphere) const {
      const Vec &center = sphere.center;
      const float negRadius = - sphere.radius;

      for ( unsigned int i = 0; i < 6; i ++ ) {

        const float distance = planes[ i ].distanceToPoint( center );

        if ( distance < negRadius ) {

          return false;

        }

      }

      return true;
    }

    Frustum &setFromMatrix(const Matrix &m) {
      const float * const me = m.elements;
      const float me0 = me[ 0 ], me1 = me[ 1 ], me2 = me[ 2 ], me3 = me[ 3 ];
      const float me4 = me[ 4 ], me5 = me[ 5 ], me6 = me[ 6 ], me7 = me[ 7 ];
      const float me8 = me[ 8 ], me9 = me[ 9 ], me10 = me[ 10 ], me11 = me[ 11 ];
      const float me12 = me[ 12 ], me13 = me[ 13 ], me14 = me[ 14 ], me15 = me[ 15 ];

      planes[ 0 ].setComponents( me3 - me0, me7 - me4, me11 - me8, me15 - me12 ).normalize();
      planes[ 1 ].setComponents( me3 + me0, me7 + me4, me11 + me8, me15 + me12 ).normalize();
      planes[ 2 ].setComponents( me3 + me1, me7 + me5, me11 + me9, me15 + me13 ).normalize();
      planes[ 3 ].setComponents( me3 - me1, me7 - me5, me11 - me9, me15 - me13 ).normalize();
      planes[ 4 ].setComponents( me3 - me2, me7 - me6, me11 - me10, me15 - me14 ).normalize();
      planes[ 5 ].setComponents( me3 + me2, me7 + me6, me11 + me10, me15 + me14 ).normalize();

      return *this;
    }

    static Frustum fromMatrix(const Matrix &m) {
      return Frustum().setFromMatrix(m);
    }
};

#endif
