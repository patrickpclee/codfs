#ifndef __FILE_META_DATA_CACHE__
#define __FILE_META_DATA_CACHE__

#include <unordered_map>
#include "../common/metadata.hh"
#include "../common/define.hh"



class FileMetaDataCache {
	public:
		// Throw out_of_range error if does not exist
		uint32_t path2Id(string path);
		string id2Path(uint32_t id);

		struct FileMetaData getMetaData(uint32_t id);
		void saveMetaData(const struct FileMetaData& fileMetaData);
		void removeMetaData(uint32_t id);
		int renameMetaData(string path, string new_path);
	private:
        RWMutex _metaDataCacheMutex;
		std::unordered_map<uint32_t, struct FileMetaData> _metaDataCache;
		std::unordered_map<string, uint32_t> _fileIdCache;
}; 
#endif 
