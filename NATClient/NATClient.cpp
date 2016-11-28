// NATClient.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#define  MAIN_PORT 33000
#define SERVER_IP "127.0.0.1"
int main()
{
	/////////////初始化套接字///////////////////////////////////////////////////
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);//版本号1.1
									   //1.加载套接字库	
	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0)
	{
		return -1;
	}///...if
	 //判断是否我们请求的winsocket版本，如果不是
	 //则调用WSACleanup终止winsocket的使用并返回
	if (LOBYTE(wsaData.wVersion) != 1 || HIBYTE(wsaData.wVersion) != 1)
	{
		WSACleanup();
		return -1;
	}///...if
	 ////////////////////////////////////////////////////////////

	int ID = 0;
	char serverip[128] = {'\0'};
	printf("input ID\n");
	scanf_s("%d", &ID);
	getchar();
	printf("input ip\n");
	scanf_s("%s", serverip,16);
	getchar();
	SOCKET mainSock = socket(AF_INET, SOCK_STREAM, 0);
	SOCKADDR_IN mainAddr;
	inet_pton(AF_INET, serverip, (void*)&(mainAddr.sin_addr));
	mainAddr.sin_family = AF_INET;
	mainAddr.sin_port = htons((u_short)MAIN_PORT);
	connect(mainSock, (struct sockaddr*) &mainAddr,sizeof(mainAddr));
	char  sendbuff[128] = {0};
	int * tmp = (int*)sendbuff;
	*tmp = htonl(ID);
	send(mainSock, sendbuff, 4, 0);

	char mode = 0;
	while (true)
	{
		printf("input mode. s for server, c for client\n");
		scanf_s("%c", &mode,1);
		getchar();
		if ('c' == mode || 's'== mode)
		{
			break;
		}
		
	}
	int flag = 1;
	int len = sizeof(int);
	SOCKET exSock = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(exSock, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, len);
	SOCKADDR_IN exAddr;
	inet_pton(AF_INET, serverip, (void*)&(exAddr.sin_addr));
	exAddr.sin_family = AF_INET;
	exAddr.sin_port = htons((u_short)(MAIN_PORT + 1));

	SOCKADDR_IN addrLocal;
	addrLocal.sin_addr.S_un.S_addr = INADDR_ANY;
	//inet_pton(AF_INET, "192.168.16.104", (void*)&(addrLocal.sin_addr));
	addrLocal.sin_family = AF_INET;
	char  recvBuf[128];
	int needRecv = 16;
	int realRecv = 0;
	int recvCnt = 0;
	if ('c' == mode)
	{
		addrLocal.sin_port = htons((u_short)(MAIN_PORT + 2));

		if (SOCKET_ERROR == bind(exSock, (SOCKADDR *)&addrLocal, sizeof(SOCKADDR)))
		{
			printf("bind faild");
			closesocket(exSock);
			return -1;
		}
		
		connect(exSock, (struct sockaddr*) &exAddr, sizeof(exAddr));
		tmp = (int*)sendbuff;
		*tmp = htonl(ID);
		send(exSock, sendbuff, 4, 0);

		

		int peerID = 0;
		printf("input peer ID\n");
		scanf_s("%d", &peerID);
		getchar();

		tmp = (int*)sendbuff;
		*tmp = htonl(1);
		tmp = (int*)(sendbuff+4);
		*tmp = htonl(peerID);
		send(mainSock, sendbuff, 8, 0);

		needRecv = 12;
		realRecv = 0;
		recvCnt = 0;
		while (needRecv > realRecv)
		{
			recvCnt = recv(mainSock, recvBuf + realRecv, needRecv - realRecv, 0);
			if (0 > recvCnt)
			{
				printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
				closesocket(mainSock);
				return NULL;
			}
			realRecv += recvCnt;
		}
	
		int *tmpID = (int*)recvBuf;
		int peerIP = ntohl(*tmpID);
		 tmpID = (int*)(recvBuf + 4);
		int peerPort = ntohl(*tmpID);
		 tmpID = (int*)(recvBuf + 8);
		int requestID = ntohl(*tmpID);

		SOCKADDR_IN addrPeer;
		addrPeer.sin_addr.S_un.S_addr = htonl(peerIP);
		addrPeer.sin_family = AF_INET;
		addrPeer.sin_port = htons((u_short)peerPort);
		char tmpIP[128] = { '\0' };
		inet_ntop(AF_INET, (void*)&addrPeer.sin_addr, tmpIP, 128);
		printf("to connect,ip:%s,port:%d,requestID:%d\n", tmpIP, peerPort, requestID);
		
		SOCKET exSock_ex = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(exSock_ex, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, len);
		if (SOCKET_ERROR == bind(exSock_ex, (SOCKADDR *)&addrLocal, sizeof(SOCKADDR)))
		{
			printf("bind faild");
			closesocket(exSock_ex);
			return -1;
		}
		int cnt = 0;
		while (SOCKET_ERROR == connect(exSock_ex, (struct sockaddr*) &addrPeer, sizeof(addrPeer)))
		{
			cnt++;
			printf("reconnect %d\n", WSAGetLastError());
			Sleep(500);
			if (20 < cnt)
			{
				printf("failed to connect %d\n", WSAGetLastError());
				break;
			}
		}
		
		char keychar;
		while (true)
		{
			scanf_s("%c", &keychar,1);
			getchar();
			send(exSock_ex, &keychar, 1, 0);
			Sleep(10);
		}
		
		system("PAUSE");
	}

	if ('s' == mode)
	{
		addrLocal.sin_port = htons((u_short)(MAIN_PORT + 3));

		if (SOCKET_ERROR == bind(exSock, (SOCKADDR *)&addrLocal, sizeof(SOCKADDR)))
		{
			printf("bind faild");
			closesocket(exSock);
			return -1;
		}


		 needRecv = 16;
		 realRecv = 0;
		 recvCnt = 0;
		while (needRecv > realRecv)
		{
			recvCnt = recv(mainSock, recvBuf + realRecv, needRecv - realRecv, 0);
			if (0 > recvCnt)
			{
				printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
				closesocket(mainSock);
				return NULL;
			}
			realRecv += recvCnt;
		}
		int *tmpID = (int*)recvBuf;
		int peerID = ntohl(*tmpID);
		 tmpID = (int*)(recvBuf+4);
		int peerIP = ntohl(*tmpID);
		 tmpID = (int*)(recvBuf+8);
		int peerPort = ntohl(*tmpID);
		 tmpID = (int*)(recvBuf+12);
		int requestID = ntohl(*tmpID);

		SOCKADDR_IN addrPeer;
		addrPeer.sin_addr.S_un.S_addr = htonl(peerIP);
		addrPeer.sin_family = AF_INET;
		addrPeer.sin_port = htons((u_short)peerPort);
		char tmpIP[128] = { '\0' };
		inet_ntop(AF_INET, (void*)&addrPeer.sin_addr, tmpIP,128);
		printf("ID:%d request connect,ip:%s,port:%d,requestID:%d\n", peerID, tmpIP, peerPort, requestID);
		
		connect(exSock, (struct sockaddr*) &exAddr, sizeof(exAddr));
		tmp = (int*)sendbuff;
		*tmp = htonl(ID);
		send(exSock, sendbuff, 4, 0);
		//closesocket(exSock);
		SOCKET exSock_new = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(exSock_new, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, len);
		if (SOCKET_ERROR == bind(exSock_new, (SOCKADDR *)&addrLocal, sizeof(SOCKADDR)))
		{
			printf("bind faild\n");
			closesocket(exSock_new);
			return -1;
		}
		printf("connect peer,fuck nat\n");
		
		int cnt = 0;
		while (SOCKET_ERROR == connect(exSock_new, (struct sockaddr*) &addrPeer, sizeof(addrPeer)))
		{
			cnt++;
			printf("refuck %d\n", WSAGetLastError());
			Sleep(500);
			if (2 < cnt)
			{
				
				break;
			}
		}


		printf("end fuck nat\n");
		//closesocket(exSock);
		SOCKET exSock_ex = socket(AF_INET, SOCK_STREAM, 0);
		setsockopt(exSock_ex, SOL_SOCKET, SO_REUSEADDR, (char*)&flag, len);
		if (SOCKET_ERROR == bind(exSock_ex, (SOCKADDR *)&addrLocal, sizeof(SOCKADDR)))
		{
			printf("bind faild");
			closesocket(exSock_ex);
			return -1;
		}
		if (SOCKET_ERROR == listen(exSock_ex, 100))
		{
			printf("listen faild");
			closesocket(exSock_ex);
			return NULL;
		}
		tmp = (int*)(sendbuff+8);
		*tmp = htonl(requestID);
		int ret = send(mainSock, sendbuff, 12, 0);
		if (12 != ret)
		{
			printf("send failed\n");
		}
		SOCKET socketCommunication;
		int len = sizeof(SOCKADDR);
		


			if (INVALID_SOCKET == (socketCommunication = accept(exSock_ex, NULL, NULL)))
			{
				printf("accept failderr:%d\n", WSAGetLastError());

	
			}
			printf("success！！！！！！！！！！！！！！！！！！！！！\n");
			char pr;
			while (true)
			{
				recv(socketCommunication, &pr, 1, 0);
				printf("%c", pr);
				Sleep(10);
			}
	


	}
	
    return 0;
}

