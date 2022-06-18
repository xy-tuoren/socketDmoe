#ifndef _EasyTcpServer_hpp_
#define _EasyTcpServer_hpp_
#define FD_SETSIZE 1024
#include "CELLTimestamp.hpp"
//缓冲区大小
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
	//第二缓冲区 消息缓冲区
	char szMsBuf[RECV_BUFF_SZIE * 10];
	//消息缓冲区的数据尾部位置
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
	//缓冲区
	char szRecv[RECV_BUFF_SZIE] = {};
	CELLTimestamp _Time;
	int _recvCount;
public:
	EasyTcpServer();
	virtual ~EasyTcpServer();
	SOCKET getSocket() const;
	//初始化socket
	// //初始化socket
	SOCKET InitSocket()
	{
		if (INVALID_SOCKET != _sock) {
			cout << "socket:" << _sock << "关闭旧连接!" << endl;
			Close();
		}
#ifdef _WIN32
		//启动windows socket 2.x环境
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		//建立一个流socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			cout << "socket=" << _sock << "错误" << "建立socket失败!" << endl;
		}
		else
		{
			cout << "socket=" << _sock << "建立socket成功!" << endl;
		}
		return _sock;
	}
	//绑定IP和端口号
	int Bind(const char* ip, unsigned short port) {
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		sockaddr_in _sin = {};
		_sin.sin_family = AF_INET;
		_sin.sin_port = htons(port); //端口转换成无符号短整型
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
			std::cout << "绑定用于接受客户端连接的网络端口:" << port << "失败" << std::endl;
		}
		else
		{
			std::cout << "绑定" << port << "端口成功" << std::endl;
		}
		return ret;
	}
	//监听端口号
	int Listen(int n)
	{
		int ret = listen(_sock, n);
		if (SOCKET_ERROR == ret) {
			std::cout << "监听用于接受客户端连接的网络端口失败" << std::endl;
		}
		else
		{
			std::cout << "监听端口成功" << std::endl;
		}
		return ret;
	}
	//接收客户端连接
	SOCKET Accept()
	{
		// accept等待接受客户端连接
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
			std::cout << "socket=" << _sock << "错误，接受到无效客户端的SOCKET" << std::endl;
		}
		else
		{
			//新客户端加入广播到所有客户端
			//NewUserJoin userJoin;
			//userJoin.sock = cSock;
			//SendData2All(&userJoin);
			_clients.emplace_back(new ClientSocket(cSock));
			std::cout << "服务器socket=" << _sock << "新客户端加入: socket=" << cSock << " IP = " << inet_ntoa(clientAddr.sin_addr) << "客户端数量:" << _clients.size() << std::endl;
		}
		return cSock;
	}
	//关闭socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
#ifdef _WIN32	
				//清理套接字
				closesocket(_clients[i]->getSockFd());
				delete _clients[i];
#else
				//清理套接字
				close(_clients[i]->getSockFd());
				delete _clients[i];
#endif
			}
			_clients.clear();
#ifdef _WIN32
			//清除Windows socket环境
			WSACleanup();
			system("pause");
