//
// Created by Chaos on 2019/10/19.
//

#include "pch.h"
#include <winsock2.h>
#include <stdio.h>
#include <time.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

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
	const int CLIENT_MSG_SIZE = 64;    // 缓冲区长度
	char messageRecv[CLIENT_MSG_SIZE];    // 来自于客户端的数据
	char messageSend[CLIENT_MSG_SIZE];  // 发送给客户端的数据

	while (1)   // 循环等待连接
	{
		printf("服务器已启动，等待客户端连接...\n");

		clientSocket = accept(serverSocket, (struct sockaddr *) &clientAddrIn, &addrInSize);
		if (clientSocket == INVALID_SOCKET) // 连接失败
		{
			printf("连接客户端失败\n");
			printf("error: ");
			printf("%d", WSAGetLastError());
			closesocket(serverSocket);
			WSACleanup();
			return -1;
		}
		printf("客户端");
		printf("%s", inet_ntoa(clientAddrIn.sin_addr));
		printf("连接成功\n");


		strcpy(messageSend, "请输入请求(获取本地时间或关闭连接)：");
		send(clientSocket, messageSend, strlen(messageSend) + 1, 0);

		// 服务器接收消息直到客户端退出
		while (1)
		{
			if (recv(clientSocket, messageRecv, CLIENT_MSG_SIZE, 0) == SOCKET_ERROR)    // 接收出错
			{
				printf("连接错误，消息接收失败\n");
				printf("error: ");
				printf("%d\n", WSAGetLastError());
				closesocket(clientSocket);
				break;
			}

			printf("Message Received From Client: \n");
			printf("%s\n", messageRecv);

			// 请求当前时间
			if (strcmp(messageRecv, "获取本地时间") == 0)
			{
				time_t curTime;
				time(&curTime);
				strcpy(messageSend, ctime(&curTime));
				printf("send time: %s", messageSend);
				send(clientSocket, messageSend, strlen(messageSend) + 1, 0);  //这里必须是str+1否则客户端recv参数返回就错误
				memset(messageSend, 0, CLIENT_MSG_SIZE);
			}
			else if (strcmp(messageRecv, "关闭连接") == 0)
			{
				shutdown(clientSocket, SD_BOTH);
				break;
			}
			else
			{
				strcpy(messageSend, "请输入请求(获取本地时间或关闭连接)：");
				send(clientSocket, messageSend, strlen(messageSend) + 1, 0);
			}
		}
		closesocket(clientSocket);
	}
	closesocket(serverSocket);
	WSACleanup();

	return 0;
}