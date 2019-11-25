
#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#pragma comment(lib,"ws2_32.lib")


#define WSAERROR -1		// WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define RECVERROR -4	// 接收消息错误
#define LISTENERROR -5	// 监听错误
#define BUFFSIZE 1024	// 缓冲区大小

//定长数据
int recvn(SOCKET s, char *recvbuf, unsigned int fixedlen)
{
	int iResult;
	int cnt;
	cnt = fixedlen;
	while (cnt > 0)
	{
		iResult = recv(s, recvbuf, cnt, 0);
		if (iResult > 0)
		{
			recvbuf += iResult;	// 指针后移
			cnt -= iResult;
		}
		else if (iResult == 0)
		{
			printf("连接关闭\n");
			return fixedlen - cnt;
		}
		else
		{
			printf("接收错误\n");
			return RECVERROR;
		}

	}
	return fixedlen;
}

//变长数据接收
int recvvl(SOCKET s, char *recvbuf, unsigned int recvbuflen)
{
	int iResult;//存储单次recv操作的返回值
	unsigned int reclen; //用于存储消息首部存储的长度信息
	iResult = recvn(s, (char *)&reclen, sizeof(unsigned int));
	if (iResult != sizeof(unsigned int))
	{
		if (iResult == -1)
		{
			printf("接收错误");
			return RECVERROR;
		}
		else
		{
			printf("连接关闭");
			return 0;
		}
	}
	reclen = ntohl(reclen);
	printf("\n该消息长度：%d\n", reclen);
	if (reclen > recvbuflen)
	{
		while (reclen > 0)
		{
			iResult = recvn(s, recvbuf, recvbuflen);
			if (iResult != recvbuflen)
			{
				if (iResult == -1)
				{
					printf("接收发生错误:%d\n", WSAGetLastError());
					return RECVERROR;
				}
				if (iResult == 0)
				{
					// printf("连接关闭\n");
					return 0;
				}
			}
			reclen -= recvbuflen;
			if (reclen < recvbuflen)
				recvbuflen = reclen;
		}
		printf("可变长度的消息超出预分配的接收缓存\r\n");
		return -1;
	}
	//接收可变长消息
	iResult = recvn(s, recvbuf, reclen);
	if (iResult != reclen)
	{
		if (iResult == -1)
		{
			printf("接收发生错误:%d\n", WSAGetLastError());
			return -1;
		}
		if (iResult == 0)
		{
			printf("连接关闭\n");
			return 0;
		}
	}
	return iResult;
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
	int recvCount = -1;
	char echo[] = "echo: ";
	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区

	while (1)
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

		while (1)
		{
			memset(recvBuff, 0, BUFFSIZE);
			recvCount = recvvl(clientSocket, recvBuff, BUFFSIZE);
			if (recvCount == -1)
				return RECVERROR;
			if (recvCount == 0)
				break;

			if (strcmp(recvBuff, "q") == 0)
			{
				printf("客户端退出连接\n");
				break;
			}
				
			printf("Received from client:%s\n", recvBuff);
			strcpy(sendBuff, echo);
			strcat(sendBuff, recvBuff);
			unsigned int len = strlen(sendBuff) + 1;
			len = htonl(len);
			send(clientSocket, (const char*)&len, sizeof(unsigned int), 0);	// 先发送长度
			send(clientSocket, sendBuff, strlen(sendBuff) + 1, 0);
		}
	}
	
	closesocket(serverSocket);
	WSACleanup();
	return 0;
}
