/**
 * communicator.cc
 */

#include <iostream>
#include <thread>
#include <sys/types.h>		// required by select()
#include <unistd.h>		// required by select()
#include <sys/select.h>	// required by select()
#include <sys/ioctl.h>
#include "connection.hh"
#include "communicator.hh"
#include "socketexception.hh"
#include "../config/config.hh"
#include "../common/enums.hh"
#include "../common/debug.hh"
#include "../protocol/message.pb.h"
#include "../protocol/messagefactory.hh"

using namespace std;

// global variable defined in each component
extern ConfigLayer* configLayer;

// mutex
mutex outMessageQueueMutex;
mutex waitReplyMessageMapMutex;

Communicator::Communicator() {

	// verify protobuf version
	GOOGLE_PROTOBUF_VERIFY_VERSION;

	// initialize variables
	_requestId.store(0);
	_connectionMap = {};
	_outMessageQueue = {};
	_waitReplyMessageMap = {};
	_maxFd = 0;

	// select timeout
	_timeoutSec = configLayer->getConfigInt("Communication>SelectTimeout>sec");
	_timeoutUsec = configLayer->getConfigInt(
			"Communication>SelectTimeout>usec");

	// chunk size
	_chunkSize = configLayer->getConfigInt("Communication>ChunkSize");

	debug("%s\n", "Communicator constructed");
}

Communicator::~Communicator() {
	debug("%s\n", "Communicator destructed");
}

void Communicator::createServerSocket(uint16_t port) {

	// create a socket for accepting new peers
	if (!_serverSocket.create()) {
		throw SocketException("Could not create server socket.");
	}

	if (!_serverSocket.bind(port)) {
		throw SocketException("Could not bind to port.");
	}

	if (!_serverSocket.listen()) {
		throw SocketException("Could not listen to socket.");
	}

	debug("Server socket sockfd = %" PRIu32 "\n", _serverSocket.getSockfd());
}

/*
 * Runs in a while (1) loop
 * 1. Add serverSockfd to fd_set
 * 2. Add sockfd for all connections to fd_set
 * 3. Use select to monitor all fds
 * 4. If select returns:
 * 		serverSockfd is set : accept connection and save in map
 * 		other sockfd is set : receive a header
 * 		timeout: check if all connections are still alive
 */

void Communicator::waitForMessage() {

	char* buf; // message receive buffer
	map<uint32_t, Connection*>::iterator p; // connectionMap iterator

	int result; // return value for select
	fd_set sockfdSet; // fd_set for select
	struct timeval tv; // timeout for select

	const uint32_t serverSockfd = _serverSocket.getSockfd();

	// adjust _maxFd
	if (serverSockfd > _maxFd) {
		_maxFd = serverSockfd;
	}

	while (1) {

		// reset timeout
		tv.tv_sec = _timeoutSec;
		tv.tv_usec = _timeoutUsec;

		FD_ZERO(&sockfdSet);

		// add listen socket
		FD_SET(serverSockfd, &sockfdSet);

		// add all socket descriptors into sockfdSet
		for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
			FD_SET(p->second->getSockfd(), &sockfdSet);
		}

		// invoke select
		result = select(_maxFd + 100, &sockfdSet, NULL, NULL, &tv);

		if (result < 0) {
			cerr << "select error" << endl;
			return;
		} else if (result == 0) {
			debug("%s\n", "select timeout");
		} else {
			debug("%s\n", "select returns");
		}

		// if there is a new connection
		if (FD_ISSET(_serverSocket.getSockfd(), &sockfdSet)) {

			// accept connection
			Connection* conn = new Connection();
			_serverSocket.accept(conn->getSocket());

			// add connection to _connectionMap
			_connectionMap[conn->getSockfd()] = conn;

			debug("New connection sockfd = %" PRIu32 "\n", conn->getSockfd());

			// adjust _maxFd
			if (conn->getSockfd() > _maxFd) {
				_maxFd = conn->getSockfd();
			}
		}

		// if there is data in existing connections
		for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
			// if socket has data available
			if (FD_ISSET(p->second->getSockfd(), &sockfdSet)) {

				// check if connection is lost
				int nbytes = 0;
				ioctl(p->second->getSockfd(), FIONREAD, &nbytes);
				if (nbytes == 0) {
					// disconnect and remove from _connectionMap
					p->second->disconnect();
					_connectionMap.erase(p->first);
					continue;
				}

				// receive message into buffer, memory allocated in recvMessage
				buf = p->second->recvMessage();
				dispatch(buf, p->first);
			}
		}

	} // end while (1)
}

/**
 * 1. Set a requestId for a message if it is 0
 * 2. Push the message to _outMessageQueue
 * 3. If need to wait for reply, add the message to waitReplyMessageMap
 */

void Communicator::addMessage(Message* message, bool expectReply) {
	// check if request ID = 0
	const uint32_t requestId = generateRequestId();

	if (message->getMsgHeader().requestId == 0) {
		message->setRequestId(requestId);
	}

	{
		lock_guard<mutex> lk(outMessageQueueMutex);
		_outMessageQueue.push_back(message);
	}

	if (expectReply) {

		message->setExpectReply(true);
		debug("Message (ID: %" PRIu32 ") added to waitReplyMessageMap\n",
				message->getMsgHeader().requestId);

		{
			lock_guard<mutex> lk(waitReplyMessageMapMutex);
			_waitReplyMessageMap[requestId] = message;
		}
	}
}