#endif
			cout << "已退出!" << endl;
			_sock = INVALID_SOCKET;
		}
	}
	//处理网络消息
	bool OnRun()
	{
		if (isRun())
		{
			//伯克利套接字 BSD socket
			//描述符(socket)集合
			fd_set fdRead;
			fd_set fdWrite;
			fd_set fdExp;
			//清理集合
			FD_ZERO(&fdRead);
			FD_ZERO(&fdWrite);
			FD_ZERO(&fdExp);
			//将描述符(socket)加入集合
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
			//nfds(select的第一个参数)是一个整数值 是指fd_set集合中所有描述符(socket)的范围，而不是数量
			//即是所有文件描述符最大值加1，在window中这个参数可以写0
			timeval t = { 0,0 };
			int ret = select(maxSock + 1, &fdRead, &fdWrite, &fdExp, &t);
			//没有响应
			if (ret == 0) {
				return true;
			}
			if (ret < 0) {
				std::cout << "select任务结束!" << std::endl;
				Close();
				return false;
			}
			//判断描述符(socket)是否在集合中
			if (FD_ISSET(_sock, &fdRead)) {
				FD_CLR(_sock, &fdRead);
				//等待接受客户端连接
				Accept();
				return true;
			}
			for (int i = (int)_clients.size() - 1; i >= 0; i--)
			{
				if (FD_ISSET(_clients[i]->getSockFd(), &fdRead))
				{
					//处理与客户端的连接与通讯
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
	//是否工作中
	bool isRun()
	{
		return _sock != INVALID_SOCKET;
	}
	//接收数据 处理粘包 拆分包
	int RecvData(ClientSocket* client) {
		//接受客户端请求数据
		int nLen = (int)recv(client->getSockFd(), szRecv, RECV_BUFF_SZIE, 0);
		//unsigned optVal;
		//int tempLen = sizeof(int);
		//getsockopt(_sock, SOL_SOCKET, SO_SNDBUF,(char*)&optVal,&tempLen);
		//printf("缓冲区:%d\n",optVal);
		if (nLen <= 0) {
			std::cout << "客户端" << client->getSockFd() << "退出" << std::endl;
			return -1;
		}
		//将收到得数据拷贝到消息缓冲区
		memcpy((void*)(client->getMsgBuf()+client->getLastPos()), szRecv, nLen);
		//std::cout << strlen(szRecv) << "缓冲区大小:" << strlen(client->getMsgBuf()) << "nLen:" << nLen << "尾部指针位置:" << client->getLastPos() << std::endl;
		//消息缓冲区的数据尾部位置后移
		client->setLastPos(client->getLastPos()+nLen);
		while (client->getLastPos() >= sizeof(DataHeader))
		{ 
			//这时就可以知道当前消息体的长度
			DataHeader* header = (DataHeader *)client->getMsgBuf();
			//判断消息缓冲区的数据长度大于消息长度
			if (client->getLastPos() >= header->dataLength) 
			{
				//消息缓冲区剩余未处理数据的长度
				int nsize = client->getLastPos() - header->dataLength;
				//处理网络消息
				OnNetMsg(client->getSockFd(),header->cmd);
				//将消息缓冲区剩余未处理数据前移
				client->setMsgBuf(client->getMsgBuf()+header->dataLength,nsize);
				//消息缓冲区的数据尾部位置前移
				client->setLastPos(nsize);
			}
			else
			{
				//消息缓冲区剩余数据不够一条完整消息
				break;
			}
		}
		return 0;
	};
	//响应网络消息
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
		//处理请求
		switch (cmd)
		{
		case CMD_LOGIN: {
			Login* login = (Login*)szRecv;
			//std::cout << "收到客户端" << cSock << "数据长度:" << login->dataLength << "收到命令:CMD_LOGIN" << "收到userName:" << login->userName << "收到passwd" << login->passWord << std::endl;
			//逻辑业务...
			//发送数据
			//LoginResult ret;
			//SendData(cSock, &ret);
		}break;
		case CMD_LOGINOUT: {
			Loginout* loginout = (Loginout*)szRecv;
			//std::cout << "收到客户端" << cSock << "数据长度:" << loginout->dataLength << "收到命令 : CMD_LOGINOUT" << "收到userName : " << loginout->userName << std::endl;
			//逻辑业务...
			//LoginOutResult ret;
			//SendData(cSock, &ret);
		}break;
		default: {
			DataHeader header;
			std::cout<< "收到错误数据!" << std::endl;
			SendData(cSock, &header);
		}break;
		}
	}
	//数据发送给指定Socket
	int SendData(SOCKET cSock, DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(cSock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}
	//数据发送给所有Socket
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