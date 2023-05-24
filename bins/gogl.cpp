#include "gogl.hpp"
#include "GLFW/glfw3.h"
#include "log.hpp"

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
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);
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

    glViewport(0, 0, WindowWidth, WindowHeight);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        GL_CALL(glClearColor(0.2, 0.2, 0.2, 1.0));
        GL_CALL(glClear(GL_COLOR_BUFFER_BIT));

        glfwSwapBuffers(window);
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}