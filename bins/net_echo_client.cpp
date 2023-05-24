#define NET_IMPLEMENTATION
#include "net.hpp"

constexpr uint16_t PORT = 8080;

int main() {
    auto netInstance = net::Init();
    auto result = net::AddrInfoBuilder::CreateTCP("localhost", PORT).Build();
    if (result.result != 0) {
        std::cerr << "create TCP addrinfo failed: " << net::GetLastError() << std::endl;
    }

    auto socket = netInstance->CreateSocket(result.value);
    socket->Bind();

    std::cout << "connecting..." << std::endl;
    socket->Connect();

    std::cout << "connected, send 'quit' to close connect" << std::endl;

    int iResult = 0;
    char buf[1024] = {0};
    do {
        std::cin >> buf;
        iResult = socket->Send(buf, strlen(buf) + 1);
        std::cout << "send: " << iResult << std::endl;
        if (iResult == 0) {
            std::cout << "server closed" << std::endl;
        }
    } while (iResult > 0);

    return 0;
}