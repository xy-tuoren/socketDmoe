#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#define FD_SETSIZE 1024
#include "CELLTimestamp.hpp"
//��������С
#ifndef RECV_BUFF_SZIE
#define RECV_BUFF_SZIE 10240
#endif // !RECV_BUFF_SZIE
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else // _WIN32
#include<unistd.h>
#include<arpa/inet.h>
#include<string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR           (-1)
#endif
#include <iostream>
#include <thread>
#include <vector>
#include<cstdio>
#include "MessageHeader.hpp"
using namespace std;

class ClientSocket
{
public:
	~ClientSocket();
	ClientSocket(SOCKET);
	SOCKET getSockFd() const;
	const char* getMsgBuf() const;
	void setMsgBuf(const char*,int);
	int getLastPos() const;
	void setLastPos(int);
private:
	//socket fd_set file desc set
	SOCKET sockfd;
	//�ڶ������� ��Ϣ������
	char szMsBuf[RECV_BUFF_SZIE * 10];
	//��Ϣ������������β��λ��
	int lastPos = 0;
};

ClientSocket::ClientSocket(SOCKET sockfd = INVALID_SOCKET):sockfd(sockfd)
{
	memset(szMsBuf, 0, sizeof(szMsBuf));
}

ClientSocket::~ClientSocket()
{
}

SOCKET ClientSocket::getSockFd() const
{
	return sockfd;
}

inline const char* ClientSocket::getMsgBuf() const
{
	return szMsBuf;
}

inline void ClientSocket::setMsgBuf(const char* ch,int bit)
{
	memcpy(szMsBuf, ch, bit);
}

inline int ClientSocket::getLastPos() const
{
	return lastPos;
}

inline void ClientSocket::setLastPos(int pos)
{
	lastPos = pos;
}

class EasyTcpServer
{
private:
	SOCKET _sock;
	std::vector<ClientSocket*> _clients;
	//������
	char szRecv[RECV_BUFF_SZIE] = {};
	CELLTimestamp _Time;
	int _recvCount;
public:
	EasyTcpServer();
	virtual ~EasyTcpServer();
	SOCKET getSocket() const;
	//��ʼ��socket
	// //��ʼ��socket
	SOCKET InitSocket()
	{
		if (INVALID_SOCKET != _sock) {
			cout << "socket:" << _sock << "�رվ�����!" << endl;
			Close();
		}
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//����һ����socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			cout << "socket=" << _sock << "����" << "����socketʧ��!" << endl;
		}
		else
		{
			cout << "socket=" << _sock << "����socket�ɹ�!" << endl;
		}
		return _sock;
	}
	//��IP�Ͷ˿ں�
	int Bind(const char* ip, unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //�˿�ת�����޷��Ŷ�����
#ifdef _WIN32
		if (ip) {
			_sin.sin_addr.S_un.S_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.S_un.S_addr = INADDR_ANY;
		}
#else
		if (ip) {
			_sin.sin_addr.s_addr = inet_addr(ip);
		}
		else
		{
			_sin.sin_addr.s_addr = INADDR_ANY;
		}
#endif
		int ret = bind(_sock, (sockaddr*)&_sin, sizeof(_sin)) == SOCKET_ERROR;
		if (SOCKET_ERROR == ret)
		{
			std::cout << "�����ڽ��ܿͻ������ӵ�����˿�:" << port << "ʧ��" << std::endl;
		}
		else
		{
			std::cout << "��" << port << "�˿ڳɹ�" << std::endl;
		}
		return ret;
	}
	//�����˿ں�
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			std::cout << "�������ڽ��ܿͻ������ӵ�����˿�ʧ��" << std::endl;
		}
		else
		{
			std::cout << "�����˿ڳɹ�" << std::endl;
		}
		return ret;
	}
	//���տͻ�������
	SOCKET Accept()
	{
		// accept�ȴ����ܿͻ�������
		sockaddr_in clientAddr = {};
		int nAddrLen = sizeof(sockaddr_in);
		SOCKET cSock = INVALID_SOCKET;
#ifdef _WIN32
		int* len = &nAddrLen;
#else
		socklen_t* len = (socklen_t*)&nAddrLen;
#endif // _WIN32
		cSock = accept(_sock, (sockaddr*)&clientAddr, len);
		if (cSock == INVALID_SOCKET) {
			std::cout << "socket=" << _sock << "���󣬽��ܵ���Ч�ͻ��˵�SOCKET" << std::endl;
		}
		else
		{
			//�¿ͻ��˼���㲥�����пͻ���
			//NewUserJoin userJoin;
			//userJoin.sock = cSock;
			//SendData2All(&userJoin);
			_clients.emplace_back(new ClientSocket(cSock));
			std::cout << "������socket=" << _sock << "�¿ͻ��˼���: socket=" << cSock << " IP = " << inet_ntoa(clientAddr.sin_addr) << "�ͻ�������:" << _clients.size() << std::endl;
		}
		return cSock;
	}
	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
#ifdef _WIN32	
				//�����׽���
				closesocket(_clients[i]->getSockFd());
				delete _clients[i];
#else
				//�����׽���
				close(_clients[i]->getSockFd());
				delete _clients[i];
#endif
			}
			_clients.clear();
#ifdef _WIN32
			//���Windows socket����
			WSACleanup();
			system("pause");
