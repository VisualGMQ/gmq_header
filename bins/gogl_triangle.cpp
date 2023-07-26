#include "gogl.hpp"
#include "GLFW/glfw3.h"
#include "log.hpp"

#define STB_IMAGE_IMPLEMENTATION
#include "3rdlibs/stb_image.h"

#include <iterator>
#include <iostream>

using namespace gogl;

struct Context final {
    std::unique_ptr<BufferManager> bufmgr;
    std::unique_ptr<TextureManager> texmgr;
    std::unique_ptr<ShaderManager> shadermgr;
    std::unique_ptr<AttrPointerManager> attrmgr;

    Context()
        : bufmgr(std::make_unique<BufferManager>()),
          texmgr(std::make_unique<TextureManager>()),
          shadermgr(std::make_unique<ShaderManager>()),
          attrmgr(std::make_unique<AttrPointerManager>()) { }
};

void ErrorCallback(int error, const char* description) {
    LOGE("[GLFW]: ", error, " - ", description);
}

constexpr int WindowWidth = 1024;
constexpr int WindowHeight = 720;

int main() {
    if (!glfwInit()) {
        LOGF("[APP]: glfw init failed");
        return 1;
    }

    glfwSetErrorCallback(ErrorCallback);

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(WindowWidth, WindowHeight, "VisualDebugger", NULL, NULL);
    if (!window) {
        LOGE("[GLFW]: create window failed");
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1); // Enable vsync
    if (gladLoadGL() == 0) {
        LOGF("[GLAD]: glad init failed!");
        return 2;
    }

    // create context
    std::unique_ptr<Context> ctx = std::make_unique<Context>();

    // load shader
    std::ifstream vertex_file("./res/shaders/vert.shader"),
                  fragment_file("./res/shaders/frag.shader");
    std::string vertex_source(std::istreambuf_iterator<char>(vertex_file), (std::istreambuf_iterator<char>())),
                fragment_source(std::istreambuf_iterator<char>(fragment_file), (std::istreambuf_iterator<char>()));
    ShaderModule vertex = ShaderModule::CreateVertexShader(vertex_source);
    ShaderModule fragment = ShaderModule::CreateFragmentShader(fragment_source);
    auto& shader = ctx->shadermgr->Create("shader", vertex, fragment);

    // prepare data
    float vertices[] = {
        // vec3 position, vec2 color, texcoord
        -0.5, -0.5, 0,    1.0, 0.0,   0.0, 0.0,
         0.5, -0.5, 0,    0.0, 1.0,   1.0, 0.0,
           0,  0.5, 0,    0.0, 0.0,   0.5, 1.0,
    };

    // create buffer
    auto& buffer = ctx->bufmgr->Create("buffer", BufferType::Array);
    buffer.Bind();
    buffer.SetData(vertices, sizeof(vertices));

    // create attribute pointer
    auto& pointer = ctx->attrmgr->Create("attr", BufferLayout::CreateFromTypes({
        Attribute::Type::Vec3,
        Attribute::Type::Vec2,
        Attribute::Type::Vec2,
    }));
    pointer.Bind();

    // create texture
    int w, h;
    stbi_set_flip_vertically_on_load(1);
    void* pixels = stbi_load("./res/img/img.jpg", &w, &h, nullptr, 3);
    auto &texture = ctx->texmgr->Create("image",
                                        Texture::Type::Dimension2,
                                        pixels,
                                        w, h,
                                        Sampler::CreateLinearRepeat(),
                                        Format::RGB, Format::RGB);

    GL_CALL(glViewport(0, 0, WindowWidth, WindowHeight));

    shader.SetInt("image", 0);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        GL_CALL(glClearColor(0.2, 0.2, 0.2, 1.0));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));
        pointer.Bind();
        shader.Use();
        texture.Bind();

        shader.DrawArray(PrimitiveType::Triangles, 0, 3);

        glfwSwapBuffers(window);
    }

    ctx.reset();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}