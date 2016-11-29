// NATServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <WS2tcpip.h>
#include <stdio.h>
#include <list>

#include "pthread.h"
#include "P2PClient.h"
using namespace std;


#define MAIN_PORT 3000












void * requestHandleThreadProc(void *)
{
	static int requestID = 0;

	CP2PRequest * requst = NULL;
	CP2PClient* hostClient = NULL;
	CP2PClient* peerClient = NULL;
	while (true)
	{

		if (0 < getRequstList()->size())
		{
			requestID++;
			requst = getRequstList()->front();
			getRequstList()->pop_front();
			requst->requestID = requestID;
			hostClient = NULL;
			peerClient = NULL;
			std::list<CP2PClient*>::iterator it;
			for (it=getClientList()->begin();it!= getClientList()->end();it++)
			{
				if ((*it)->mClientID == requst->peerID)
				{
					peerClient = *it;

				}
				if ((*it)->mClientID == requst->hostID)
				{
					hostClient = *it;

				}
			}
			if (NULL== hostClient || NULL== peerClient)
			{
				printf("offline \n;");
			}
			else
			{
				peerClient->CP2PClientSendRequest(hostClient->mClientID, hostClient->mClientIP, hostClient->mClientPort, requestID);
			}
			while (0 == peerClient->mStatus)
			{
				Sleep(100);
			}
			int ret = peerClient->CP2PClientRecvReady();
			if (requestID != ret)
			{
				printf("request id error!!!!!!!!!!\n");
				printf("request id error!!!!!!!!!!\n");
				printf("request id error!!!!!!!!!!\n");
			}
			hostClient->CP2PClientSendReady(peerClient->mClientIP,peerClient->mClientPort, requestID);
			
			
		}
		
		Sleep(1);
	}
	return NULL;
}
void* extendPortThreadProc(void * mParam)
{
	char ip[128] = {0};
	SOCKET socketEX;
	SOCKET socketCommunication;
	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons((u_short)(MAIN_PORT+1));
	socketEX = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == bind(socketEX, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR)))
	{
		printf("bind faild");
		closesocket(socketEX);
		return NULL;
	}

	if (SOCKET_ERROR == listen(socketEX, 100))
	{
		printf("listen faild");
		closesocket(socketEX);
		return NULL;
	}
	SOCKADDR_IN addrPeer;
	int len = sizeof(SOCKADDR);
	while (1)
	{
		
		if (INVALID_SOCKET == (socketCommunication = accept(socketEX, (SOCKADDR *)&addrPeer, &len)))
		{
			printf("accept failderr:%d\n", WSAGetLastError());
			continue;
		}
		printf("new ex connect!\n");
		char  recvBuf[128];
		int needRecv = 4;
		int realRecv = 0;
		int recvCnt = 0;
		while (needRecv > realRecv)
		{
			recvCnt = recv(socketCommunication, recvBuf + realRecv, needRecv - realRecv, 0);
			if (0 > recvCnt)
			{
				printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
				closesocket(socketCommunication);
				return NULL;
			}
			realRecv += recvCnt;
		}
		int *tmpID = (int*)recvBuf;
		int ID  = ntohl(*tmpID);
		

		std::list<CP2PClient*>::iterator it;
		for (it = getClientList()->begin(); it != getClientList()->end(); it++)
		{
			if ((*it)->mClientID == ID)
			{
				(*it)->mClientIP =  ntohl(addrPeer.sin_addr.S_un.S_addr) ;
				(*it)->mClientPort = ntohs(addrPeer.sin_port);
				inet_ntop(AF_INET, (void*)&addrPeer.sin_addr, ip,128);
				printf("ID:%d is update ip:%s port:%d\n", ID, ip, (*it)->mClientPort);
			}

		}
		closesocket(socketCommunication);


	}
	return NULL;
}

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

	SOCKET socketMain;
	SOCKET *socketCommunication = NULL;
	SOCKADDR_IN addrSrv; 
	addrSrv.sin_addr.S_un.S_addr = INADDR_ANY;
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons((u_short)MAIN_PORT);
	socketMain = socket(AF_INET, SOCK_STREAM, 0);
	if (SOCKET_ERROR == bind(socketMain, (SOCKADDR *)&addrSrv, sizeof(SOCKADDR)))
	{
		printf("bind faild");
		closesocket(socketMain);
		return -1;
	}

	if (SOCKET_ERROR == listen(socketMain, 100))
	{
		printf("listen faild");
		closesocket(socketMain);
		return -1;
	}


	pthread_t requestHandleThreadID;
	int ret = pthread_create(&requestHandleThreadID, NULL, requestHandleThreadProc, NULL);
	if (ret != 0)
	{
		printf("requestHandleThreadProc thread creat fail\n");
		return -1;
	}
	pthread_detach(requestHandleThreadID);

	pthread_t extendPortThreadID;
	 ret = pthread_create(&extendPortThreadID, NULL, extendPortThreadProc, NULL);
	if (ret != 0)
	{
		printf("extendPortThreadProc thread creat fail\n");
		return -1;
	}
	pthread_detach(extendPortThreadID);
	
	CP2PClient* client = NULL;
	while (1)
	{
		socketCommunication = new SOCKET;
		if (INVALID_SOCKET == (*socketCommunication = accept(socketMain, (SOCKADDR *)NULL, NULL)))
		{
			printf("accept failderr:%d\n",WSAGetLastError());
			delete socketCommunication;
			continue;
		}
		printf("new connect!\n");
		client = new CP2PClient(socketCommunication);
		if (0 == client->CP2PClientRecvRegistInfo())//最好做成异步
		{
			getClientList()->push_back(client);
			client->CP2PClientInitCommunicationThread();
		}
		else
		{			
			delete client;
			client = NULL;
			printf("regist failed!\n");
		}

		
	}
	

	

	system("PAUSE");
    return 0;
}

