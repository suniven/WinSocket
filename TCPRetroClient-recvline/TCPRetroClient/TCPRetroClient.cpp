//
// Created by Chaos on 2019/10/19.
//

#include "pch.h"
#include <winsock2.h>
#include <stdio.h>

#pragma comment(lib,"ws2_32.lib")
#define BUFFSIZE 256		//缓冲区大小

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
	clientAddrIn.sin_addr.S_un.S_addr = inet_addr("10.236.78.15");    //服务器地址

	char messageSend[BUFFSIZE];  // 客户端发送缓冲区
	char messageRecv[BUFFSIZE];  // 客户端接收缓冲区

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
	int recvCount = -1;
	while (1)
	{
		memset(messageSend, '\0', BUFFSIZE);
		printf("请输入消息：");
		fgets(messageSend, BUFFSIZE - 1, stdin);
		// 去\n
		messageSend[strlen(messageSend)-1] = '\0';
		if (strcmp(messageSend, "q") == 0)
			break;
		send(clientSocket, messageSend, strlen(messageSend)+1, 0);
		//循环接收
		while (1)
		{
			recvCount = recv(clientSocket, messageRecv, BUFFSIZE, 0);
			if (recvCount == 0)
			{
				printf("连接关闭\n");
				return 0;
			}
			else if (recvCount < 0)
			{
				printf("消息接收错误");
				return -1;
			}
			else
			{
				messageRecv[recvCount] = '\0';
				fputs(messageRecv, stdout);
				printf("\n");
				break;
			}
		}
	}
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}