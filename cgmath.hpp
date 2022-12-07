#pragma once

#include <utility>
#include <iostream>
#include <cmath>
#include <vector>
#include <array>

// you can open SSE if you want to use SIMD
#ifdef USE_SIMD
#include "nmmintrin.h"
#endif

namespace cgmath {

constexpr double PI = 3.14159265358979;

// some math function

inline double Rad2Deg(double value) {
    return value * 180.0 / PI;
}

inline double Deg2Rad(double value) {
    return value * PI / 180.0;
}

// vector structure decalre

template <typename T, unsigned int AttrNum>
class VecMemberGenerator;

template <typename T, unsigned int AttrNum>
struct Vector final: public VecMemberGenerator<T, AttrNum> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 2>::setVec(this);
        *this = o;
    }

    Vector() = default;

    Vector(const std::initializer_list<T>& datas) {
        VecMemberGenerator<T, AttrNum>::setVec(this);
        VecMemberGenerator<T, AttrNum>::initVec(datas);
    }

    T data[AttrNum];
};

// disable Vector<T, 1>
template <typename T>
struct Vector<T, 1> final: public VecMemberGenerator<T, 1> {};

template <typename T>
struct Vector<T, 2> final: public VecMemberGenerator<T, 2> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 2>::setVec(this);
        *this = o;
    }

    Vector() = default;

    Vector(const std::initializer_list<T>& datas) {
        VecMemberGenerator<T, 2>::setVec(this);
        VecMemberGenerator<T, 2>::initVec(datas);
    }

    T Cross(const Vector& o) {
        return ::cgmath::Cross(*this, o);
    }

    union {
        T data[2];
        struct { T x, y; };
    };
};

template <typename T>
struct Vector<T, 3> final: public VecMemberGenerator<T, 3> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 3>::setVec(this);
        *this = o;
    }

    Vector() = default;

    Vector(const std::initializer_list<T>& datas) {
        VecMemberGenerator<T, 3>::setVec(this);
        VecMemberGenerator<T, 3>::initVec(datas);
    }

    auto Cross(const Vector& o) {
        return ::cgmath::Cross(*this, o);
    }

    union {
        T data[3];
        struct { T x, y, z; };
    };
};

template <typename T>
struct Vector<T, 4> final: public VecMemberGenerator<T, 4> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 4>::setVec(this);
        *this = o;
    }

    Vector() = default;

    Vector(const std::initializer_list<T>& datas) {
        VecMemberGenerator<T, 4>::setVec(this);
        VecMemberGenerator<T, 4>::initVec(datas);
    }

    union {
        T data[4];
        struct { T x, y, z, w; };
    };
};

// some full specialization for SIMD

#ifdef USE_SIMD

template <>
struct Vector<float, 4> final {
public:
    Vector(float x, float y, float z, float w) {
        data_ = _mm_setr_ps(x, y, z, w);
    }

    Vector operator+(const Vector& oth) const {
        return Vector{_mm_add_ps(data_, oth.data_)};
    }

    Vector operator-(const Vector& oth) const {
        return Vector{_mm_sub_ps(data_, oth.data_)};
    }

    Vector operator*(const Vector& oth) const {
        return Vector{_mm_mul_ps(data_, oth.data_)};
    }

    Vector operator/(const Vector& oth) const {
        return Vector{_mm_div_ps(data_, oth.data_)};
    }

    Vector& operator+=(const Vector& oth) {
        *this = *this + oth;
        return *this;
    }

    Vector& operator-=(const Vector& oth) {
        *this = *this - oth;
        return *this;
    }

    Vector& operator*=(const Vector& oth) {
        *this = *this * oth;
        return *this;
    }

    Vector& operator*=(float value) {
        *this = *this * value;
        return *this;
    }

    Vector& operator/=(const Vector& oth) {
        *this = *this / oth;
        return *this;
    }

    Vector& operator/=(float value) {
        *this = *this / value;
        return *this;
    }

    Vector operator*(float value) const {
        return *this * Vector(value, value, value, value);
    }

    Vector operator/(float value) const {
        return *this / Vector(value, value, value, value);
    }

    float Dot(const Vector& oth) const {
        return ::cgmath::Dot(*this, oth);
    }

    float x() const {
        return _mm_cvtss_f32(data_);
    }

    float y() const {
        float p[4];
        _mm_store_ps(p, data_);
        return p[1];
    }

