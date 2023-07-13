#pragma once

#include <iostream>
#include <fstream>
#include <vector>
#include <string_view>
#include <cassert>

struct Color final {
    float r, g, b;
};

class PPM final {
 public:
    PPM(unsigned int w, unsigned int h): w_(w), h_(h) {
        data_.resize(w * h, Color{0, 0, 0});
    }

    void Set(const Color& c, int x, int y) {
        assert(x >= 0 && y >= 0);
        data_.at(x + y * w_) = c;
    }

    auto W() const {
        return w_;
    }

    auto H() const {
        return h_;
    }

    void Fill(const Color& color) {
        data_.assign(w_ * h_, color);
    }

    void Save(std::string_view filename) {
        std::ofstream file(filename.data());
        if (file.fail()) {
            throw std::runtime_error("open file failed");
        }

        file << "P3" << std::endl
             << W() << " " << H() << std::endl
             << "255" << std::endl;

        for (int y = 0; y < H(); y++) {
            for (int x = 0; x < W(); x++) {
                auto& color = data_.at(x + y * W());
                file << int(color.r * 255) << " " << int(color.g * 255) << " " << int(color.b * 255) << " ";
            }
            file << std::endl;
        }
    }

 private:
    std::vector<Color> data_;
    unsigned int w_;
    unsigned int h_;
};
