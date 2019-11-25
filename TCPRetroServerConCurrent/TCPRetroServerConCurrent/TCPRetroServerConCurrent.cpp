
// TCP并发回射服务器

#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <iostream>
#pragma comment(lib,"ws2_32.lib")


#define WSAERROR -1		// WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define RECVERROR -4	// 接收消息错误
#define LISTENERROR -5	// 监听错误
#define BUFFSIZE 256	// 缓冲区大小

static int ServerPort = 9999;

struct Para {
	SOCKET clientSocket;
	SOCKADDR_IN clientAddrIn;
};

DWORD WINAPI Echo(LPVOID pParam)
{
	Para* para = (Para*)pParam;
	printf("A Thread Created...\n");

	int recvCount = -1;
	char echo[] = "echo: ";
	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区

	SOCKET clientSocket = para->clientSocket;
	SOCKADDR_IN clientAddrIn = para->clientAddrIn;

	// 服务器接收消息直到客户端退出
	while (1)
	{
		recvCount = recv(clientSocket, recvBuff, BUFFSIZE - 1, 0);
		if (recvCount < 0)    // 接收出错
		{
			printf("消息接受错误\n");
			printf("error: ");
			printf("%d\n", WSAGetLastError());
			closesocket(clientSocket);
			break;
		}
		else if (recvCount == 0)	//连接关闭
		{
			printf("客户端连接关闭\n");
			break;
		}
		recvBuff[recvCount] = '\0';
		if (strcmp(recvBuff, "q\n") == 0)
		{
			printf("IP地址：%s  ", inet_ntoa(clientAddrIn.sin_addr));
			printf("端口：%d ", htons(clientAddrIn.sin_port));
			printf("的客户端退出连接\n");
			break;
		}
		printf("Recv From Client %s Port %d: %s\n", inet_ntoa(clientAddrIn.sin_addr), htons(clientAddrIn.sin_port), recvBuff);
		strcpy(sendBuff, echo);
		strcat(sendBuff, recvBuff);
		send(clientSocket, sendBuff, strlen(sendBuff) + 1, 0);
	}
	closesocket(clientSocket);
	return 0;
}

int main()
{
	WSADATA wsadata;    // WSADATA对象，接收WSAStartup的返回数据
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) // 版本是2
	{
		WSACleanup();
		printf("WSAStartup调用出错\n");
		return WSAERROR;
	}
	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);	// 创建套接字
	if (serverSocket == INVALID_SOCKET)	// 创建失败
	{
		WSACleanup();
		printf("创建套接字错误: %d\n", WSAGetLastError());
		return SOCKERROR;
	}
	// 在IPv4中用sockaddr_in
	SOCKADDR_IN serverAddrIn;
	serverAddrIn.sin_family = AF_INET;
	serverAddrIn.sin_port = htons(9999);   // 网络字节序
	serverAddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))    // 转化一下
	{
		WSACleanup();
		printf("绑定错误: %d\n", WSAGetLastError());
		return BINDERROR;
	}

	printf("服务器启动... ...\n\n");

	// 监听
	if (listen(serverSocket, 5))
	{
		closesocket(serverSocket);
		WSACleanup();
		printf("监听错误: ");
		printf("%d\n", WSAGetLastError());
		return LISTENERROR;
	}

	SOCKET clientSocket;    // 接收客户端连接的套接字
	SOCKADDR_IN clientAddrIn;
	int addrInSize = sizeof(SOCKADDR_IN);

	while (true)	// 循环处理
	{
		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddrIn, &addrInSize);
		if (clientSocket == INVALID_SOCKET) // 连接失败
		{
			printf("连接客户端失败\n");
			printf("error: ");
			printf("%d\n", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return -1;
		}
		printf("接收到一个客户端连接：\n");
		printf("IP地址：%s  ", inet_ntoa(clientAddrIn.sin_addr));
		printf("端口：%d\n", htons(clientAddrIn.sin_port));

		Para* para = (Para*)malloc(sizeof(struct Para));
		para->clientSocket = clientSocket;
		para->clientAddrIn = clientAddrIn;
		HANDLE hThread;
		hThread = CreateThread(nullptr, 0, Echo, para, 0, NULL);
		CloseHandle(hThread);
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
