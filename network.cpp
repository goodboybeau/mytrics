/*
 * timer.hpp
 *
 *  Created on: Sep 11, 2015
 *      Author: jaronhalt
 */

#include "network.hpp"

int resolve_host(const std::string& host, sockaddr_in& addrinfo)
{
	int status;
	struct addrinfo hints, *res, *p;

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_INET;
	//hints.ai_socktype = socket::SOCKET_TYPE::UDP;

	if((status = getaddrinfo(host.c_str(), nullptr, &hints, &res)) != 0)
	{
		std::cerr << "getaddrinfo error: " << status << " " << gai_strerror(status) << std::endl;
	}
	else
	{
		for(p = res;p != nullptr; p = p->ai_next)
		{
			if (p->ai_family == AF_INET)
			{
				addrinfo = *(struct sockaddr_in *)p->ai_addr;
				std::cout << "found addr " << inet_ntoa(addrinfo.sin_addr) << std::endl;
				break;
			}
		}
		freeaddrinfo(res); // free the linked list
	}

	return status;
}

NetworkSocket::NetworkSocket(const std::string& host, const uint16_t& port, const socket::SOCKET_TYPE stype)
	: socket(stype)
	, host(host)
	, port(port)
{
	std::cout << "creating network socket to host " << this->host << std::endl;
	if(0 != resolve_host(this->host, this->addr))
	{
		std::cout << "Error resolving host " << std::endl;
	}
	this->addr.sin_port = htons(this->port);
}

UDPSocket::UDPSocket(const std::string& host, const uint16_t& port)
	: NetworkSocket(host, port, socket::SOCKET_TYPE::UDP)
{
}

int
UDPSocket::send(const std::string& data) const
{
	int sent = 0;

	sent = sendto((int)this->get(), (const void*)data.c_str(), data.length(), 0, (struct sockaddr*)&this->addr, sizeof(this->addr));

	return sent;
}

int
UDPSocket::recv(uint8_t* buffer, int length, bool allocate_buffer)
{
	int received = 0;
	uint32_t alen = sizeof(this->addr);

	if(allocate_buffer)
	{
		buffer = new uint8_t[length];
	}

	received = recvfrom((int)this->get(), (void*)buffer, length, 0, (struct sockaddr*)&this->addr, &alen);

	return received;
}

int
UDPSocket::bind()
{
	int res = 0;
	res = ::bind(this->get(), (const sockaddr*)&this->addr, sizeof(this->addr));
	return res;
}

#include <chrono>
#include <thread>
TCPSocket::TCPSocket(const std::string& host, const uint16_t& port)
	: NetworkSocket(host, port, socket::SOCKET_TYPE::TCP)
	, connected(false)
{
}


int
TCPSocket::connect()
{
	int res = 0;
	bool expected = false;
	if(this->connected.compare_exchange_strong(expected, true))
	{
		res = ::connect(this->get(), (struct sockaddr*)&this->addr, sizeof(this->addr));
		if (res < 0)
		{
			std::cout << "Error connecting... " << res << std::endl;
			this->connected.store(false);
		}
		else
		{
			std::cout << "Connected successfully..." << std::endl;
		}
	}
	return res;
}

int
TCPSocket::send(const std::string& data)
{
	this->connect();

	int sent = 0;
	sent = write(this->get(), (const void*)data.c_str(), data.length());
	std::cout << "TCP sent " << sent << " bytes" << std::endl;

	uint8_t buffer[1024] = {0};
	int read_res = read(this->get(), (void*)buffer, sizeof(buffer));
	std::cout << "read " << read_res << " bytes" << std::endl;

	if(read_res == 0)
	{
		this->connected.store(false);
	}
	return sent;
}

int
TCPSocket::recv(uint8_t* buffer, int length, bool allocate_buffer)
{
	int received = 0;
	if(allocate_buffer)
	{
		buffer = new uint8_t[length];
	}

	received = ::recv((int)this->get(), (void*)buffer, length, 0);

	return received;
}
