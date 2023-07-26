#include "renderer2d.hpp"
#include "GLFW/glfw3.h"
#define STB_IMAGE_IMPLEMENTATION
#include "3rdlibs/stb_image.h"

constexpr int WindowWidth = 1024;
constexpr int WindowHeight = 720;

int main() {
    if (!glfwInit()) {
        LOGF("[APP]: glfw init failed");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_SAMPLES, 8);
    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "renderer2d", NULL, NULL);
    if (!window) {
        LOGE("[GLFW]: create window failed");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    if (gladLoadGL() == 0) {
        LOGF("[GLAD]: glad init failed!");
        return 2;
    }

    auto renderer = std::make_unique<renderer2d::Renderer2D>();
    renderer->SetViewport({0, 0}, {WindowWidth, WindowHeight});
    renderer->SetClearColor({0.2, 0.2, 0.2, 1.0});

    int w, h;
    void* datas = stbi_load("res/img/img.jpg", &w, &h, nullptr, 4);
    std::unique_ptr<gogl::Texture> texture = std::make_unique<gogl::Texture>(
        gogl::Texture::Type::Dimension2, datas, w, h,
        gogl::Sampler::CreateLinearRepeat(), gogl::Format::RGBA,
        gogl::Format::RGBA);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        renderer->Clear();

        renderer->DrawLine({0, 0}, {WindowWidth, WindowHeight}, {1, 0, 0, 1});
        renderer->DrawRect(cgmath::Rect{100, 200, 50, 100}, {0, 1, 0, 1},
                           cgmath::CreateZRotation(cgmath::Deg2Rad(15.0f)));

        renderer->FillRect(cgmath::Rect{400, 200, 50, 100}, {0, 1, 0, 1},
                           cgmath::CreateZRotation(cgmath::Deg2Rad(15.0f)),
                           renderer2d::Image{texture.get(), std::nullopt});

        glfwSwapBuffers(window);
    }

    texture.reset();
    renderer.reset();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;


    return 0;
}