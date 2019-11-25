
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
					printf("连接关闭\n");
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
	clientAddrIn.sin_addr.S_un.S_addr = inet_addr("10.241.192.130");    //服务器地址

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
		printf("请输入要发送的消息：");
		fgets(sendBuff, BUFFSIZE - 1, stdin);
		//吃回车
		sendBuff[strlen(sendBuff) - 1] = '\0';
		if (strcmp(sendBuff, "q") == 0)
		{
			printf("退出连接\n");
			break;
		}
		unsigned int len = strlen(sendBuff) + 1;
		len = htonl(len);
		send(clientSocket, (const char*)&len, sizeof(unsigned int), 0);	// 先发送长度
		send(clientSocket, sendBuff, strlen(sendBuff) + 1, 0);

		//接受数据
		recvCount = recvvl(clientSocket, recvBuff, BUFFSIZE);
		if (recvCount == -1)
			return RECVERROR;
		if (recvCount == 0)
			break;
		printf("%s\n", recvBuff);

	}
	
	closesocket(clientSocket);
	WSACleanup();
	return 0;
}
