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
    socket->Listen(SOMAXCONN);
    std::cout << "listening..." << std::endl;

    int iResult = 0;
    char buf[1024] = {0};
    net::Result<int, std::unique_ptr<net::Socket>> acceptResult = socket->Accept();
    if (acceptResult.result != 0) {
        iResult = -1;
    }

    std::cout << "accepted" << std::endl;

    do {
        auto& clientSocket = acceptResult.value;

        iResult = clientSocket->Recv(buf, sizeof(buf));
        std::cout << "recived: " << iResult << std::endl;
        if (iResult == 0) {
            std::cout << "client closed" << std::endl;
        } else if (iResult > 0) {
            std::cout << "recived: " << buf << std::endl;
        }
    } while (iResult > 0);

    return 0;
}