#ifndef _EasyTcpClient_hpp_
#define _EasyTcpClient_hpp_
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
//��������С
#define RECV_BUFF_SZIE 1024
#include<windows.h>
#include<WinSock2.h>
#pragma comment(lib,"ws2_32.lib")
#else // _WIN32
#include<unistd.h>
#include<arpa/inet.h>
#include <string.h>
#define SOCKET int
#define INVALID_SOCKET  (SOCKET)(~0)
#define SOCKET_ERROR           (-1)
#endif
#include <iostream>
#include <thread>
#include <vector>
#include "MessageHeader.hpp"
using namespace std;
class EasyTcpClient
{
public:
	EasyTcpClient()
	{
		_sock = INVALID_SOCKET;
	};
	//����������
	virtual ~EasyTcpClient()
	{
		Close();
	};
	//��ʼ��socket
	void InitSocket()
	{
#ifdef _WIN32
		//����windows socket 2.x����
		WORD ver = MAKEWORD(2, 2);
		WSADATA dat;
		WSAStartup(ver, &dat);
#endif
		if (INVALID_SOCKET != _sock) {
			cout << "socket:" << _sock << "�رվ�����!" << endl;
			Close();
		}
		//����һ����socket
		_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
		if (INVALID_SOCKET == _sock) {
			cout << "socket=" << _sock << "����" << "����socketʧ��!" << endl;
		}
		else
		{
			//cout << "socket=" << _sock << "����socket�ɹ�!" << endl;
		}
	}
	//���ӷ�����
	int Connect(const char* ip, unsigned short port)
	{
		if (INVALID_SOCKET == _sock) {
			InitSocket();
		}
		//���ӷ����� connet
		sockaddr_in sin = {};
		sin.sin_family = AF_INET;
		sin.sin_port = htons(port);
#ifdef _WIN32	
		sin.sin_addr.S_un.S_addr = inet_addr(ip);
#else
		sin.sin_addr.s_addr = inet_addr(ip);
#endif    
		int ret = connect(_sock, (sockaddr*)&sin, sizeof(sin));
		if (SOCKET_ERROR == ret) {
			std::cout << "socket=" << _sock << "���ӷ�����:" << ip << ":" << port << "ʧ��" << std::endl;
		}
		else
		{
			//std::cout << "socket=" << _sock << "���ӷ�����:" << ip << ":" << port << "�ɹ�" << std::endl;
		}
		return ret;
	}
	//�ر�socket
	void Close()
	{
		if (_sock != INVALID_SOCKET)
		{
#ifdef _WIN32   
			//�ر��׽���
			closesocket(_sock);
			//���Windows socket����
			//WSACleanup();
			//system("pause");
#else
			close(_sock);
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
			fd_set fdReads;
			FD_ZERO(&fdReads);
			FD_SET(_sock, &fdReads);
			timeval t = { 0,0 };
			int ret = select(_sock + 1, &fdReads, 0, 0, &t);
			if (ret < 0) {
				std::cout << "socket=" << _sock << "select�������1" << std::endl;
				Close();
				return false;
			}
			if (FD_ISSET(_sock, &fdReads)) {
				FD_CLR(_sock, &fdReads);
				if (RecvData() < 0) {
					std::cout << "socket=" << _sock << "select�������2" << std::endl;
					Close();
					return false;
				};
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
	//�������ݴ���ճ�����
	int RecvData() {
		int nLen = (int)recv(_sock, szRecv, RECV_BUFF_SZIE, 0);
		if (nLen <= 0) {
			std::cout << "socket=" << _sock << "��������Ͽ�����,���������" << std::endl;
			return -1;
		}
		//���յ������ݿ�������Ϣ������
		memcpy(szMsgBuf + lastPos, szRecv, nLen);
		//��Ϣ������������β��λ�ú���
		lastPos += nLen;
		//�ж���Ϣ�����������ݳ��ȴ�����ϢͷDateHeader����
		while(lastPos >= sizeof(DataHeader))
		{
			//��ʱ�Ϳ���֪����ǰ��Ϣ��ĳ���
			DataHeader* header = (DataHeader*)szMsgBuf;
			//�ж���Ϣ�����������ݳ��ȴ�����Ϣ����
			if (lastPos >= header->dataLength)
			{
				//��Ϣ������ʣ��δ�������ݵĳ���
				int nSize = lastPos - header->dataLength;
				//����������Ϣ
				onNetMsg(header->cmd);
				//����Ϣ������ʣ��δ��������ǰ��
				memcpy(szMsgBuf, szMsgBuf + header->dataLength, nSize);
				//��Ϣ������������β��λ��ǰ��
				lastPos = nSize;
			}
			else
			{
				//��Ϣ������ʣ�����ݲ���һ��������Ϣ
				break;
			}
		} 
		return 0;
	};
	//��Ӧ
	virtual void onNetMsg(int cmd)
	{
		//�������˷��͵�����
		switch (cmd)
		{
		case CMD_LOGIN_RESULT: {
			//LoginResult* loginRet = (LoginResult*)szRecv;
			//std::cout << "socket=" << _sock << "�յ���������Ϣ:" << "���ݳ���:" << loginRet->dataLength << "�յ�����:CMD_LOGIN_RESULT" << "����" << loginRet->result << std::endl;
		}break;
		case CMD_LOGINOUT_RESULT: {
			//LoginOutResult* loginOutRet = (LoginOutResult*)szRecv;
			//std::cout << "socket=" << _sock << "�յ���������Ϣ" << "���ݳ���:" << loginOutRet->dataLength << "�յ�����:CMD_LOGINOUT_RESULT" << "����" << loginOutRet->result << std::endl;
		}break;
		case CMD_NEW_USER_JOIN: {
			//NewUserJoin* newUserJoin = (NewUserJoin*)szRecv;
			//std::cout << "socket=" << _sock << "�յ���������Ϣ" << "���ݳ���:" << newUserJoin->dataLength << "�յ�����:CMD_NEW_USER_JOIN" << "����" << newUserJoin->sock << std::endl;
		}break;
		default: {
			std::cout << "socket=" << _sock << "�յ�����˴�����Ϣ!" << "����Ϊ:" << ((DataHeader*)szRecv)->dataLength << std::endl;
		}break;
		}
	}

	//��������
	int SendData(DataHeader* header)
	{
		if (isRun() && header)
		{
			return send(_sock, (const char*)header, header->dataLength, 0);
		}
		return SOCKET_ERROR;
	}

	SOCKET getSocket() 
	{
		return _sock;
	}
private:
	SOCKET _sock;
	//���ջ�����
	char szRecv[RECV_BUFF_SZIE] = {};
	//�ڶ������� ��Ϣ������
	char szMsgBuf[RECV_BUFF_SZIE * 10] = {};
	int lastPos=0;
};
#endif