    float z() const {
        float p[4];
        _mm_store_ps(p, data_);
        return p[2];
    }

    float w() const {
        float p[4];
        _mm_store_ps(p, data_);
        return p[3];
    }

    void Print(std::ostream& o) const {
        float p[4];
        _mm_store_ps(p, data_);
        o << "Vec4(" << p[0] << ", " << p[1] << ", " << p[2] << ", " << p[3] << ")";
    }

    bool operator==(const Vector& o) const {
        return (_mm_movemask_ps(_mm_cmpeq_ps(data_, o.data_)) & 15) == 15;
    }

    bool operator!=(const Vector& o) const {
        return !(*this == o);
    }

private:
    __m128 data_;

    friend float Dot(const Vector<float, 4>& lhs, const Vector<float, 4>& rhs);

    explicit Vector(__m128 data): data_(data) {}
};

Vector<float, 4> operator*(float value, const Vector<float, 4>& v) {
    return v * Vector<float, 4>(value, value, value, value);
}

inline float Dot(const Vector<float, 4>& lhs, const Vector<float, 4>& rhs) {
    return _mm_cvtss_f32(_mm_dp_ps(lhs.data_, rhs.data_, 0xFF));
}

inline std::ostream& operator<<(std::ostream& o, const Vector<float, 4>& v) {
    v.Print(o);
    return o;
}

#endif

// some types

#ifndef VECTOR_DATA_TYPE
#define VECTOR_DATA_TYPE float
#endif

using Vec2 = Vector<VECTOR_DATA_TYPE, 2>;
using Vec3 = Vector<VECTOR_DATA_TYPE, 3>;
using Vec4 = Vector<VECTOR_DATA_TYPE, 4>;


// some math function for vector

template <typename T, unsigned int AttrNum>
std::ostream& operator<<(std::ostream& o, const Vector<T, AttrNum>& v) {
    o << "Vec" << AttrNum << "(";
    for (int i = 0; i < AttrNum; i++) {
        o << v.data[i];
        if (i != AttrNum - 1) {
            o << ", ";
        }
    }
    o << ")";
    return o;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator+(const Vector<T, AttrNum>& v1, const Vector<U, AttrNum>& v2) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] + v2.data[i];
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator-(const Vector<T, AttrNum>& v1, const Vector<U, AttrNum>& v2) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] - v2.data[i];
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator*(const Vector<T, AttrNum>& v1, const Vector<U, AttrNum>& v2) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] * v2.data[i];
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator*(const Vector<T, AttrNum>& v1, const U& scalar) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] * scalar;
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator/(const Vector<T, AttrNum>& v1, const Vector<U, AttrNum>& v2) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] / v2.data[i];
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator*(const U& scalar, const Vector<T, AttrNum>& v1) {
    return v1 * scalar;
}

template <typename T, typename U, unsigned int AttrNum>
auto operator/(const Vector<T, AttrNum>& v1, const U& scalar) {
    Vector<std::common_type_t<T, U>, AttrNum> result;
    for (int i = 0; i < AttrNum; i++) {
        result.data[i] = v1.data[i] / scalar;
    }
    return result;
}

template <typename T, typename U, unsigned int AttrNum>
auto Dot(const Vector<T, AttrNum>& v1, const Vector<U, AttrNum>& v2) {
    using CommonType = std::common_type_t<T, U>;
    CommonType sum = CommonType{};

    for (int i = 0; i < AttrNum; i++) {
        sum += v1.data[i] * v2.data[i];
    }

    return sum;
}

template <typename T, typename U>
auto Cross(const Vector<T, 2>& v1, const Vector<U, 2>& v2) {
    using CommonType = std::common_type_t<T, U>;
    CommonType result = CommonType{};

    result = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];

    return result;
}

template <typename T, typename U>
auto Cross(const Vector<T, 3>& v1, const Vector<U, 3>& v2) {
    using CommonType = std::common_type_t<T, U>;
    Vector<CommonType, 3> result;

    result.data[0] = v1.data[1] * v2.data[2] - v1.data[2] * v2.data[1];
    result.data[1] = v1.data[2] * v2.data[0] - v1.data[0] * v2.data[2];
    result.data[2] = v1.data[0] * v2.data[1] - v1.data[1] * v2.data[0];

    return result;
}

