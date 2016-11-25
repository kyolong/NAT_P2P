// NATServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <Winsock2.h>
#include <stdio.h>
#include <list>

#include "pthread.h"
using namespace std;


#define MAIN_PORT 33000
#define NEED_RECV_1 4;
#define NEED_RECV_2 8;

#define  REQUEST 1

class CP2PRequest
{
	public:
		int hostID;
		int peerID;
		int requestID;
		CP2PRequest(int host,int peer)
		{
			hostID = host;
			peerID = peer;
			requestID = 0;

		}
		~CP2PRequest()
		{

		}
};


list<CP2PRequest*> requestList;

class CP2PClient
{
public:
	
	SOCKET * mSocket;
	pthread_t mCommunicationThreadProcID;
	pthread_mutex_t mMutex;
public:
	int  CP2PClientRecvRegistInfo();

	int  CP2PClientSendRequest(int host,int ip,int port,int id);

	int  CP2PClientInitCommunicationThread();

	static void * CP2PClientCommunicationThreadProc(void* pParam);

	CP2PClient(SOCKET * sock);
	~CP2PClient();

public:
	int mClientID;
	int mClientIP;
	int mClientPort;
};

list<CP2PClient*> clientList;//应该要加锁

CP2PClient::CP2PClient(SOCKET * sock)
{
	mSocket = sock;
	mClientID = 0;
	mClientIP = 0;
	mClientPort = 0;
	
}



CP2PClient::~CP2PClient()
{
	delete mSocket;
}


int CP2PClient::CP2PClientRecvRegistInfo()
{
	char  recvBuf[128];
	int needRecv = NEED_RECV_1;
	int realRecv = 0;
	int recvCnt = 0;
	while (needRecv > realRecv)
	{
		recvCnt = recv(*mSocket, recvBuf + realRecv, needRecv - realRecv, 0);
		if (0 > recvCnt)
		{
			printf("recv failed %s,%s\n", __FUNCTION__, __LINE__);
			closesocket(*mSocket);
			return -1;
		}
		realRecv += recvCnt;
	}
	int *tmpID = (int*)recvBuf;
	mClientID = ntohl(*tmpID);
	printf("ID:%d is registed\n", mClientID);
	return 0;
}

int CP2PClient::CP2PClientSendRequest(int host, int ip, int port, int id)
{

	return 0;
}

int  CP2PClient::CP2PClientInitCommunicationThread()
{
	int ret = pthread_create(&mCommunicationThreadProcID, NULL, CP2PClientCommunicationThreadProc, this);
	if (ret != 0)
	{
		printf("CP2PClientInitCommunicationThreadProc thread creat fail\n");
		return -1;
	}
	pthread_detach(mCommunicationThreadProcID);
	return 0;
}

void * CP2PClient::CP2PClientCommunicationThreadProc(void* pParam)
{
	printf("Enter %s \n", __FUNCTION__);
		
	CP2PClient *thisP2PClient = (CP2PClient*)pParam;

	char  recvBuf[128];
	int needRecv = NEED_RECV_2;
	int realRecv = 0;
	int recvCnt = 0;
	while (needRecv > realRecv)
	{
		recvCnt = recv(*(thisP2PClient->mSocket), recvBuf + realRecv, needRecv - realRecv, 0);
		if (0 > recvCnt)
		{
			printf("recv failed %s,%s\n", __FUNCTION__, __LINE__);
			closesocket(*(thisP2PClient->mSocket));
			return NULL;
		}
		realRecv += recvCnt;
	}
	
	
	int *tmpCmd = (int*)recvBuf;
	int *tmpVal= (int*)(recvBuf+4);
	int Cmd = ntohl(*tmpCmd);
	int Val = ntohl(*tmpVal);
	CP2PRequest * requst = NULL;
	printf("[ID:%d] cmd is :%d,val is : %d\n", thisP2PClient->mClientID, Cmd, Val);
	switch (Cmd)
	{
	case REQUEST:
		requst = new CP2PRequest(thisP2PClient->mClientID, Val);
		requestList.push_back(requst);
		break;
	default:
		break;
	}



	printf("Exit %s \n", __FUNCTION__);
	return NULL;
}

void * requestHandleThreadProc(void *)
{
	static int requestID = 0;

	CP2PRequest * requst = NULL;
	while (true)
	{

		if (0 < requestList.size())
		{
			requestID++;
			requst = requestList.front();
			requestList.pop_front();
			requst->requestID = requestID;
			std::list<CP2PClient*>::iterator it;
			for (it=clientList.begin();it!=clientList.end();it++)
			{
				if ((*it)->mClientID == requst->peerID)
				{
					
				}
				
			}

		}
		
		Sleep(1);
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
	
	CP2PClient* client = NULL;
	while (1)
	{
		socketCommunication = new SOCKET;
		if (INVALID_SOCKET == (*socketCommunication = accept(socketMain, (SOCKADDR *)NULL, NULL)))
		{
			printf("listen failderr:%d\n",WSAGetLastError());
			delete socketCommunication;
			continue;
		}
		printf("new connect!\n");
		client = new CP2PClient(socketCommunication);
		if (0 == client->CP2PClientRecvRegistInfo())
		{
			clientList.push_back(client);
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

