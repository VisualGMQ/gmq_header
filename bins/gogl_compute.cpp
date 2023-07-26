#include "gogl.hpp"
#include "GLFW/glfw3.h"

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

int main() {
    if (!glfwInit()) {
        LOGF("[APP]: glfw init failed");
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    GLFWwindow* window = glfwCreateWindow(200, 200, "VisualDebugger", NULL, NULL);
    if (!window) {
        LOGE("[GLFW]: create window failed");
    }

    glfwMakeContextCurrent(window);


    if (gladLoadGL() == 0) {
        LOGF("[GLAD]: glad init failed!");
        return 2;
    }
    PhysicDevice phyDev;
    auto size = phyDev.GetComputeWorkGroupMaxSize();
    std::cout << "max compute work group size:" << size[0] << ", " << size[1] << ", " << size[2] << std::endl;
    std::cout << "max compute work invoke count:" << phyDev.GetComputeGroupInvocationMaxCount() << std::endl;

    std::unique_ptr<Context> ctx = std::make_unique<Context>();

    std::ifstream shader_file("./res/shaders/compute.shader");
    std::string shader_source(std::istreambuf_iterator<char>(shader_file), (std::istreambuf_iterator<char>()));
    auto& shader = ctx->shadermgr->Create("compute", ShaderModule::CreateComputeShader(shader_source));
    
    float buf[256] = {0};
    auto& inputBufferA = ctx->bufmgr->Create("input1", BufferType::ShaderStorage);
    auto& inputBufferB = ctx->bufmgr->Create("input2", BufferType::ShaderStorage);
    auto& outputBuffer = ctx->bufmgr->Create("output", BufferType::ShaderStorage);

    for (int i = 0; i < 256; i++) {
        buf[i] = i;
    }
    inputBufferA.SetData(buf, sizeof(buf));
    inputBufferA.Bind2Base(0);

    for (int i = 0; i < 256; i++) {
        buf[i] = i + 1;
    }
    inputBufferB.SetData(buf, sizeof(buf));
    inputBufferB.Bind2Base(1);

    outputBuffer.ExtendSize(sizeof(buf));
    outputBuffer.Bind2Base(2);

    shader.Use();
    shader.DispatchCompute(256, 1, 1);
    shader.WaitMemoryBarrier(BarrierType::ShaderStorage);

    float data[256] = {0};
    outputBuffer.GetSubData(data, 0, sizeof(data));
    for (int i = 0; i < 256; i++) {
        std::cout << (int)data[i] << std::endl;
    }

    ctx.reset();

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}