template <typename T, unsigned int AttrNum>
T LengthSquare(const Vector<T, AttrNum>& v) {
    T sum = T{};

    for (auto& data : v.data) {
        sum += data * data;
    }

    return sum;
}

template <typename T, unsigned int AttrNum>
auto Length(const Vector<T, AttrNum>& v) {
    return std::sqrt(LengthSquare(v));
}

template <typename T, unsigned int AttrNum>
auto Normalize(const Vector<T, AttrNum>& v) {
    return v / Length(v);
}

template <typename T, typename U, unsigned int AttrNum>
auto Project(const Vector<T, AttrNum>& src, const Vector<T, AttrNum>& des) {
    return src.Dot(des) / des.Length();
}

template <typename T, unsigned int AttrNum>
auto Reflect(const Vector<T, AttrNum>& v, const Vector<T, AttrNum>& n) {
    auto norm = Project(v, n);
    return 2 * norm - v;
}

/* @return  T radians*/
template <typename T>
auto GetAngle(const Vector<T, 2>& v) {
    return std::atan(v.y, v.x);
}

template <typename T, unsigned int AttrNum>
auto GetAngleBetweenVecs(const Vector<T, AttrNum>& v1, const Vector<T, AttrNum>& v2) {
    return std::acos(v1.Dot(v2) / (v1.Length() * v2.Length()));
}

/* @return  Vector<T, 3>  in radians*/
template <typename T>
Vector<T, 3> GetAngle(const Vector<T, 3>& v) {
    return Vector<T, 3>(GetAngleBetweenVecs(v, Vector<T, 3>(1, 0, 0)),
                        GetAngleBetweenVecs(v, Vector<T, 3>(0, 1, 0)),
                        GetAngleBetweenVecs(v, Vector<T, 3>(0, 0, 1)));
}

/* a help class to auto-generate member func for Vector
   use CRTP (see: https://en.wikibooks.org/wiki/More_C%2B%2B_Idioms/Curiously_Recurring_Template_Pattern) */
template <typename T, unsigned int AttrNum>
class VecMemberGenerator {
public:
    using VecT = Vector<T, AttrNum>;

    VecT& operator+=(const VecT& o) const {
        for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] += o.data[i];
        }
        return *vec_;
    }

    VecT& operator-=(const VecT& o) const {
        for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] -= o.data[i];
        }
        return *vec_;
    }

    VecT& operator*=(const VecT& o) const {
        for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] *= o.data[i];
        }
        return *vec_;
    }

    VecT& operator/=(const VecT& o) const {
        for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] /= o.data[i];
        }
        return *vec_;
    }

	VecT& operator-() {
		for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] = -vec_->data[i];
		}
		return *vec_;
	}

    void Normalize() {
        *vec_ = ::cgmath::Normalize(*vec_);
    }

    template <typename U>
    auto Dot(const Vector<U, AttrNum>& o) {
        return cgmath::Dot(*vec_, o);
    }

    auto LengthSquare() {
        return ::cgmath::LengthSquare(*vec_);
    }

    auto Length() {
        return ::cgmath::Length(*vec_);
    }

    template <typename U>
    VecT& operator=(const Vector<U, AttrNum>& o) {
        if (&o == vec_) {
            return *vec_;
        }

        for (int i = 0; i < AttrNum; i++) {
            vec_->data[i] = o.vec_->data[i];
        }
        return *vec_;
    }

    template <typename U>
    bool operator==(const Vector<U, AttrNum>& o) const {
        for (int i = 0; i < AttrNum; i++) {
            if (vec_->data[i] != o.data[i])
                return false;
        }
        return true;
    }

    template <typename U>
    bool operator!=(const Vector<U, AttrNum>& o) const {
        return !(*vec_ == o);
    }

    const T& operator[](unsigned int idx) const {
        return vec_->data[idx];
    }

     T& operator[](unsigned int idx) {
        return vec_->data[idx];
    }

protected:
    VecT* vec_ = nullptr;

    VecMemberGenerator() = default;
    VecMemberGenerator(const VecMemberGenerator& o) {}

    void setVec(VecT* v) { vec_ = v; }

    void initVec(const std::initializer_list<T>& datas) {
        auto it = datas.begin();
        int i = 0;
        while (it != datas.end() && i < AttrNum) {
            vec_->data[i] = *it;
            it++;
            i++;
        }

        while (i < AttrNum) {
            vec_->data[i] = T{};
            i++;
        }
    }
};

// matrx, column first

