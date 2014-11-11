#ifndef __METADATA_MODULE_HH__
#define __METADATA_MODULE_HH__

#include <stdint.h>
#include <vector>
using namespace std;

#include "configmetadatamodule.hh"
#include "filemetadatamodule.hh"
#include "segmentmetadatamodule.hh"

#include "../common/metadata.hh"

class MetaDataModule {
public:
	MetaDataModule();

	/**
	 * @brief	Create Meta Data Entry for a New File
	 *
	 * @param	cleintId	Client ID
	 * @param	path	Path to the File
	 * @param	fileSize	Size of the File
	 *
	 * @return	File ID
	 */
	uint32_t createFile(uint32_t clientId, const string &path,
			uint64_t fileSize);

	/**
	 * @brief	Open a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	ID of the File
	 */
	void openFile(uint32_t clientId, uint32_t filieId);

	/**
	 * @brief	Delete a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileId	File ID
	 */
	void deleteFile(uint32_t clientId, uint32_t fileId);

	/**
	 * @brief	Rename a File
	 *
	 * @param	clientId	Client ID
	 * @param	fileID	File ID
	 * @param	newPath	New File Path
	 */
	void renameFile(uint32_t clientId, uint32_t fileId, const string& newPath);

	/**
	 *	@brief	Set File Size of a File
	 *
	 *	@param	fileId	ID of the File
	 *	@param	fileSize	Size of the File
	 */
	void setFileSize(uint32_t fileId, uint64_t fileSize);

	/**
	 * @brief	Read File Size of a File
	 *
	 * @param	fileId	ID of the File
	 *
	 * @return	File Size
	 */
	uint64_t readFileSize(uint32_t fileId);

	/**
	 * @brief	Generate List of Segment ID
	 *
	 * @param	numOfObjs	Number of Segments
	 *
	 * @return	List of Segment ID
	 */
	vector<uint64_t> newSegmentList(uint32_t numOfObjs);

	/**
	 * @brief	Save Segment List of a File
	 *
	 * @param	fileId	ID of the File
	 * @param	segmentList	List of Segment ID
	 */

	void saveSegmentList(uint32_t fileId, const vector<uint64_t> &segmentList);

	/**
	 * @brief	Read the list of segments which belong to the file
	 * @param fileId File ID
	 * @return list of the composing segments
	 */

	vector<uint64_t> readSegmentList(uint32_t fileId);

	/**
	 * @brief	Read the list of segments stored by the OSD
	 * @param osdId OSD ID
	 * @return list of segmentID owned by the OSD
	 */

	vector<uint64_t> readOsdSegmentList(uint32_t osdId);

	/**
	 * @brief	Read the list of segments stored by the OSD as primary
	 * @param osdId OSD ID
	 * @return list of segmentID owned by the OSD as primary
	 */

	vector<uint64_t> readOsdPrimarySegmentList(uint32_t osdId);

	/**
	 * @brief	Save Segment Info
	 *
	 * @param	segmentId	ID of the Segment
	 * @param	segmentInfo	Info of the Segment
	 */
	void saveSegmentInfo(uint64_t segmentId,
			struct SegmentMetaData segmentInfo);
    /**
	 * @brief	Save Segment Info to Cache
	 *
	 * @param	segmentId	ID of the Segment
	 * @param	segmentInfo	Info of the Segment
	 */
	void saveSegmentInfoToCache(uint64_t segmentId,
			struct SegmentMetaData segmentInfo);


	/**
	 * @brief	Read Segment Info
	 *
	 * @param	segmentId	ID of the Segment
	 *
	 * @return	Info of the Segment
	 */
	struct SegmentMetaData readSegmentInfo(uint64_t segmentId);

	/**
	 * @brief	Set Primary of a Segment
	 *
	 * @param	segmentId	ID of the Segment
	 * @param	primaryOsdId	ID of the Primary
	 */
	void setPrimary(uint64_t segmentId, uint32_t primaryOsdId);

	/**
	 * @brief	Select an acting primary OSD for the segment
	 * @param segmentId Segment ID
	 * @param nodeList list of nodes storing the blocks for the segment
	 * @param nodeStatus status of the nodes storing the blocks
	 * @return Selected acting primary OSD ID
	 */

	uint32_t selectActingPrimary(uint64_t segmentId, vector<uint32_t> nodeList,
			vector<bool> nodeStatus);

	/**
	 * @brief	Get Primary of a Segment
	 *
	 * @param	segmentId	ID of the Segment
	 *
	 * @return	ID of the Primary
	 */
	uint32_t getPrimary(uint64_t segmentId);

	/**
	 * @brief	Save Node List of a Segment
	 *
	 * @param	segmentId	ID of the Segment
	 * @param	segmentNodeList	List of Node ID
	 */
	void saveNodeList(uint64_t segmentId,
			const vector<uint32_t> &segmentNodeList);

	/**
	 * @brief	Read Node List of a Segment
	 *
	 * @param	segmentId	ID of the Segment
	 *
	 * @return	List of Node ID
	 */
	vector<uint32_t> readNodeList(uint64_t segmentId);

	/**
	 * @brief	Lookup the File Path with File ID
	 *
	 * @param	fileId	ID of the File
	 * 
	 * @return	Path to the File
	 */
	string lookupFilePath(uint32_t fileId);

	/**
	 * @brief	Lookup the File ID with file Path
	 *
	 * @param	path	Path to the File
	 *
	 * @return	ID of the File
	 */
	uint32_t lookupFileId(const string &path);

	/**
	 * @brief Search segment IDs with a specific coding scheme
	 * @param codingScheme CodingScheme struct
	 * @return list of segment ID
	 */

	vector<pair<uint32_t, uint64_t>> getSegmentsFromCoding(
			CodingScheme codingScheme);

private:

	ConfigMetaDataModule* _configMetaDataStorage;
	FileMetaDataModule* _fileMetaDataModule;
	SegmentMetaDataModule* _segmentMetaDataModule;
};

#endif
