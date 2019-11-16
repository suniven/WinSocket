//
// Created by Chaos on 2019/10/19.
// 以recvvl方式接收数据
//

#include "pch.h"
#include <winsock2.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define BUFFSIZE 256    // 缓冲区长度

int main()
{
	WSADATA wsadata;    // WSADATA对象，接收WSAStartup的返回数据
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) // 版本是2
	{
		WSACleanup();
		printf("WSAStartup调用出错\n");
		return -1;
	}

	SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (serverSocket == INVALID_SOCKET)
	{
		WSACleanup();
		printf("create socket error: ");
		printf("%d\n", WSAGetLastError());
		return -1;
	}

	// 在IPv4中用sockaddr_in
	SOCKADDR_IN serverAddrIn;
	serverAddrIn.sin_family = AF_INET;
	serverAddrIn.sin_port = htons(9999);   // 网络字节序
	serverAddrIn.sin_addr.S_un.S_addr = htonl(INADDR_ANY);

	if (bind(serverSocket, (struct sockaddr*)&serverAddrIn, sizeof(serverAddrIn)))    // 转化一下
	{
		WSACleanup();
		printf("bind error: ");
		printf("%d\n", WSAGetLastError());
		return -1;
	}

	printf("服务器启动，等待客户端连接...\n\n");

	// 监听
	if (listen(serverSocket, 3))
	{
		closesocket(serverSocket);
		WSACleanup();
		printf("socket error: ");
		printf("%d\n", WSAGetLastError());
		return -1;
	}

	SOCKET clientSocket;    // 接收客户端连接的套接字
	SOCKADDR_IN clientAddrIn;
	int addrInSize = sizeof(SOCKADDR_IN);
	int recvCount = -1;
	int RECVBUFFSIZE = 0;

	//指定接收长度
	printf("请输入每次接收字符串的长度：");
	scanf("%d", &RECVBUFFSIZE);

	char echo[] = "echo:";
	char* messageRecvPer = (char*)malloc(sizeof(char)*(RECVBUFFSIZE + 1));   // 每次接收的的数据
	char* messageRecv = (char*)malloc(sizeof(char)*BUFFSIZE);	// 来自于客户端的数据 
	char* messageSend = (char*)malloc(sizeof(char)*BUFFSIZE);	// 发送到客户端的数据 

	while (1)   // 循环等待连接
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

		// 服务器接收消息直到客户端退出
		while (1)
		{
			memset(messageRecv, '\0', BUFFSIZE);
			while (1)
			{
				recvCount = recv(clientSocket, messageRecvPer, RECVBUFFSIZE, 0);
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
				if (messageRecvPer[recvCount - 1] == '\0')
				{
					strcat(messageRecv, messageRecvPer);
					break;
				}
				messageRecvPer[recvCount] = '\0';
				if (strcmp(messageRecvPer, "q") == 0)
				{
					printf("客户端退出连接\n");
					break;
				}
				strcat(messageRecv, messageRecvPer);
			}
			if (recvCount <= 0)
				break;
			printf("Recv From Client: %s\n", messageRecv);
			strcpy(messageSend, echo);
			strcat(messageSend, messageRecv);
			send(clientSocket, messageSend, strlen(messageSend) + 1, 0);
		}
		closesocket(clientSocket);
	}
	closesocket(serverSocket);
	WSACleanup();

	return 0;
}