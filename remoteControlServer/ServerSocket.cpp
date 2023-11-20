#include "pch.h"
#include "ServerSocket.h"

CServerSocket* CServerSocket:: m_serverSocket = NULL;
CServerSocket::Deletor CServerSocket::deletor;