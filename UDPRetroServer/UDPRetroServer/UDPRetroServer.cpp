// UDP循环回射服务器

#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define WSAERROR -1	//WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define BUFFSIZE 256	//缓冲区大小

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
	int addrInSize = sizeof(SOCKADDR_IN);
	int recvCount = -1;	// recvfrom函数的返回值
	while (true)	// 循环处理
	{
		recvCount = recvfrom(serverSocket, recvBuff, BUFFSIZE - 1, 0, (SOCKADDR*)&clientAddrIn, &addrInSize);
		if (recvCount >= 0)	// 返回值大于等于0，接收到数据
		{
			if (strcmp(recvBuff, "q") == 0)	// 用户输入"q"后退出
				break;

			recvBuff[recvCount] = '\0';
			printf("接收到一个连接：%s \n", inet_ntoa(clientAddrIn.sin_addr));
			printf("消息：%s\n", recvBuff);

			// 回射
			strcpy(sendBuff, echo);
			strncat(sendBuff, recvBuff, recvCount);
			printf("Retro：%s\n", sendBuff);

			sendto(serverSocket, sendBuff, strlen(sendBuff) + 1, 0, (SOCKADDR*)&clientAddrIn, addrInSize);
		}
		else
		{
			printf("接收消息出错：%d\n", WSAGetLastError());
			continue;
		}
	}

	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
