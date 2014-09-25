/**
 * communicator.hh
 */

#ifndef __COMMUNICATOR_HH__
#define __COMMUNICATOR_HH__

#include <string>
#include <vector>
#include <list>
#include <queue>
#include <map>
#include <atomic>
#include "../protocol/messagefactory.hh"
#include "../protocol/message.hh"
#include "../common/enums.hh"
#include "../common/define.hh"
#include "../common/recvbuffer.hh"
#include "../datastructure/concurrentmap.hh"
#include "socket.hh"
#include "component.hh"
#include "connection.hh"

#include "../datastructure/lowlockqueue.hh"

using namespace std;

// forward declaration to avoid circular dependency
class Message;
class MessageFactory;
class Connection;

/**
 * Abstract Communication module for all components.
 * Handles I/O Multiplexing
 */

class Communicator {
public:

	/**
	 * Constructor
	 */

	Communicator(); // constructor

	/**
	 * Destructor
	 */

	virtual ~Communicator(); // destructor

	/**
	 * Listen to all the socket descriptors and call select for I/O multiplexing
	 * When a Message is received, call dispatch() to execute handler
	 */

	void waitForMessage();

	/**
	 * Add message to _outMessageQueue
	 * Set a unique request ID if request ID = 0
	 * @param message Message to send
	 * @param expectReply Whether or not this message should wait for a reply
	 * @param waitOnRequestId Specify a requestId to wait on
	 */

	void addMessage(Message* message, bool expectReply = false,
			uint32_t waitOnRequestId = 0);

	void sendMessage(uint32_t fd);

	/**
	 * Create a server socket at the specified port
	 * @return Binded Socket
	 */

	void createServerSocket();

	/**
	 * Establish a connection to a component. Save the connection to list
	 * @param ip Destination IP
	 * @param port Destination Port
	 * @param Destination type: MDS/CLIENT/MONITOR/OSD
	 * @return Sockfd Descriptor of the new connection
	 */

	uint32_t connectAndAdd(string ip, uint16_t port,
			ComponentType connectionType); // establish a connection

	/**
	 * Disconnect from component and remove from list
	 * @param sockfd
	 */

	void disconnectAndRemove(uint32_t sockfd); // kill and remove connection

	// FOR TESTING ONLY
	uint32_t getMdsSockfd();
	uint32_t getMonitorSockfd();
	uint32_t getOsdSockfd();

	/**
	 * Obtain a list of MDS retrieved from config file
	 * @return list of MDS Component
	 */

	vector<Component> getMdsList();

	/**
	 * Obtain a list of OSD retrieved from config file
	 * @return list of OSD Component
	 */

	vector<Component> getOsdList();

	/**
	 * Obtain a list of Monitor retrieved from config file
	 * @return list of Monitor Component
	 */

	vector<Component> getMonitorList();

	/**
	 * Generate a monotonically increasing requestID
	 * @return Generated requestID
	 */

	inline uint32_t generateRequestId() {
		return ++_requestId;
	}

	/**
	 * Retrieve the pointer to a sent message by its requestId
	 * Remove the message from sentMessageQueue
	 * @param requestId Request ID
	 * @return Pointer to sent message
	 */

	Message* popWaitReplyMessage(uint32_t requestId);

	/**
	 * Connect to all components specified in the config file
	 */

	void connectAllComponents();

	/**
	 * Runs in a separate detached thread
	 * Execute communicator->sendMessage function
	 * @param communicator Corresponding communicator of the component
	 */
	static void sendThread(Communicator* communicator);

	/**
	 * Set the Component ID
	 * @param id Component ID
	 */

	void setId(uint32_t id);

	/**
	 * Set the Component Type
	 * @param componentType Component Type
	 */

	void setComponentType(ComponentType componentType);

	/**
	 * Process handshake request and send reply
	 * @param requestId Request ID
	 * @param _sockfd Socket Descriptor of the source
	 * @param componentId Component ID of the source
	 * @param componentType Component Type of the source
	 */

	void handshakeRequestProcessor(uint32_t requestId, uint32_t _sockfd,
			uint32_t componentId, ComponentType componentType);

	/**
	 * Obtain the sockfd concerning the component's connection
	 * @param componentId Component ID
	 * @return Socket descriptor of the component
	 */

	uint32_t getSockfdFromId(uint32_t componentId);

	/**
	 * Send an segment to a socket descriptor
	 * @param componentId My Component ID
	 * @param sockfd Destination Socket Descriptor
	 * @param segmentData SegmentData structure
	 * @param codingScheme (Optional) Coding Scheme
	 * @param codingSetting (Optional) Coding Setting
	 * @return Number of bytes sent
	 */

	uint32_t sendSegment(uint32_t componentId, uint32_t sockfd,
			struct SegmentData segmentData, CodingScheme codingScheme =
					DEFAULT_CODING, string codingSetting = "");

	void lockDataQueue(uint32_t sockfd);
	void unlockDataQueue(uint32_t sockfd);

	/**
	 * Connect to monitor (test)
	 */
	void connectToMonitor();
	void connectToMds();

	void connectToMyself(string ip, uint16_t port, ComponentType type);
//TODO: documentation

	void connectToOsd(uint32_t ip, uint32_t port);

	uint16_t getServerPort();

