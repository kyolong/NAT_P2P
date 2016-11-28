#include "stdafx.h"
#include <stdio.h>
#include <Winsock2.h>
#include "P2PClient.h"
#include "pthread.h"
#define NEED_RECV_1 4
#define NEED_RECV_2 8
#define  REQUEST 1


list<CP2PRequest*> requestList;
list<CP2PClient*> clientList;//应该要加锁

list<CP2PRequest*>* getRequstList()
{
	return &requestList;
}

list<CP2PClient*>* getClientList()
{
	return &clientList;
}

CP2PClient::CP2PClient(SOCKET * sock)
{
	mSocket = sock;
	mClientID = 0;
	mClientIP = 0;
	mClientPort = 0;
	mStatus = 0;

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
			printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
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
	char sentBuff[128];
	int * tmp = NULL;
	tmp = (int*)sentBuff;
	*tmp = htonl(host);
	tmp = (int*)(sentBuff+4);
	*tmp = htonl(ip);
	tmp = (int*)(sentBuff + 8);
	*tmp = htonl(port);
	tmp = (int*)(sentBuff + 12);
	*tmp = htonl(id);
	send(*mSocket,sentBuff,16,0);
	return 0;
}

int CP2PClient::CP2PClientRecvReady()
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
			printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
			closesocket(*mSocket);
			return -1;
		}
		realRecv += recvCnt;
	}
	int *tmpID = (int*)recvBuf;
	int requestID = ntohl(*tmpID);
	printf("ID:%d is ready,requestID is %d\n", mClientID, requestID);
	return requestID;
}

int CP2PClient::CP2PClientSendReady(int ip, int port, int id)
{
	char sentBuff[128];
	int * tmp = NULL;
	tmp = (int*)(sentBuff);
	*tmp = htonl(ip);
	tmp = (int*)(sentBuff + 4);
	*tmp = htonl(port);
	tmp = (int*)(sentBuff + 8);
	*tmp = htonl(id);
	send(*mSocket, sentBuff, 12, 0);
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
			printf("recv failed %s,%d\n", __FUNCTION__, __LINE__);
			closesocket(*(thisP2PClient->mSocket));
			return NULL;
		}
		realRecv += recvCnt;
	}


	int *tmpCmd = (int*)recvBuf;
	int *tmpVal = (int*)(recvBuf + 4);
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


	thisP2PClient->mStatus = 1;
	printf("Exit %s \n", __FUNCTION__);
	return NULL;
}

CP2PRequest::CP2PRequest(int host, int peer)
{
	hostID = host;
	peerID = peer;
	requestID = 0;

}
