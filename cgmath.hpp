#pragma once

#include <utility>
#include <iostream>
#include <cmath>

namespace cgmath {

#define PI 3.14159265358979

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
struct Vector: public VecMemberGenerator<T, AttrNum> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 2>::setVec(this);
        *this = o;
    }

    Vector() = default;

    template <typename... Params>
    Vector(const Params&... params) {
        VecMemberGenerator<T, AttrNum>::setVec(this);
        VecMemberGenerator<T, AttrNum>::initVec(params...);
    }

    T data[AttrNum];
};

template <typename T>
struct Vector<T, 2>: public VecMemberGenerator<T, 2> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 2>::setVec(this);
        *this = o;
    }

    Vector() = default;

    template <typename... Params>
    Vector(const Params&... params) {
        VecMemberGenerator<T, 2>::setVec(this);
        VecMemberGenerator<T, 2>::initVec(params...);
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
struct Vector<T, 3>: public VecMemberGenerator<T, 3> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 3>::setVec(this);
        *this = o;
    }

    Vector() = default;

    template <typename... Params>
    Vector(const Params&... params) {
        VecMemberGenerator<T, 3>::setVec(this);
        VecMemberGenerator<T, 3>::initVec(params...);
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
struct Vector<T, 4>: public VecMemberGenerator<T, 4> {
    Vector(const Vector& o) {
        VecMemberGenerator<T, 4>::setVec(this);
        *this = o;
    }

    Vector() = default;

    template <typename... Params>
    Vector(const Params&... params) {
        VecMemberGenerator<T, 4>::setVec(this);
        VecMemberGenerator<T, 4>::initVec(params...);
    }

    union {
        T data[4];
        struct { T x, y, z, w; };
    };
};

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

protected:
    VecT* vec_ = nullptr;

    VecMemberGenerator() = default;
    VecMemberGenerator(const VecMemberGenerator& o) {}

    void setVec(VecT* v) { vec_ = v; }

    template <typename... Params>
    void initVec(const Params&... params) {
        static_assert(sizeof...(params) <= AttrNum);
        doInit<0>(params...);
    }

private:
    template <unsigned int idx, typename Param, typename... Params>
    void doInit(const Param& param, Params&&... params) {
        if (idx < AttrNum) {
            vec_->data[idx] = param;
            doInit<idx + 1>(params...);
        }
    }

    template <unsigned int idx>
    void doInit() {
        for (int i = idx; i < AttrNum; i++) {
            vec_->data[idx] = T{};
        }
    }
};

}