	/**
	 * Set the Server Port of the Communicator
	 * @param port Server Port
	 */
	void setServerPort(uint16_t port);
protected:

	/**
	 * Initiate upload process to OSD (Step 1)
	 * @param componentId My Component ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param segLength Size of the segment
	 * @param bufLength Size of the buf to be send
	 * @param chunkCount Number of chunks that will be sent
	 * @param updateKey Update Key
	 */

	DataMsgType putSegmentInit(uint32_t componentId, uint32_t dstOsdSockfd,
			uint64_t segmentId, uint32_t segLength, uint32_t bufLength, 
            uint32_t chunkCount, CodingScheme codingScheme, 
            string codingSetting, string updateKey);

	/**
	 * Send an segment chunk to OSD (Step 2)
	 * @param componentId Component ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param buf Buffer containing the segment
	 * @param offset Offset of the chunk inside the buffer
	 * @param length Length of the chunk
	 * @param dataMsgType Data Msg Type
	 * @param updateKey Update Key
	 */

	void putSegmentData(uint32_t componentID, uint32_t dstOsdSockfd,
			uint64_t segmentId, char* buf, uint64_t offset, uint32_t length,
			DataMsgType dataMsgType, string updateKey);

	/**
	 * Finalise upload process to OSD (Step 3)
	 * @param componentId Component ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param segmentId Segment ID
	 * @param dataMsgType Data Msg Type
	 * @param updateKey Update Key
	 * @param offsetLength <offset, length> for chunks in segment
	 */

	void putSegmentEnd(uint32_t componentId, uint32_t dstOsdSockfd,
			uint64_t segmentId, DataMsgType dataMsgType, string updateKey,
			vector<offset_length_t> offsetLength);

	uint32_t putSmallSegment (uint32_t componentId, uint32_t dstOsdSockfd,
	        uint64_t segmentId, uint32_t segLength, uint32_t bufLength,
	        CodingScheme codingScheme, string codingSetting,
	        string updateKey, char* buf, vector<offset_length_t> offsetLength);

	/**
	 * Runs in a separate detached thread
	 * Execute message->parse function
	 * @param message
	 */

	static void handleThread(Message* message);

	/**
	 * Get the MsgType from raw buffer and get a Message segment from the MessageFactory
	 * Execute message.handle() in a separate thread
	 * @param buf Pointer to the buffer holding the Message
	 * @param sockfd Socket Descriptor of incoming connection
	 * @param threadPoolLevel Level of thread pool that the dispatch function is executed
	 */

	void dispatch(char* buf, uint32_t sockfd, uint32_t threadPoolLevel);

	Message* popMessage(uint32_t fd);

	/**
	 * Delete the message when it is deletable
	 * @param message Message pointer to delete
	 */

	void waitAndDelete(Message* message);

	/**
	 * Connect to all the component in the list
	 * @param componentList Component List
	 */

	void connectToComponents(vector<Component> componentList);

	/**
	 * Parse the config file and extract information about components
	 * @param componentType Type of components to extract
	 * @return List of the component information
	 */

	vector<struct Component> parseConfigFile(string componentType);

	/**
	 * Send a handshake request to the destination component
	 * @param sockfd My Socket Descriptor
	 * @param componentId My Component ID
	 * @param componentType My Component Type
	 */

	void requestHandshake(uint32_t sockfd, uint32_t componentId,
			ComponentType componentType);


	/**
	 * DEBUG: Print the component information saved in the list
	 * @param componentType Type of component to print (for heading)
	 * @param componentList List of component information
	 */

	void printComponents(string componentType, vector<Component> componentList);

	// DEBUG
	void listThreadPool();

	void parsing(uint32_t sockfd);

	string getIpPortFromSockfd (uint32_t sockfd);

	map<uint32_t, thread> _sendThread;
	map<uint32_t, mutex*> _dataMutex;
	map<uint32_t, struct LowLockQueue<Message *>*> _outMessageQueue;
	map<uint32_t, struct LowLockQueue<Message *>*> _outDataQueue;
	map<uint32_t, struct LowLockQueue<Message *>*> _outBlockQueue;
	atomic<uint32_t> _requestId; // atomic monotically increasing request ID

	uint16_t _serverPort; // listening port for incoming connections
	Socket _serverSocket; // socket for accepting incoming connections
	map<uint32_t, Connection*> _connectionMap; // a map of all connections
	ConcurrentMap<uint32_t, uint32_t> _componentIdMap; // a map from component ID to sockfd
	ConcurrentMap<uint32_t, Message *> _waitReplyMessageMap; // map of message waiting for reply
	uint32_t _maxFd; // maximum number of socket descriptors among connections

	// self identity
	ComponentType _componentType;
	uint32_t _componentId; // id of myself

	// config values
	uint32_t _timeoutSec, _timeoutUsec;
	uint32_t _chunkSize;
	uint32_t _pollingInterval;

	// component list
	vector<Component> mdsList;
	vector<Component> osdList;
	vector<Component> monitorList;

	// Recv Optimization
	map<uint32_t, mutex*> _sockfdMutexMap;
	map<uint32_t, struct RecvBuffer *> _sockfdBufMap;
	map<uint32_t, bool> _sockfdInQueueMap;

	atomic<uint32_t> _updateId;

	bool _forwardMode;
	string _forwardIp;
};
#endif
