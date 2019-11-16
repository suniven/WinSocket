//
// Created by Chaos on 2019/10/19.
//

#include "pch.h"
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

#define SERVER_MSG_SIZE 64

int main()
{
	WSADATA wsadata;    // WSADATA对象，接收WSAStartup的返回数据
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) // 版本是2
	{
		WSACleanup();
		printf("WSAStartup调用出错\n");
		return -1;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		WSACleanup();
		printf("create socket error: ");
		printf("%d\n", WSAGetLastError());
		return -1;
	}

	// 在IPv4中用sockaddr_in
	SOCKADDR_IN clientAddrIn;
	clientAddrIn.sin_family = AF_INET;
	clientAddrIn.sin_port = htons(9999);   // 网络字节序
	clientAddrIn.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");    //服务器地址

	char messageRecv[SERVER_MSG_SIZE];  // 客户端接收缓冲区

	// 连接服务器失败
	if (connect(clientSocket, (struct sockaddr*) &clientAddrIn, sizeof(clientAddrIn)))
	{
		printf("连接服务器失败\n");
		printf("error: ");
		printf("%d\n", WSAGetLastError());
		closesocket(clientSocket);
		WSACleanup();
		return -1;
	}

	printf("连接服务器成功\n");
	int size;
	//循环接收
	while (1)
	{
		size = recv(clientSocket, messageRecv, SERVER_MSG_SIZE-1, 0);
		if (size == 0)
		{
			printf("连接关闭\n");
			break;
		}
		else if (size < 0)
		{
			printf("error");
			break;
		}
		else
		{
			messageRecv[size] = '\0';
			printf("Message Size: %d\n", size);
			printf("Message Received From Server:\n%s\n", messageRecv);
		}
	}
	closesocket(clientSocket);
	WSACleanup();

	getchar();
	return 0;
}