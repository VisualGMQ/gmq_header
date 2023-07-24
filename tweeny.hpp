//! @file tweeny.hpp
//! @brief a small tween lib for game and gui interaction

#pragma once

#include <functional>
#include <cmath>
#include <algorithm>
#include <vector>

namespace tweeny {

enum class TweenyDirection {
    Forward = 1,
    Backward = -1,
};

template <typename T>
using TweenFunc = std::function<T(float, T, T)>;

template <typename T>
struct Easing {
    inline static TweenFunc<T> Linear = [](float t, T a, T b) { return a + (b - a) * t; };
};

template <typename T>
class Tween final {
public:

    static Tween From(T from) {
        Tween tweeny;
        tweeny.keyPoints_.push_back({from, 0});
        return tweeny;
    }

    Tween& To(T value) {
        keyPoints_.push_back({value, 0});
        return *this;
    }

    Tween& Via(TweenFunc<T> func) {
        func_ = func;
        return *this;
    }

    Tween& Loop(int loop) {
        loop_ = loop;
        return *this;
    }

    Tween& Timing(float time) {
        if (!keyPoints_.empty()) {
            keyPoints_.back().time = time;
        }
        return *this;
    }

    Tween& During(float during) {
        if (keyPoints_.size() >= 2) {
            keyPoints_.back().time = keyPoints_[keyPoints_.size() - 2].time + during;
        }
        return *this;
    }

    auto Direction() const {
        return direction_;
    }

    Tween& Forward() {
        direction_ = TweenyDirection::Forward;
        return *this;
    }

    Tween& Backward() {
        direction_ = TweenyDirection::Backward;
        return *this;
    }

    void Step(T step) {
        if (keyPoints_.size() <= 2) {
            return;
        }

        curDur_ += static_cast<int>(direction_) * step;
        const auto& from = keyPoints_[curPoint_];
        const auto& to = keyPoints_[curPoint_ + 1];
        if (curDur_ < from.time) {
            curPoint_ --;
        } else if (curDur_ > to.time) {
            curPoint_ ++;
        }

        if (curPoint_ < 0) {
            curPoint_ = 0;
            curDur_ = keyPoints_[curPoint_].time;
            if (loop_ != 0) {
                curPoint_ = keyPoints_.size() - 2;
                curDur_ = keyPoints_.back().time;
                loop_ -= loop_ < 0 ? 0 : 1;
            }
        } else if (curPoint_ + 1 >= keyPoints_.size()) {
            curPoint_ = keyPoints_.size() - 1;
            curDur_ = keyPoints_[curPoint_].time;
            if (loop_ != 0) {
                curPoint_ = 0;
                curDur_ = 0;
                loop_ -= loop_ < 0 ? 0 : 1;
            }
        }
    }

    T CurValue() const {
        if (keyPoints_.size() < 2) {
            if (keyPoints_.size() == 1) {
                return keyPoints_.back().value;
            } else {
                return T{};
            }
        }

        if (curPoint_ >= keyPoints_.size() - 1) {
            return keyPoints_.back().value;
        }
        
        const auto& from = keyPoints_[curPoint_];
        const auto& to = keyPoints_[curPoint_ + 1];
        return func_(static_cast<float>(curDur_ - from.time) / (to.time - from.time),
                     from.value, to.value);
    }

    T CurTick() const {
        return curDur_;
    }

private:
    struct KeyPoint {
        T value;
        float time;
    };

    TweenFunc<T> func_ = Easing<T>::Linear;
    std::vector<KeyPoint> keyPoints_;
    int loop_ = 0;
    TweenyDirection direction_ = TweenyDirection::Forward;
    T curDur_ = 0;
    size_t curPoint_ = 0;
};

}