#endif
			cout << "���˳�!" << endl;
			_sock = INVALID_SOCKET;
		}
	}
	//����������Ϣ
	bool OnRun()
	{
		if (isRun())
		{
			//�������׽��� BSD socket
			//������(socket)����
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			//������
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//��������(socket)���뼯��
			FD_SET(_sock, &fdRead);
			FD_SET(_sock, &fdWrite);
			FD_SET(_sock, &fdExp);
			SOCKET maxSock = _sock;
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				FD_SET(_clients[i]->getSockFd(), &fdRead);
				if (maxSock < _clients[i]->getSockFd())
				{
					maxSock = _clients[i]->getSockFd();
				}
			}
			//nfds(select�ĵ�һ������)��һ������ֵ ��ָfd_set����������������(socket)�ķ�Χ������������
			//���������ļ����������ֵ��1����window�������������д0
			timeval t = { 0,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//û����Ӧ
			if (ret == 0) {
				return true;
			}
			if (ret < 0) {
				std::cout << "select�������!" << std::endl;
				Close();
				return false;
			}
			//�ж�������(socket)�Ƿ��ڼ�����
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				//�ȴ����ܿͻ�������
				Accept();
				return true;
			}
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->getSockFd(), &fdRead))
				{
					//������ͻ��˵�������ͨѶ
					if (-1 == RecvData(_clients[i]))
					{
						delete _clients[i];
						auto iter = _clients.begin() + i;
						_clients.erase(iter);
					}
				}
			}
			return true;
		}
		return false;
	}
	//�Ƿ�����
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//�������� ����ճ�� ��ְ�
	int RecvData(ClientSocket* client) {
		//���ܿͻ�����������
		int nLen = (int)recv(client->getSockFd(), szRecv, RECV_BUFF_SZIE, 0);
		//unsigned optVal;
		//int tempLen = sizeof(int);
		//getsockopt(_sock, SOL_SOCKET, SO_SNDBUF,(char*)&optVal,&tempLen);
		//printf("������:%d\n",optVal);
		if (nLen <= 0) {
			std::cout << "�ͻ���" << client->getSockFd() << "�˳�" << std::endl;
			return -1;
		}
		//���յ������ݿ�������Ϣ������
		memcpy((void*)(client->getMsgBuf()+client->getLastPos()), szRecv, nLen);
		//std::cout << strlen(szRecv) << "��������С:" << strlen(client->getMsgBuf()) << "nLen:" << nLen << "β��ָ��λ��:" << client->getLastPos() << std::endl;
		//��Ϣ������������β��λ�ú���
		client->setLastPos(client->getLastPos()+nLen);
		while (client->getLastPos() >= sizeof(DataHeader))
		{ 
			//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
			DataHeader* header = (DataHeader *)client->getMsgBuf();
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (client->getLastPos() >= header->dataLength) 
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nsize = client->getLastPos() - header->dataLength;
				//����������Ϣ
				OnNetMsg(client->getSockFd(),header->cmd);
				//����Ϣ������ʣ��δ��������ǰ��
				client->setMsgBuf(client->getMsgBuf()+header->dataLength,nsize);
				//��Ϣ������������β��λ��ǰ��
				client->setLastPos(nsize);
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		}
		return 0;
	};
	//��Ӧ������Ϣ
	virtual void OnNetMsg(SOCKET cSock, int cmd)
	{
		++_recvCount;
		auto t1 = _Time.getElapsedSecond();
		if ( t1>= 1.0)
		{
			printf("time:<%llf>,clientCount<%d>,socket<%d>,recvCount<%d>\n",t1,_clients.size(),_sock,_recvCount);
			_recvCount = 0;
			_Time.update();
		}
		//��������
		switch (cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)szRecv;
			//std::cout << "�յ��ͻ���" << cSock << "���ݳ���:" << login->dataLength << "�յ�����:CMD_LOGIN" << "�յ�userName:" << login->userName << "�յ�passwd" << login->passWord << std::endl;
			//�߼�ҵ��...
			//��������
			//LoginResult ret;
			//SendData(cSock, &ret);
		}break;
		case CMD_LOGINOUT: {
			Loginout* loginout = (Loginout*)szRecv;
			//std::cout << "�յ��ͻ���" << cSock << "���ݳ���:" << loginout->dataLength << "�յ����� : CMD_LOGINOUT" << "�յ�userName : " << loginout->userName << std::endl;
			//�߼�ҵ��...
			//LoginOutResult ret;
			//SendData(cSock, &ret);
		}break;
		default: {
			DataHeader header;
			std::cout<< "�յ���������!" << std::endl;
			SendData(cSock, &header);
		}break;
		}
	}
	//���ݷ��͸�ָ��Socket
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//���ݷ��͸�����Socket
	void SendData2All(DataHeader* header)
	{
		for (int i = (int)_clients.size() - 1; i >= 0; i--)
		{
			SendData(_clients[i]->getSockFd(), header);
		}
	}
};

EasyTcpServer::EasyTcpServer()
{
	_sock = INVALID_SOCKET;
	_recvCount = 0;
}

EasyTcpServer::~EasyTcpServer()
{
	Close();
}
inline SOCKET EasyTcpServer::getSocket() const
{
	return _sock;
}
#endif // !_EasyTcpServer_hpp_