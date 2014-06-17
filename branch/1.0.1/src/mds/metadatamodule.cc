#include "metadatamodule.hh"

#include "../common/debug.hh"

MetaDataModule::MetaDataModule() {
	_configMetaDataStorage = new ConfigMetaDataModule();
	_fileMetaDataModule = new FileMetaDataModule(_configMetaDataStorage);
	_segmentMetaDataModule = new SegmentMetaDataModule(_configMetaDataStorage);

	srand(time(NULL));
}

/**
 * @brief	Create Meta Data Entry for a New File
 */
uint32_t MetaDataModule::createFile(uint32_t clientId, const string &path,
		uint64_t fileSize) {
	uint32_t fileId = _fileMetaDataModule->generateFileId();

	_fileMetaDataModule->createFile(clientId, path, fileSize, fileId);

	return fileId;
}

/**
 * @brief	Open a File
 */
void MetaDataModule::openFile(uint32_t clientId, uint32_t fileId) {
	return;
}

void MetaDataModule::deleteFile(uint32_t clientId, uint32_t fileId) {
	_fileMetaDataModule->deleteFile(fileId);
}

void MetaDataModule::renameFile(uint32_t clientId, uint32_t fileId,
		const string& newPath) {
	_fileMetaDataModule->renameFile(fileId, newPath);
}

/**
 *	@brief	Set File Size of a File
 */
void MetaDataModule::setFileSize(uint32_t fileId, uint64_t fileSize) {
	_fileMetaDataModule->setFileSize(fileId, fileSize);
	return;
}

/**
 * @brief	Read File Size of a File
 */
uint64_t MetaDataModule::readFileSize(uint32_t fileId) {
	return _fileMetaDataModule->readFileSize(fileId);
}

/**
 * @brief	Generate List of Segment ID
 */
vector<uint64_t> MetaDataModule::newSegmentList(uint32_t numOfObjs) {
	vector<uint64_t> segmentList(numOfObjs);
	for (uint32_t i = 0; i < numOfObjs; ++i) {
		//segmentList.push_back(newSegmentId());
		segmentList[i] = _segmentMetaDataModule->generateSegmentId();
	}
	return segmentList;
}

/**
 * @brief	Save Segment List of a File
 */
void MetaDataModule::saveSegmentList(uint32_t fileId,
		const vector<uint64_t> &segmentList) {
	debug("Save %d %zu\n", fileId, segmentList.size());
	_fileMetaDataModule->saveSegmentList(fileId, segmentList);

	return;
}

vector<uint64_t> MetaDataModule::readSegmentList(uint32_t fileId) {
	return _fileMetaDataModule->readSegmentList(fileId);
}

vector<uint64_t> MetaDataModule::readOsdPrimarySegmentList(uint32_t osdId) {
	return _segmentMetaDataModule->findOsdPrimarySegments(osdId);
}

vector<uint64_t> MetaDataModule::readOsdSegmentList(uint32_t osdId) {
	return _segmentMetaDataModule->findOsdSegments(osdId);
}

/**
 * @brief	Save Segment Info
 */
void MetaDataModule::saveSegmentInfo(uint64_t segmentId,
		struct SegmentMetaData segmentInfo) {
	_segmentMetaDataModule->saveSegmentInfo(segmentId, segmentInfo);
	return;
}

/**
 * @brief	Save Segment Info to Cache
 */
void MetaDataModule::saveSegmentInfoToCache(uint64_t segmentId,
		struct SegmentMetaData segmentInfo) {
	_segmentMetaDataModule->saveSegmentInfoToCache(segmentId, segmentInfo);

	return;
}
/**
 * @brief	Read Segment Info
 *
 * @param	segmentId	ID of the Segment
 *
 * @return	Info of the Segment
 */
struct SegmentMetaData MetaDataModule::readSegmentInfo(uint64_t segmentId) {
	return _segmentMetaDataModule->readSegmentInfo(segmentId);
}

/**
 * @brief	Set Primary of a Segment
 */
void MetaDataModule::setPrimary(uint64_t segmentId, uint32_t primaryOsdId) {
	_segmentMetaDataModule->setPrimary(segmentId, primaryOsdId);
	return;
}

uint32_t MetaDataModule::selectActingPrimary(uint64_t segmentId,
		vector<uint32_t> nodeList, vector<bool> nodeStatus) {

	int failedOsdCount = (int) count(nodeStatus.begin(), nodeStatus.end(),
			false);

	if (failedOsdCount == (int) nodeStatus.size()) {
		debug_yellow("All OSD has died for segmentId = %" PRIu64 "\n",
				segmentId);
		exit(-1);
	}

	srand(time(NULL));

	// random until a healthy OSD is found
	while (true) {
		int newPrimary = rand() % nodeStatus.size();
		if (nodeStatus[newPrimary] == true) {
			setPrimary(segmentId, nodeList[newPrimary]);
			return nodeList[newPrimary];
		}
	}

	return 0;
}

/**
 * @brief	Get Primary of a Segment
 */
uint32_t MetaDataModule::getPrimary(uint64_t segmentId) {
	return _segmentMetaDataModule->getPrimary(segmentId);
}

/**
 * @brief	Save Node List of a Segment
 */
void MetaDataModule::saveNodeList(uint64_t segmentId,
		const vector<uint32_t> &segmentNodeList) {
	_segmentMetaDataModule->saveNodeList(segmentId, segmentNodeList);
	return;
}

/**
 * @brief	Read Node List of a Segment
 */
vector<uint32_t> MetaDataModule::readNodeList(uint64_t segmentId) {
	return _segmentMetaDataModule->readNodeList(segmentId);
}

uint32_t MetaDataModule::lookupFileId(const string &path) {
	return _fileMetaDataModule->lookupFileId(path);
}

string MetaDataModule::lookupFilePath(uint32_t fileId) {
	return "";
}

vector<pair<uint32_t, uint64_t>> MetaDataModule::getSegmentsFromCoding(
		CodingScheme codingScheme) {
	return _segmentMetaDataModule->getSegmentsFromCoding(codingScheme);
}
