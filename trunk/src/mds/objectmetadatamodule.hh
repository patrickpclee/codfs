#ifndef __OBJECT_METADATA_MODULE_HH__
#define __OBJECT_METADATA_MODULE_HH__

#include <stdint.h>
#include <vector>

#include "configmetadatamodule.hh"

#include "../storage/mongodb.hh"

#include "../common/metadata.hh"

class ObjectMetaDataModule {
public:
	/**
	 * @brief	Default Constructor
	 *
	 * @param	configMetaDataModule	Configuration Meta Data Module
	 */
	ObjectMetaDataModule(ConfigMetaDataModule* configMetaDataModule);

	/**
	 * @brief	Save Object Info
	 *
	 * @param	objectId	ID of the Object
	 * @param	objectInfo	Info of the Object
	 */
	void saveObjectInfo(uint64_t objectId, struct ObjectMetaData objectInfo);

	/**
	 * @brief	Read Object Info
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	Info of the Object
	 */
	struct ObjectMetaData readObjectInfo(uint64_t objectId);

	/**
	 * @brief	Save Node List of a Object
	 *
	 * @param	objectId	ID of the Object
	 * @param	objectNodeList	List of Node ID
	 */
	void saveNodeList(uint64_t objectId,
			const vector<uint32_t> &objectNodeList);

	/**
	 * @brief	Read Node List of a Object
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	List of Node ID
	 */
	vector<uint32_t> readNodeList(uint64_t objectId);

	/**
	 * @brief Find all objects owned by the osd
	 * @param osdId Osd ID
	 * @return list of objectId
	 */
	vector<uint64_t> findOsdObjects(uint32_t osdId);

	/**
	 * @brief Find all the objects owned by the osd as primary
	 * @param osdId Osd ID
	 * @return list of objectId
	 */

	vector<uint64_t> findOsdPrimaryObjects(uint32_t osdId);

	/**
	 * @brief	Set Primary of a Object
	 *
	 * @param	objectId	ID of the Object
	 * @param	primaryOsdId	ID of the Primary
	 */
	void setPrimary(uint64_t objectId, uint32_t primary);

	/**
	 * @brief	Get Primary of a Object
	 *
	 * @param	objectId	ID of the Object
	 *
	 * @return	ID of the Primary
	 */
	uint32_t getPrimary(uint64_t objectId);

	/**
	 * @brief	Generate a New Object ID
	 *
	 * @return	File ID
	 */
	uint64_t generateObjectId();
private:
	/// Collection
	string _collection;

	/// Configuration Meta Data Module
	ConfigMetaDataModule* _configMetaDataModule;

	/// Underlying Meta Data Storage
	MongoDB* _objectMetaDataStorage;

	//ObjectMetaDataCache *_objectMetaDataCache;
};
#endif
