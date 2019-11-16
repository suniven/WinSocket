//
// Created by Chaos on 2019/10/19.
//

#include "pch.h"
#include <winsock2.h>
#include <stdio.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>

#pragma comment(lib,"ws2_32.lib")

#define CLIENT_MSG_SIZE 64    // 缓冲区长度

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
	char messageSend[CLIENT_MSG_SIZE];  // 发送给客户端的数据

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

	//发送当前时间
	time_t curTime;
	time(&curTime);
	strcpy(messageSend, ctime(&curTime));
	printf("send time: %s", messageSend);
	send(clientSocket, messageSend, strlen(messageSend) + 1, 0);  //这里必须是str+1否则客户端recv参数返回就错误
	memset(messageSend, 0, CLIENT_MSG_SIZE);

	closesocket(clientSocket);
	closesocket(serverSocket);
	printf("连接关闭\n");
	WSACleanup();

	getchar();

	return 0;
}