Message* Communicator::popWaitReplyMessage(uint32_t requestId) {

	lock_guard<mutex> lk(waitReplyMessageMapMutex);

	// check if message is in map
	if (_waitReplyMessageMap.count(requestId)) {

		// find message
		Message* message = _waitReplyMessageMap[requestId];

		// remove message from map
		_waitReplyMessageMap.erase(requestId);

		return message;
	}
	return NULL;
}

void Communicator::sendMessage() {

	// get config: polling interval
	const uint32_t pollingInterval = configLayer->getConfigInt(
			"Communication>SendPollingInterval");

	while (1) {

		// send all message in the outMessageQueue
		while (!_outMessageQueue.empty()) {

			// get and remove from queue
			outMessageQueueMutex.lock();
			Message* message = _outMessageQueue.front();
			_outMessageQueue.pop_front();
			outMessageQueueMutex.unlock();

			const uint32_t sockfd = message->getSockfd();

			// handle disconnected component
			if (sockfd == (uint32_t) -1) {
				debug("Message (ID: %" PRIu32 ") disconnected, ignore\n",
						message->getMsgHeader().requestId);
				continue;
			}

			if (!(_connectionMap.count(sockfd))) {
				debug("%s\n", "Connection not found!");
				map<uint32_t, Connection*>::iterator p; // connectionMap iterator
				exit(-1);
			}
			_connectionMap[sockfd]->sendMessage(message);

			// debug

			message->printHeader();
			//message->printPayloadHex();
			debug(
					"Message (ID: %" PRIu32 ") FD = %" PRIu32 " removed from queue\n",
					message->getMsgHeader().requestId, sockfd);

			// delete message if it is not waiting for reply
			if (!(message->getExpectReply())) {
				debug("Deleting Message (ID: %" PRIu32 ")\n",
						message->getMsgHeader().requestId);
				delete message;
				debug("%s\n", "Message Deleted");
			}
		}

	}

	// in terms of 10^-6 seconds
	usleep(pollingInterval);
}

/**
 * 1. Connect to target component
 * 2. Add the connection to the corresponding map
 */

void Communicator::connectAndAdd(string ip, uint16_t port,
		ComponentType connectionType) {

	// Construct a Connection object and connect to component
	Connection* conn = new Connection();
	const uint32_t sockfd = conn->doConnect(ip, port, connectionType);

	// Save the connection into corresponding list
	_connectionMap[sockfd] = conn;

	// adjust _maxFd
	if (sockfd > _maxFd)
		_maxFd = sockfd;
}

/**
 * 1. Disconnect the connection
 * 2. Remove the connection from map
 * 3. Run the connection destructor
 */

void Communicator::disconnectAndRemove(uint32_t sockfd) {

	if (_connectionMap.count(sockfd)) {
		Connection* conn = _connectionMap[sockfd];
		conn->disconnect();
		_connectionMap.erase(sockfd);
		delete conn;
	} else {
		cerr << "Connection not found, cannot remove connection" << endl;
	}

}

uint32_t Communicator::getMdsSockfd() {
	// TODO: assume return first MDS
	map<uint32_t, Connection*>::iterator p;

	for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
		if (p->second->getConnectionType() == MDS) {
			return p->second->getSockfd();
		}
	}

	return -1;
}

uint32_t Communicator::getMonitorSockfd() {
	// TODO: assume return first Monitor
	map<uint32_t, Connection*>::iterator p;

	for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
		if (p->second->getConnectionType() == MONITOR) {
			return p->second->getSockfd();
		}
	}

	return -1;
}

uint32_t Communicator::getOsdSockfd() {
	// TODO: assume return first Osd
	map<uint32_t, Connection*>::iterator p;

	for (p = _connectionMap.begin(); p != _connectionMap.end(); p++) {
		if (p->second->getConnectionType() == OSD) {
			return p->second->getSockfd();
		}
	}

	return -1;
}

// static function
void Communicator::handleThread(Message* message) {
	message->handle();
}

// static function
void Communicator::sendThread(Communicator* communicator) {
	communicator->sendMessage();
}

/**
 * 1. Get the MsgHeader from the receive buffer
 * 2. Get the MsgType from the MsgHeader
 * 3. Use the MessageFactory to obtain a new Message object
 * 4. Fill in the socket descriptor into the Message
 * 5. message->parse() and fill in payload pointer
 * 6. start new thread for message->handle()
 */

void Communicator::dispatch(char* buf, uint32_t sockfd) {
	struct MsgHeader msgHeader;
	memcpy(&msgHeader, buf, sizeof(struct MsgHeader));

	const MsgType msgType = msgHeader.protocolMsgType;

	// delete after message is handled
	Message* message = MessageFactory::createMessage(this, msgType);

	message->setSockfd(sockfd);
	message->setRecvBuf(buf);
	message->parse(buf);

	// set payload pointer
	message->setPayload(
			buf + sizeof(struct MsgHeader) + msgHeader.protocolMsgSize);

	// debug
	message->printHeader();
	message->printProtocol();
	//message->printPayloadHex();

	thread t(handleThread, message);
	t.detach();
}

uint32_t Communicator::generateRequestId() {

	// increment _requestId

	_requestId.store(_requestId.load() + 1);
	return _requestId.load();

}
