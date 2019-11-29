// UDP回射客户端

#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>

#pragma comment(lib,"ws2_32.lib")

#define WSAERROR -1		//WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define RECVERROR -4	// 接收消息出错
#define BUFFSIZE 64	//缓冲区大小


int main()
{
	WSADATA wsadata;    // WSADATA对象，接收WSAStartup的返回数据
	if (WSAStartup(MAKEWORD(2, 2), &wsadata)) // 版本是2
	{
		WSACleanup();
		printf("WSAStartup调用错误\n");
		return WSAERROR;
	}

	SOCKET clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
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
	clientAddrIn.sin_addr.S_un.S_addr = inet_addr("10.241.192.130");    //服务器地址

	char recvBuff[BUFFSIZE];	// 接收缓冲区
	char sendBuff[BUFFSIZE];	// 发送缓冲区
	int count = 0;				// 发送的数据包个数
	int addrInSize = sizeof(clientAddrIn);
	int recvCount = -1;	// recvfrom函数的返回值

	memset(sendBuff, 'w', BUFFSIZE - 1);	// 发送的一个包
	sendBuff[BUFFSIZE - 1] = '\0';

	printf("请输入发送的数据报个数：");
	scanf("%d", &count);
	for (int i = 0; i < count; i++)
	{
		sendto(clientSocket, sendBuff, strlen(sendBuff) + 1, 0, (SOCKADDR*)&clientAddrIn, addrInSize);
	}
	
	closesocket(clientSocket);
	WSACleanup();

	return 0;
}
