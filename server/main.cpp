#include <iostream>
#include <winsock2.h>
#include <mswsock.h>
#include <windows.h>
#include <thread>
#include <vector>

#pragma comment(lib, "ws2_32.lib")

constexpr int BUFFER_SIZE = 1024;
constexpr int PORT = 9000;

struct ClientContext {
    SOCKET socket;
    char buffer[BUFFER_SIZE];
    OVERLAPPED overlapped;
    WSABUF wsabuf;
};

void handleError(const char* message) {
    std::cerr << message << " Error Code: " << WSAGetLastError() << std::endl;
    exit(EXIT_FAILURE);
}

void workerThread(HANDLE iocpHandle) {
    while (true) {
        DWORD bytesTransferred = 0;
        ULONG_PTR completionKey = 0;
        LPOVERLAPPED overlapped = nullptr;

        BOOL result = GetQueuedCompletionStatus(
            iocpHandle, &bytesTransferred, &completionKey, &overlapped, INFINITE);

        if (!result || bytesTransferred == 0) {
            if (overlapped) {
                ClientContext* context = reinterpret_cast<ClientContext*>(completionKey);
                closesocket(context->socket);
                delete context;
            }
            continue;
        }

        ClientContext* context = reinterpret_cast<ClientContext*>(completionKey);
        if (overlapped) {
            context->buffer[bytesTransferred] = '\0';
            std::cout << "Received: " << context->buffer << std::endl;

            // Echo back
            context->wsabuf.len = bytesTransferred;
            DWORD sentBytes = 0;
            WSASend(context->socket, &context->wsabuf, 1, &sentBytes, 0, nullptr, nullptr);

            // Prepare for next receive
            DWORD flags = 0;
            ZeroMemory(&context->overlapped, sizeof(OVERLAPPED));
            context->wsabuf.buf = context->buffer;
            context->wsabuf.len = BUFFER_SIZE;

            WSARecv(context->socket, &context->wsabuf, 1, nullptr, &flags, &context->overlapped, nullptr);
        }
    }
}

int main() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        handleError("WSAStartup failed");
    }

    SOCKET listenSocket = WSASocket(AF_INET, SOCK_STREAM, 0, nullptr, 0, WSA_FLAG_OVERLAPPED);
    if (listenSocket == INVALID_SOCKET) {
        handleError("Failed to create listen socket");
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == SOCKET_ERROR) {
        handleError("Bind failed");
    }

    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        handleError("Listen failed");
    }

    HANDLE iocpHandle = CreateIoCompletionPort(INVALID_HANDLE_VALUE, nullptr, 0, 0);
    if (!iocpHandle) {
        handleError("Failed to create IOCP");
    }

    CreateIoCompletionPort(reinterpret_cast<HANDLE>(listenSocket), iocpHandle, 0, 0);

    std::vector<std::thread> threads;
    const int threadCount = std::thread::hardware_concurrency();
    for (int i = 0; i < threadCount; ++i) {
        threads.emplace_back(workerThread, iocpHandle);
    }

    std::cout << "Echo server running on port " << PORT << std::endl;

    while (true) {
        sockaddr_in clientAddr{};
        int addrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(listenSocket, reinterpret_cast<sockaddr*>(&clientAddr), &addrLen);
        if (clientSocket == INVALID_SOCKET) {
            std::cerr << "Failed to accept client connection" << std::endl;
            continue;
        }

        auto* clientContext = new ClientContext{ clientSocket };
        clientContext->wsabuf.buf = clientContext->buffer;
        clientContext->wsabuf.len = BUFFER_SIZE;

        CreateIoCompletionPort(reinterpret_cast<HANDLE>(clientSocket), iocpHandle, reinterpret_cast<ULONG_PTR>(clientContext), 0);

        DWORD flags = 0;
        ZeroMemory(&clientContext->overlapped, sizeof(OVERLAPPED));
        if (WSARecv(clientSocket, &clientContext->wsabuf, 1, nullptr, &flags, &clientContext->overlapped, nullptr) == SOCKET_ERROR) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                delete clientContext;
                closesocket(clientSocket);
            }
        }
    }

    for (auto& thread : threads) {
        if (thread.joinable()) {
            thread.join();
        }
    }

    closesocket(listenSocket);
    CloseHandle(iocpHandle);
    WSACleanup();

    return 0;
}
