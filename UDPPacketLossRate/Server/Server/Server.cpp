// UDP循环回射服务器

#include "pch.h"
#include <stdio.h>
#include <winsock2.h>
#include <string.h>
#include <time.h>

#pragma comment(lib,"ws2_32.lib")

#define WSAERROR -1		// WSADATA对象错误
#define SOCKERROR -2	// 创建套接字错误
#define BINDERROR -3	// 绑定IP和端口错误
#define BUFFSIZE 256	// 缓冲区大小

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
	int index = 0;	// 报文序号
	int count = 0;	// 服务器接收到的报文数
	int sleepInterval = 0;	//控制接收速率
	double lossRate = 0;

	printf("请输入接收两条报文之间的时间间隔(ms)：");
	scanf("%d", &sleepInterval);

	while (true)	// 循环处理
	{
		recvCount = recvfrom(serverSocket, recvBuff, BUFFSIZE, 0, (SOCKADDR*)&clientAddrIn, &addrInSize);
		index++;
		if (recvCount >= 0)	// 返回值大于0，接收到数据
		{
			printf("第%d次接收，接收字符个数%d", index, recvCount);
			count++;
		}
		else
		{
			printf("接收消息出错：%d\n", WSAGetLastError());
			printf("出错报文序号: %d\n", index);
			continue;
		}
		Sleep(sleepInterval);
		lossRate = 1 - (double)count / index;
		printf("当前接收%d次，%d次接收成功\n", index, count);
		printf("(报文已到达服务器缓冲区的情况下)丢包率%lf\n\n", lossRate);
	}

	closesocket(serverSocket);
	WSACleanup();
	getchar();
	return 0;
}
