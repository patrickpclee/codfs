// Implementation of the Socket class.

#include "socket.hh"
#include "string.h"
#include <string.h>
#include <iostream>
#include <errno.h>
#include <fcntl.h>
#include "../common/debug.hh"

using namespace std;

Socket::Socket() :
		m_sock(-1) {

	memset(&m_addr, 0, sizeof(m_addr));

}

Socket::~Socket() {
	if (is_valid())
		::close(m_sock);
	debug ("Socket %d closed\n", m_sock);
}

bool Socket::create() {
	m_sock = socket(AF_INET, SOCK_STREAM, 0);
	debug ("Socket %d created\n", m_sock);

	if (!is_valid())
		return false;

	// TIME_WAIT - argh
	int on = 1;
	if (setsockopt(m_sock, SOL_SOCKET, SO_REUSEADDR, (const char*) &on,
			sizeof(on)) == -1)
		return false;

	return true;

}

bool Socket::bind(const int port) {

	if (!is_valid()) {
		return false;
	}

	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = INADDR_ANY;
	m_addr.sin_port = htons(port);

	int bind_return = ::bind(m_sock, (struct sockaddr *) &m_addr,
			sizeof(m_addr));

	if (bind_return == -1) {
		return false;
	}

	return true;
}

bool Socket::listen() const {
	if (!is_valid()) {
		return false;
	}

	int listen_return = ::listen(m_sock, MAXCONNECTIONS);

	if (listen_return == -1) {
		return false;
	}

	return true;
}

bool Socket::accept(Socket* new_socket) const {
	int addr_length = sizeof(m_addr);
	new_socket->m_sock = ::accept(m_sock, (sockaddr *) &m_addr,
			(socklen_t *) &addr_length);

	if (new_socket->m_sock <= 0)
		return false;
	else
		return true;
}

uint32_t Socket::sendn(const char* buf, uint32_t buf_len) {
	const uint32_t sd = m_sock;
	uint32_t n_left = buf_len; // actual data bytes sent
	uint32_t n;
	while (n_left > 0) {
		if ((n = send(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

uint32_t Socket::recvn(char* buf, uint32_t buf_len) {
	const uint32_t sd = m_sock;
	uint32_t n_left = buf_len;
	uint32_t n = 0;
	while (n_left > 0) {
		if ((n = recv(sd, buf + (buf_len - n_left), n_left, 0)) < 0) {
			return -1;
		} else if (n == 0) {
			return 0;
		}
		n_left -= n;
	}
	return buf_len;
}

bool Socket::connect(const std::string host, const int port) {
	if (!is_valid())
		return false;

	m_addr.sin_family = AF_INET;
	m_addr.sin_port = htons(port);

	int status = inet_pton(AF_INET, host.c_str(), &m_addr.sin_addr);

	if (errno == EAFNOSUPPORT)
		return false;

	status = ::connect(m_sock, (sockaddr *) &m_addr, sizeof(m_addr));

	if (status == 0)
		return true;
	else
		return false;
}

void Socket::set_non_blocking(const bool b) {

	int opts;

	opts = fcntl(m_sock, F_GETFL);

	if (opts < 0) {
		return;
	}

	if (b)
		opts = (opts | O_NONBLOCK);
	else
		opts = (opts & ~O_NONBLOCK);

	fcntl(m_sock, F_SETFL, opts);

}

uint32_t Socket::getSockfd() {
	return (uint32_t)m_sock;
}
