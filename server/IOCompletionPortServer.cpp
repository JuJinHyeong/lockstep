#include "IOCompletionPortServer.h"
#include <iostream>
#pragma comment(lib, "ws2_32.lib")

IOCompletionPort::IOCompletionPort()
{
}

void IOCompletionPort::HandleError(std::string msg)
{
	std::cerr << msg << " Error Code: " << WSAGetLastError() << std::endl;
	exit(EXIT_FAILURE);
}
