// TCP回射客户端

#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define WSAERROR -1		//WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define RECVERROR -4	// 接收消息出错
#define BUFFSIZE 256	//缓冲区大小


int main()
{
	WSADATA wsadata;    // WSADATA对象，接收WSAStartup的返回数据
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) // 版本是2
	{
		WSACleanup();
		printf("WSAStartup调用错误\n");
		return WSAERROR;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (clientSocket == INVALID_SOCKET)
	{
		WSACleanup();
		printf("创建客户端套接字错误: %d\n", WSAGetLastError());
		return SOCKERROR;
	}

	// 在IPv4中用sockaddr_in
	SOCKADDR_IN clientAddrIn;
	clientAddrIn.sin_family = AF_INET;
	clientAddrIn.sin_port = htons(9999);   // 网络字节序
	clientAddrIn.sin_addr.S_un.S_addr = inet_addr("10.236.78.15");    //服务器地址

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

	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区
	int addrInSize = sizeof(clientAddrIn);
	int recvCount = -1;	// recvfrom函数的返回值

	while (1)
	{
		printf("请输入消息：");
		scanf("%s", sendBuff);
		send(clientSocket, sendBuff, strlen(sendBuff) + 1, 0);
		if (strcmp(sendBuff, "q") == 0)
			break;
		recvCount = recv(clientSocket, recvBuff, BUFFSIZE-1, 0);
		if (recvCount > 0)
		{
			recvBuff[recvCount] = '\0';
			printf("%s\n", recvBuff);
		}
		else if(recvCount < 0)
		{
			printf("接收消息出错：%d\n", WSAGetLastError());
			return RECVERROR;
		}
		else
		{
			printf("连接关闭\n");
			break;
		}
	}

	closesocket(clientSocket);
	WSACleanup();

	return 0;
}
