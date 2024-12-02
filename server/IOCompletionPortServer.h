#pragma once
#include <WinSock2.h>
#include <Windows.h>
#include <thread>
#include <vector>
#include <string>

constexpr int MAX_SOCKETBUF = 1024;
constexpr int PORT = 9000;

enum IO_TYPE {
	IO_RECV,
	IO_SEND
};

struct OverlappedEx {
	WSAOVERLAPPED wsaOverlapped;
	WSABUF wsaBuf;
	char dataBuffer[MAX_SOCKETBUF];
	IO_TYPE ioType;
};

struct ClientInfo {
	SOCKET socketClient = INVALID_SOCKET;
	OverlappedEx recvOverlapped;
	OverlappedEx sendOverlapped;
};

class IOCompletionPort {
public:
	IOCompletionPort();
	virtual ~IOCompletionPort() = default;

public:
	bool InitSocket();
	void CloseSocket();

	bool BindandListen(int port);
	bool StartServer();
	void DestroyThread();

private:
	void HandleError(std::string msg);

	bool CreateWorkerThread();
	bool CreateAccepterThread();

	ClientInfo* GetEmptyClientInfo();

	bool BindIOCompletionPort(ClientInfo* clientInfo);
	bool BindRecv(ClientInfo* clientInfo);
	bool SendMsg(ClientInfo* clientInfo, char* pMsg, int length);
	bool IOProcess(ClientInfo* clientInfo, OverlappedEx* overlapped, DWORD& tarnsferred);

	void WorkerThread();
	void AccepterThread();

private:
	ClientInfo* clientInfo = nullptr;
	SOCKET listenSocket = INVALID_SOCKET;
	int clientCount = 0;
	std::vector<std::thread> workerThreads;
	std::vector<std::thread> accpeterThreads;
	HANDLE iocpHandle = nullptr;
	bool workerRun = false;
	bool accepterRun = false;
	char socketBuffer[MAX_SOCKETBUF] = { 0 };
};