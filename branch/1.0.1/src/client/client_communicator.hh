#ifndef __CLIENT_COMMUNICATOR_HH__
#define __CLIENT_COMMUNICATOR_HH__

#include <vector>

#include "../communicator/communicator.hh"
#include "../common/metadata.hh"

class ClientCommunicator: public Communicator {
public:
	void display();
	void osdIdListRequest();
	void segmentMessageHandler();

	/**
	 * Upload a file to OSD
	 * @param clientId Client ID
	 * @param path Destination Path
	 * @param fileSize Size of file
	 * @param numOfObjs Number of segments
	 * @return FileMetaData structure
	 */
	struct FileMetaData uploadFile(uint32_t clientId, string path,
			uint64_t fileSize, uint32_t numOfObjs);

	/**
	 * @brief	Delete a File
	 *
	 * @param	clientId	Client ID
	 * @param	path	File Path
	 * @param	fileId	File ID
	 */
	void deleteFile(uint32_t clientId, string path, uint32_t fileId);

	/**
	 * @brief	Rename a File
	 *
	 * @param	clientId	ClientID
	 * @param	fileId	File ID
	 * @param	path	File Path
	 * @param	newPath	New File Path
	 */
	void renameFile(uint32_t clientId, uint32_t fileId, const string& path, const string &newPath);

	/**
	 * @brief	Get Download Info
	 * @param clientId Client ID
	 * @param fileId File ID
	 * @return FileMetaData structure
	 */

	struct FileMetaData downloadFile(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Get Download Info
	 *
	 * @param	clientId	Client ID
	 * @param	filePath	File Path
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData downloadFile(uint32_t clientId, string filePath);

	/**
	 * @brief	Get File Info
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the file
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData getFileInfo(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Get File Info
	 *
	 * @param	clientId	Client ID
	 * @param	filePath	File Path
	 *
	 * @return	File Meta Data
	 */
	struct FileMetaData getFileInfo(uint32_t clientId, string filePath);

	/**
	 * @brief	Save SegmentList of a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 * @param	segmentList	Segment List of the File
	 */

	void saveSegmentList (uint32_t clientId, uint32_t fileId, vector<uint64_t> segmentList);

	/**
	 * @brief	Get New Segment List
	 *
	 * @param	clientId	Client ID
	 * @param	numOfObjs	Number of Segments
	 */
	vector<struct SegmentMetaData> getNewSegmentList (uint32_t clientId, uint32_t numOfObjs);

	/**
	 * @brief	Save Segment Size of a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 * @param	fileSize	Size of the file
	 */
	void saveFileSize (uint32_t clientId, uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief	Send a request to OSD for the segment
	 * @param dstSockfd OSD Socket Descriptor
	 * @param segmentId Segment ID
	 */

	void requestSegment (uint32_t dstSockfd, uint64_t segmentId);

	/**
	 * 1. Send an init message
	 * 2. Repeatedly send data chunks to OSD
	 * 3. Send an end message
	 * @param clientID Client ID
	 * @param dstOsdSockfd Destination OSD Socket Descriptor
	 * @param segmentData SegmentData Structure
	 * @param codingScheme Coding Scheme specified
	 * @param codingSetting Coding Scheme setting
	 */

	void putSegment(uint32_t clientId, uint32_t dstOsdSockfd,
			struct SegmentData segmentData, CodingScheme codingScheme,
			string codingSetting);

	/**
	 * Send an initiate message of a segment data transfer.
	 * @param requestId 	Request ID
	 * @param connectionId 	connection ID
	 * @param segmentId 		segment ID
	 */
	void replyPutSegmentInit(uint32_t requestId, uint32_t connectionId, uint64_t segmentId); // new version

	/**
	 * Send an end message of an segment data transfer.
	 * @param requestId 	Request ID
	 * @param connectionId 	connection ID
	 * @param segmentId 		segment ID
	 */
	void replyPutSegmentEnd(uint32_t requestId, uint32_t connectionId, uint64_t segmentId, bool isSmallSegment); //new version

	/**
	 * Send a request to monitor to get Osd List and connect 
	 */
	void getOsdListAndConnect();

private:

};
#endif
