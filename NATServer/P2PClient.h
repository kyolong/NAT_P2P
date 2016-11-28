#pragma once
#include <list>
#include "pthread.h"
using namespace std;

class CP2PClient
{
public:

	SOCKET * mSocket;
	pthread_t mCommunicationThreadProcID;
	pthread_mutex_t mMutex;
public:
	int  CP2PClientRecvRegistInfo();

	int  CP2PClientSendRequest(int host, int ip, int port, int id);

	int  CP2PClientRecvReady();

	int  CP2PClientSendReady(int ip, int port, int id);

	int  CP2PClientInitCommunicationThread();

	static void * CP2PClientCommunicationThreadProc(void* pParam);

	CP2PClient(SOCKET * sock);
	~CP2PClient();

public:
	int mClientID;
	int mClientIP;
	int mClientPort;
	int mStatus;
};

class CP2PRequest
{
public:
	int hostID;
	int peerID;
	int requestID;
	CP2PRequest(int host, int peer);


};

list<CP2PRequest*>* getRequstList();

list<CP2PClient*>* getClientList();