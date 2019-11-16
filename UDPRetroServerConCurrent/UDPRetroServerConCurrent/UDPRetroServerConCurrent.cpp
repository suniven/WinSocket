
// UDP并发回射服务器

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
#define BUFFSIZE 256	// 缓冲区大小

static int ServerPort = 9999;	// 创建新线程时创建一个新的套接字，端口在此基础上++

struct ParaPack {
	SOCKADDR_IN clientAddrIn;
	char recvBuff[BUFFSIZE];	// 首次接收到的客户端发送的非'q'信息
};

DWORD WINAPI Echo(LPVOID pParam)
{
	printf("A Thread Created...\n");
	char echo[] = "echo: ";
	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区

	// 创建新的套接字绑定一个新的临时端口
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// 创建套接字
	if (serverSocket == INVALID_SOCKET)	// 创建失败
	{
		WSACleanup();
		printf("创建套接字错误: %d\n", WSAGetLastError());
		return SOCKERROR;
	}
	// 在IPv4中用sockaddr_in
	SOCKADDR_IN serverAddrIn;
	serverAddrIn.sin_family = AF_INET;
	serverAddrIn.sin_port = htons(++ServerPort);   // 一个新的临时端口
	serverAddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))    // 转化一下
	{
		WSACleanup();
		printf("绑定错误: %d\n", WSAGetLastError());
		return BINDERROR;
	}

	ParaPack* para=(ParaPack*)pParam;
	SOCKADDR_IN clientAddrIn = para->clientAddrIn;
	strcpy(recvBuff, para->recvBuff);
	int addrInSize = sizeof(SOCKADDR_IN);
	int recvCount = -1;	// recvfrom函数的返回值

	// 首次回射
	printf("接收到一个连接：%s \n", inet_ntoa(clientAddrIn.sin_addr));
	printf("端口：%d\n", htons(clientAddrIn.sin_port));
	printf("消息：%s\n", recvBuff);
	strcpy(sendBuff, echo);
	strncat(sendBuff, recvBuff, recvCount);
	printf("Retro：%s\n\n", sendBuff);
	sendto(serverSocket, sendBuff, strlen(sendBuff) + 1, 0, (SOCKADDR*)&clientAddrIn, addrInSize);
	
	while (true)	// 循环处理
	{
		recvCount = recvfrom(serverSocket, recvBuff, BUFFSIZE - 1, 0, (SOCKADDR*)&clientAddrIn, &addrInSize);
		if (recvCount >= 0)	// 返回值大于等于0，接收到数据
		{
			if (strcmp(recvBuff, "q") == 0)	// 用户输入"q"后退出
			{
				printf("连接 %s 退出 \n\n", inet_ntoa(clientAddrIn.sin_addr));
				break;
			}
				

			recvBuff[recvCount] = '\0';
			printf("接收到一个连接：%s \n", inet_ntoa(clientAddrIn.sin_addr));
			printf("端口：%d\n", htons(clientAddrIn.sin_port));
			printf("消息：%s\n", recvBuff);

			// 回射
			strcpy(sendBuff, echo);
			strncat(sendBuff, recvBuff, recvCount);
			printf("Retro：%s\n\n", sendBuff);

			sendto(serverSocket, sendBuff, strlen(sendBuff) + 1, 0, (SOCKADDR*)&clientAddrIn, addrInSize);
		}
		else
		{
			printf("接收消息出错：%d\n", WSAGetLastError());
			continue;
		}
	}
	closesocket(serverSocket);
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
	SOCKET serverSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);	// 创建套接字
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

	char echo[] = "echo: ";
	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区
	SOCKADDR_IN clientAddrIn;
	SOCKADDR_IN tmp;	// 创建新线程时用
	// HANDLE hThread;
	int addrInSize = sizeof(SOCKADDR_IN);
	int recvCount = -1;	// recvfrom函数的返回值

	while (true)	// 循环处理
	{
		recvCount = recvfrom(serverSocket, recvBuff, BUFFSIZE - 1, 0, (SOCKADDR*)&clientAddrIn, &addrInSize);
		if (recvCount < 0)
		{
			printf("接收消息出错：%d\n", WSAGetLastError());
			return RECVERROR;
		}
		if (strcmp(recvBuff, "q") == 0)	// 用户输入"q"后退出，不需要创建新线程为其提供后续服务
		{
			recvBuff[recvCount] = '\0';
			printf("接收到一个连接：%s \n", inet_ntoa(clientAddrIn.sin_addr));
			printf("端口：%d\n", htons(clientAddrIn.sin_port));
			printf("消息：%s\n", recvBuff);
			printf("连接 %s 端口 %d 退出 \n", inet_ntoa(clientAddrIn.sin_addr),htons(clientAddrIn.sin_port));
			continue;
		}
			
		HANDLE hThread;
		ParaPack* para = (ParaPack*)malloc(sizeof(ParaPack));
		tmp = clientAddrIn;	// 
		para->clientAddrIn = tmp;
		recvBuff[recvCount] = '\0';
		strcpy(para->recvBuff, recvBuff);
		hThread = CreateThread(nullptr, 0, Echo, para, 0, NULL);
		CloseHandle(hThread);
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