template <typename T, int Col, int Row>
class Matrix final {
public:
    Matrix() {}

    Matrix(const std::vector<Vector<T, Row>>& cols) {
        for (int x = 0; x < Col; x++) {
            for (int y = 0; y < Row; y++) {
                Set(x, y, cols[x].data[y]);
            }
        }
    }

    Matrix(const std::initializer_list<T>& datas) {
        auto it = datas.begin();
        int i = 0;
        while (it != datas.end() && i < Col * Row) {
            int x = i % Col,
                y = i / Col;
            Set(x, y, *it);
            i++;
            it++;
        }
    }

    void Set(int x, int y, const T& value) {
        data_[y + x * Row] = value;
    }

    const T& Get(int x, int y) const {
        return data_[y + x * Row];
    }

     T& Get(int x, int y) {
        return data_[y + x * Row];
    }

    constexpr int W() const { return Col; }
    constexpr int H() const { return Row; }

    Matrix operator*(const Matrix& o) const {
        Matrix matrix;
        for (int x = 0; x < Col; x++) {
            for (int y = 0; y < Row; y++) {
                matrix.Set(x, y, Get(x, y) * o.Get(x, y));
            }
        }
        return matrix;
    }

    Matrix operator/(const Matrix& o) const {
        Matrix matrix;
        for (int x = 0; x < Col; x++) {
            for (int y = 0; y < Row; y++) {
                matrix.Set(x, y, Get(x, y) / o.Get(x, y));
            }
        }
        return matrix;
    }

    Matrix operator+(const Matrix& o) const {
        Matrix matrix;
        for (int x = 0; x < Col; x++) {
            for (int y = 0; y < Row; y++) {
                matrix.Set(x, y, Get(x, y) + o.Get(x, y));
            }
        }
        return matrix;
    }

    Matrix operator-(const Matrix& o) const {
        Matrix matrix;
        for (int x = 0; x < Col; x++) {
            for (int y = 0; y < Row; y++) {
                matrix.Set(x, y, Get(x, y) - o.Get(x, y));
            }
        }
        return matrix;
    }

    template <typename U, int Col2>
    auto Mul(const Matrix<U, Col, Col2>& m) const {
        return ::cgmath::Mul(*this, m);
    }

private:
    T data_[Col * Row];
};

template <typename T, typename U, int Col, int Row>
auto operator*(const Matrix<T, Col, Row>& m, const U& scalar)  {
    Matrix<std::common_type_t<T, U>, Col, Row> matrix;
    for (int x = 0; x < Col; x++) {
        for (int y = 0; y < Row; y++) {
            matrix.Set(x, y, m.Get(x, y) * scalar);
        }
    }
    return matrix;
}

template <typename T, typename U, int Col, int Row>
auto operator*(const U& scalar, const Matrix<T, Col, Row>& m)  {
    return m * scalar;
}

template <typename T, typename U, int Common, int Row, int Col>
auto Mul(const Matrix<T, Common, Row>& m1, const Matrix<U, Col, Common>& m2) {
    Matrix<std::common_type_t<T, U>, Col, Row> result;
    for (int i = 0; i < Row; i++) {
        for (int j = 0; j < Col; j++) {
            T sum = T{};
            for (int k = 0; k < Common; k++) {
                sum += m1.Get(k, i) * m2.Get(j, k);
            }
            result.Set(j, i, sum);
        }
    }
    return result;
}

template <typename T, typename U, int Row, int Col>
auto Mul(const Matrix<T, Col, Row>& m, const Vector<U, Col>& v) {
    Vector<std::common_type_t<T, U>, Row> result;
    for (int i = 0; i < Row; i++) {
        T sum = T{};
        for (int j = 0; j < Col; j++) {
            sum += m.Get(j, i) * v.data[j];
        }
        result.data[i] = sum;
    }
    return result;
}

using Mat22 = Matrix<float, 2, 2>;
using Mat33 = Matrix<float, 3, 3>;
using Mat44 = Matrix<float, 4, 4>;

template <typename T, int Col, int Row>
std::ostream& operator<<(std::ostream& o, const Matrix<T, Col, Row>& m) {
    o << "[" << std::endl;
    for (int y = 0; y < Row; y++) {
        for (int x = 0; x < Col; x++) {
            o << m.Get(x, y) << "\t";
        }
        std::cout << std::endl;
    }
    o << "]";

    return o;
}

}