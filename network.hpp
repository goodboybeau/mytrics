/*
 * timer.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: jaronhalt
 */

#ifdef __cplusplus
#if __cplusplus < 201103L
#define __cplusplus 201103L
#endif
#endif

#ifndef NETWORK_HPP_
#define NETWORK_HPP_

#include <iostream>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <cerrno>
#include <unistd.h>
#include <atomic>


class socket
{
	int fd;

protected:
	sockaddr_in addr;

public:

	enum SOCKET_TYPE
	{
		TCP = SOCK_STREAM,
		UDP = SOCK_DGRAM
	};

	socket(const enum SOCKET_TYPE& type, const int& family=AF_INET)
		: fd(-1)
	{
		memset(&this->addr, 0, sizeof(this->addr));

		if(0 > (this->fd = ::socket(family, type, type == SOCKET_TYPE::UDP ? IPPROTO_UDP : IPPROTO_TCP)))
		{
			this->fd = -1;
			std::cout << "failed to create socket" << std::endl;
		}
		std::cout << "socket fd: " << this->fd << std::endl;
	}

	~socket()
	{
		if(fd > 0)
		{
			close(fd);
		}
	}

	const int get() const
	{
		return this->fd;
	}
};

class NetworkSocket : protected socket
{
protected:
	std::string host;
	uint16_t port;

	NetworkSocket(const std::string& host, const uint16_t& port, const socket::SOCKET_TYPE stype);
	virtual ~NetworkSocket() = default;
public:
	virtual int send(const std::string&) const = 0;
	virtual int recv(uint8_t*, int, bool) = 0;
	virtual int bind(void) = 0;
};

class UDPSocket : protected NetworkSocket
{
public:
	UDPSocket(const std::string& host, const uint16_t& port);
	virtual ~UDPSocket() = default;

	int send(const std::string& data) const;
	int recv(uint8_t* buffer, int length=1024, bool allocate_buffer = true);
	int bind();
};

class TCPSocket : protected NetworkSocket
{
	std::atomic_bool connected;
public:
	TCPSocket(const std::string& host, const uint16_t& port);
	virtual ~TCPSocket() = default;

	int connect();
	int bind() {return -1;}
	int send(const std::string& data);
	int recv(uint8_t* buffer, int length=1024, bool allocate_buffer = true);
};


#endif /* NETWORK_HPP_ */
